/* ###################################################################
 **     Filename    : main.c
 **     Project     : Lab4
 **     Processor   : MK70FN1M0VMJ12
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2016-08-16, 22:00, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @version 1.0
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup Main_module Main module documentation
 **  @{
 */
/* MODULE main */

// CPU module - contains low level hardware initialization routines
#include "Cpu.h"
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "UART.h"
#include "packet.h"
#include "types.h"
#include "LEDS.h"
#include "Flash.h"
#include "FTM.h"
#include "PIT.h"
#include "RTC.h"
#include "SPI.h"
#include "analog.h"
#include "median.h"

// Tower version
#define Version_Number_Hi   1
#define Version_Number_Lo   0

// Packet commands
#define CMD_STARTUP_VALUE   0x04
#define CMD_VERSION_NUMBER  0x09
#define CMD_TOWER_NUMBER    0x0B
#define CMD_PROGRAM_BYTE    0x07
#define CMD_READ_BYTE       0x08
#define CMD_TOWER_MODE      0x0D
#define CMD_REAL_TIME_CLOCK 0x0C
#define CMD_PROTOCOL_MODE   0x0A
#define CMD_ANALOG_INPUT    0x50

// Function Prototypes
static void HandlePacket(void);
static BOOL HandleStartupPacket(void);
static BOOL HandleTowerVersionPacket(void);
static BOOL HandleTowerNumberPacket(void);
static BOOL HandleTowerModePacket(void);
static BOOL HandleProgramBytePacket(void);
static BOOL HandleReadBytePacket(void);
static BOOL HandleRealTimeClock(void);
static BOOL HandleProtocolModePacket(void);
static BOOL StartupValues(void);
static void PITCallback(void* arg);
static void RTCCallback(void* arg);
static void FTM0Ch0Callback(void* arg);
static void FTM0Ch1Callback(void* arg);

static const uint32_t PITPeriod     = 500000000;  /*!< PIT timer */
static const uint16_t TowerNumber   = 0xDA2;      /*!< Student ID */
static const uint16_t TowerMode     = 0x0001;     /*!< Default tower mode */
static const uint32_t BaudRate      = 115200;     /*!< Baud rate */
static uint8_t Current_Mode         = 1;          /*!< Current mode */

static TFTMChannel FTMChannel0;                   /*!< FTM channel 0 stores parameters */
static TFTMChannel FTMChannel1;                   /*!< FTM channel 1 stores parameters */
static uint16union_t *NvTowerMode;                /*!< Pointer to tower mode in flash */
static uint16union_t *NvTowerNumber;              /*!< Pointer to tower number in flash */

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
/*! @brief Main function of program
 */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  // FTM channel 0 callback
  FTMChannel0.channelNb = 0;
  FTMChannel0.delayCount = CPU_MCGFF_CLK_HZ_CONFIG_0;
  FTMChannel0.ioType.outputAction = TIMER_OUTPUT_DISCONNECT;
  FTMChannel0.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
  FTMChannel0.userArguments = NULL;
  FTMChannel0.userFunction = &FTM0Ch0Callback;

  // FTM channel 1 callback
  FTMChannel1.channelNb = 1;
  FTMChannel1.delayCount = CPU_MCGFF_CLK_HZ_CONFIG_0 / 100;
  FTMChannel1.ioType.outputAction = TIMER_OUTPUT_DISCONNECT;
  FTMChannel1.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;
  FTMChannel1.userArguments = NULL;
  FTMChannel1.userFunction = &FTM0Ch1Callback;

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  __DI();

  FTM_Init();
  LEDs_Init();
  RTC_Init(*RTCCallback, NULL );
  PIT_Init(CPU_BUS_CLK_HZ, *PITCallback, NULL);
  Analog_Init(CPU_BUS_CLK_HZ);

  if (Packet_Init(BaudRate,CPU_BUS_CLK_HZ) &&  Flash_Init())
    LEDs_On(LED_ORANGE);

  Flash_AllocateVar((void*)&NvTowerNumber, sizeof(*NvTowerNumber));
  Flash_AllocateVar((void*)&NvTowerMode, sizeof(*NvTowerMode));

  if (NvTowerNumber->l == 0xFFFF)
    Flash_Write16((uint16_t*)NvTowerNumber, TowerNumber);

  if (NvTowerMode->l == 0xFFFF)
    Flash_Write16((uint16_t*)NvTowerMode, TowerMode);

  // Set channels 0 & 1
  (void)FTM_Set(&FTMChannel0);
  (void)FTM_Set(&FTMChannel1);

  __EI();

  PIT_Set(PITPeriod, bTRUE);

  // Start timer for ADC
  (void)FTM_StartTimer(&FTMChannel1);

  /* Write your code here */

  for (;;)
  {
    //attempts to process a valid packet
    if (Packet_Get())
    {
      // Turn on LED and turn it off after a 1 second timer start
      LEDs_On(LED_BLUE);
      // Start channel 0 timer
      FTM_StartTimer(&FTMChannel0);
      // Handles the packet
      HandlePacket();
    }
  }

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
#ifdef PEX_RTOS_START
  PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
#endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*! @brief Handles commands of Tower serial protocol packet acknowledgment
 *
 */
static void HandlePacket(void)
{
  // Acknowledgement mask
  uint8_t PACKET_ACK_MASK = 0x80;
  // Set ack as zero
  uint8_t ack = 0;
  BOOL success;

  // Gets the ack mask from command
  ack = Packet_Command & PACKET_ACK_MASK;
  // Clears ack mask from command
  Packet_Command &= ~PACKET_ACK_MASK;

  switch(Packet_Command)
  {
    case CMD_STARTUP_VALUE:
      HandleStartupPacket();
      success = bTRUE;
      break;
    case CMD_VERSION_NUMBER:
      HandleTowerVersionPacket();
      success = bTRUE;
      break;
    case CMD_TOWER_NUMBER:
      HandleTowerNumberPacket();
      success = bTRUE;
      break;
    case CMD_TOWER_MODE:
      HandleTowerModePacket();
      success = bTRUE;
      break;
    case CMD_PROGRAM_BYTE:
      HandleProgramBytePacket();
      success = bTRUE;
      break;
    case CMD_READ_BYTE:
      HandleReadBytePacket();
      success = bTRUE;
      break;
    case CMD_REAL_TIME_CLOCK:
      HandleRealTimeClock();
      success = bTRUE;
      break;
    case CMD_PROTOCOL_MODE:
      HandleProtocolModePacket();
      success = bTRUE;
      break;
    default:
      success = bFALSE;
      break;
  }

  // Checks if acknowledgement is required
  if (ack)
  {
    if (success)
      Packet_Put(Packet_Command | PACKET_ACK_MASK, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    else
      Packet_Put(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  }
}

/*! @brief Allocates or sends the tower startup commands specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleStartupPacket(void)
{
  if ((Packet_Parameter1 | Packet_Parameter2 |Packet_Parameter3) == 0)
    return StartupValues();
  else
    return bFALSE;
}

/*! @brief Allocates or sends packets containing startup values
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL StartupValues(void)
{
  return
  (
    Packet_Put(CMD_STARTUP_VALUE, 0, 0, 0)
      && Packet_Put(CMD_VERSION_NUMBER, 'v', Version_Number_Hi, Version_Number_Lo)
      && Packet_Put(CMD_TOWER_NUMBER, 1, NvTowerNumber->s.Lo, NvTowerNumber->s.Hi)
      && Packet_Put(CMD_TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi)
      && Packet_Put(CMD_PROTOCOL_MODE, 1, Current_Mode, 0)
  );
}

/*! @brief Allocates or sends the tower version specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleTowerVersionPacket(void)
{
  if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x') && (Packet_Parameter3 == 0x0D))
    return (Packet_Put(CMD_VERSION_NUMBER, 'v', Version_Number_Hi, Version_Number_Lo));
  else
    return bFALSE;
}

/*! @brief Allocates or sends the tower number specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleTowerNumberPacket(void)
{
  if (Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0)
    return Packet_Put(CMD_TOWER_NUMBER, 1, NvTowerNumber->s.Lo, NvTowerNumber->s.Hi);
  if (Packet_Parameter1 == 2)
    return Flash_Write16((uint16_t*)NvTowerNumber, Packet_Parameter23);

  return bFALSE;
}

/*! @brief Allocates or sends the tower mode specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleTowerModePacket(void)
{
  if (Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0)
    return Packet_Put(CMD_TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);

  if (Packet_Parameter1 == 2)
    return Flash_Write16((uint16_t*)NvTowerMode, Packet_Parameter23);

  return bFALSE;
}

/*! @brief Reads bytes specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleReadBytePacket(void)
{
  if((Packet_Parameter1 <= 7) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
    return Packet_Put(CMD_READ_BYTE, Packet_Parameter1, 0, _FB(FLASH_DATA_START + Packet_Parameter1));
  else
    return bFALSE;
}

/*! @brief Programs bytes specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleProgramBytePacket(void)
{
  if((Packet_Parameter1 <= 8) && (Packet_Parameter2 == 0))
  {
    uint32_t offset = FLASH_DATA_START + Packet_Parameter1;
    if (Packet_Parameter1 == 8)
      return(Flash_Erase());
    else
      return(Flash_Write8((uint8_t *)offset, Packet_Parameter3));
  }
  return bFALSE;
}

/*! @brief Allocates or sends the real time clock
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleRealTimeClock(void)
{
  if ((Packet_Parameter1 <= 23 || Packet_Parameter1 >= 0)
      && (Packet_Parameter2 <= 59 || Packet_Parameter2 >= 0)
      && (Packet_Parameter3 <= 59 || Packet_Parameter3 >=0))
  {
    RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    return bTRUE;
  }
  return bFALSE;
}

/*! @brief Allocates or sends the protocol mode specified in the packet
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 *
 */
static BOOL HandleProtocolModePacket(void)
{
  if (Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0)
  {
    return Packet_Put(CMD_PROTOCOL_MODE, 1, Current_Mode, 0);
  }
  else if ((Packet_Parameter1 == 2) && (Packet_Parameter3 == 0) && ((Packet_Parameter2 == 0) || (Packet_Parameter2 == 1)))
  {
    Current_Mode = Packet_Parameter2;
    return bTRUE;
  }
  return bFALSE;
}

/*! @brief PIT interrupt to toggle green LED
 * @param arg pointer to the function
 *
 */
static void PITCallback(void* arg)
{
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
  while (PIT_TFLG0==1)
  {
    ;
  }
  LEDs_Toggle(LED_GREEN);
}

/*! @brief RTC interrupt to sample data (analog)
 * @param arg pointer to the function
 *
 */
static void RTCCallback(void* arg)
{
  uint8_t hrs, mins, secs;

  LEDs_Toggle(LED_YELLOW);
  RTC_Get(&hrs, &mins, &secs);
  Packet_Put(CMD_REAL_TIME_CLOCK, hrs, mins, secs);
}

/*! @brief FTM0 interrupt to sample data (analog)
 * @param arg pointer to the function
 *
 */
static void FTM0Ch0Callback(void* arg)
{
  LEDs_Off(LED_BLUE);
}

/*! @brief FTM1 interrupt to sample data (analog)
 * @param arg pointer to the function
 *
 */
static void FTM0Ch1Callback(void* arg)
{
  // Restart timer
  (void)FTM_StartTimer(&FTMChannel1);

  for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
    Analog_Get(channelNb);

  // Async mode
  if (Current_Mode == 0)
  {
    for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
    {
      // Transmit packet with changes in the value
      if (Analog_Input[channelNb].value.l != Analog_Input[channelNb].oldValue.l)
      {
        Packet_Put(CMD_ANALOG_INPUT, channelNb, Analog_Input[channelNb].value.s.Lo, Analog_Input[channelNb].value.s.Hi);
      }
    }
  }
  // Sync mode
  else if (Current_Mode == 1)
  {

    for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
    {
      // Transmit all packets
      Packet_Put(CMD_ANALOG_INPUT, channelNb, Analog_Input[channelNb].value.s.Lo, Analog_Input[channelNb].value.s.Hi);
    }
  }
  return;
}

/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.5 [05.21]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
