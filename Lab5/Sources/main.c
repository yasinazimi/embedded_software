/*!
** @file
** @version 1.0
** @brief  Main module.
**
**   This file contains the high-level code for the project.
**   It initialises appropriate hardware subsystems,
**   creates application threads, and then starts the OS.
**
**   An example of two threads communicating via a semaphore
**   is given that flashes the orange LED. These should be removed
**   when the use of threads and the RTOS is understood.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "OS.h"
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

// Arbitrary thread stack size
#define THREAD_STACK_SIZE   100

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
static void PITThread(void *arg);
static void RTCThread(void *arg);
static void PacketThread(void* arg);
static void InitThread(void* arg);
static void FTMThread(void* arg);
static void StartupPackets(void);
static void HandleReadBytePacket(void);
static void HandleRealTimeClock(void);
static void HandleProtocolModePacket(void);
static BOOL HandleTowerVersionPacket(void);
static BOOL HandleTowerNumberPacket(void);
static BOOL HandleTowerModePacket(void);
static BOOL HandleProgramBytePacket(void);

static const uint32_t PITPeriod     = 500000000;  	/*!< PIT timer */
static const uint16_t TowerNumber   = 0xDA2;      	/*!< Student ID */
static const uint16_t TowerMode     = 0x0001;     	/*!< Default tower mode */
static const uint32_t BaudRate      = 115200;     	/*!< Baud rate */
static uint8_t Current_Mode         = 1;          	/*!< Current mode */

static TFTMChannel FTMChannel0;                   	/*!< FTM channel 0 stores parameters */
static TFTMChannel FTMChannel1;                   	/*!< FTM channel 1 stores parameters */
volatile uint16union_t *NvTowerNumber, *NvTowerMode; 	/*!< Pointer to tower number and tower mode in flash */

static uint32_t InitThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the tower initialization thread */
static uint32_t PacketThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the packet thread */
static uint32_t PITThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the PIT thread */
static uint32_t RTCThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the RTC thread */
static uint32_t DACThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the DAC thread */
static uint32_t TxThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the UART transmit thread */
static uint32_t RxThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the UART recieve thread */
static uint32_t FTMThreadStack[THREAD_STACK_SIZE] 	__attribute__ ((aligned(0x08)));	/*!< The stack for the UART recieve thread */

// ----------------------------------------
// Global Semaphores
// ----------------------------------------
static OS_ECB *InitSemaphore;    /*!< Binary semaphore for Tower init */

/*! @brief Allocates or sends packets containing startup values
 *
 *  @return void.
 */
static void StartupPackets()
{
  Packet_Put(CMD_STARTUP_VALUE, 0, 0, 0);
  Packet_Put(CMD_VERSION_NUMBER, 'v', 1, 0);
  Packet_Put(CMD_TOWER_NUMBER, 1, NvTowerNumber->s.Lo, NvTowerNumber->s.Hi);
  Packet_Put(CMD_TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
  Packet_Put(CMD_PROTOCOL_MODE, 1, Current_Mode, 0);
}

/*! @brief Allocates or sends the startup commands specified in the packet
 *
 *  @return BOOL - bTRUE indicates no problems encountered,
 *                bFALSE indicates error in putting packet or in parameters received
 */
static BOOL HandleStartupPacket(void)
{
  // Checks if parameter 1, 2 and 3 are zero
  if ((Packet_Parameter1 == 0) && (Packet_Parameter2 == 0) && (Packet_Parameter3 == 0))
  {
    // Sends startup packets
    StartupPackets();
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @brief Allocates or sends the tower version specified in the packet
 *
 *  @return BOOL - bTRUE indicates no problems encountered,
 *                bFALSE indicates error in putting packet or in parameters received
 */
static BOOL HandleTowerVersionPacket(void)
{
  // Checks for the transmitted packets from PC to Tower
  if ((Packet_Parameter1 == 'v') && (Packet_Parameter2 == 'x'))
  {
    // Transmits packets from Tower to PC by sending the tower version number
    Packet_Put(CMD_VERSION_NUMBER, 'v', 1, 0);
    return bTRUE;
  }
  else
    return bFALSE;
}

/*! @brief Allocates or sends the tower number specified in the packet
 *
 *  @return BOOL - bTRUE indicates no problems encountered,
 *                bFALSE indicates error in putting packet or in parameters received
 */
static BOOL HandleTowerNumberPacket(void)
{
  // Checks for the transmitted packets from PC to Tower
  if ((Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0))
  {
    // Transmits the tower number from Tower to PC
    Packet_Put(CMD_TOWER_NUMBER, 1, NvTowerNumber->s.Lo, NvTowerNumber->s.Hi);
    return bTRUE;
  }
  else if (Packet_Parameter1 == 2)
  {
    // Sets the tower number upon request
    return Flash_Write16((uint16_t*)NvTowerNumber, Packet_Parameter23);
  }
  else
    return bFALSE;
}

/*! @brief Allocates or sends the tower mode specified in the packet
 *
 *  @return BOOL - bTRUE indicates no problems encountered,
 *                bFALSE indicates error in putting packet or in parameters received
 */
static BOOL HandleTowerModePacket()
{
  // Checks for the transmitted packets from PC to Tower
  if ((Packet_Parameter1 == 1 && Packet_Parameter2 == 0 && Packet_Parameter3 == 0))
  {
    // Transmits the tower mode from Tower to PC
    Packet_Put(CMD_TOWER_MODE, 1, NvTowerMode->s.Lo, NvTowerMode->s.Hi);
    return bTRUE;
  }
  else if (Packet_Parameter1 == 2)
  {
    // Sets the tower mode upon request
    return Flash_Write16((uint16_t*)NvTowerMode, Packet_Parameter23);
  }
  else
    return bFALSE;
}

/*! @brief Reads bytes specified in the packet
 *
 *  @return void.
 */
static void HandleReadBytePacket()
{
  // Transmits read byte from Tower to PC
  Packet_Put(CMD_READ_BYTE, Packet_Parameter1, 0, _FB(FLASH_DATA_START + Packet_Parameter1));
}

/*! @brief Programs bytes specified in the packet
 *
 *  @return void.
 */
static BOOL HandleProgramBytePacket()
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
 *
 *  @return void.
 */
static void HandleRealTimeClock()
{
  RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
}

/*! @brief Allocates or sends the protocol mode specified in the packet
 *
 *  @return void.
 */
static void HandleProtocolModePacket()
{
  Packet_Put(CMD_PROTOCOL_MODE, 1, Current_Mode, 0);
}

/*! @brief FTM callback function to turn off blue LED
 *
 *  @return void.
 */
static void FTMThread(void* arg)
{
  for (;;)
  {
    OS_SemaphoreWait(DACSemaphore[0], 0);
    // Turn of blue LED
    LEDs_Off(LED_BLUE);
  }
}

/*! @brief PIT thread
 *
 *  @return void.
 */
static void PITThread(void *arg)
{
  for (;;)
  {
    OS_SemaphoreWait(PITSemaphore, 0);

    // Toggle green LED
    LEDs_Toggle(LED_GREEN);
  }
}

/*! @brief RTC thread
 *
 *  @return void.
 */
static void RTCThread(void *arg)
{
  for (;;)
  {
    OS_SemaphoreWait(RTCSemaphore, 0);
    LEDs_Toggle(LED_YELLOW);

    uint8_t hrs, mins, secs;
    RTC_Get(&hrs, &mins, &secs);
    // Transmits RTC from Tower to PC
    Packet_Put(CMD_REAL_TIME_CLOCK, hrs, mins, secs);
  }
}

/*! @brief Digital to analog thread
 *
 *  @return void.
 */
static void DACThread(void* arg)
{
  // FTM channel 0 callback
  FTMChannel0.channelNb = 0;
  FTMChannel0.delayCount = CPU_MCGFF_CLK_HZ_CONFIG_0;
  FTMChannel0.ioType.outputAction = TIMER_OUTPUT_DISCONNECT;
  FTMChannel0.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;

  // FTM channel 1 callback
  FTMChannel1.channelNb = 1;
  FTMChannel1.delayCount = CPU_MCGFF_CLK_HZ_CONFIG_0 / 100; // 10ms
  FTMChannel1.ioType.outputAction = TIMER_OUTPUT_DISCONNECT;
  FTMChannel1.timerFunction = TIMER_FUNCTION_OUTPUT_COMPARE;

  // Set channels 0 & 1
  //(void)FTM_Set(&FTMChannel0);
  (void)FTM_Set(&FTMChannel1);

  // Restart timer
  //(void)FTM_StartTimer(&FTMChannel0);
  (void)FTM_StartTimer(&FTMChannel1);

  for (;;)
  {
    OS_SemaphoreWait(DACSemaphore[1], 0);

    // Start FTM channel1 timer
    (void)FTM_StartTimer(&FTMChannel1);

    for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
      Analog_Get(channelNb);

    // This handles the asynchronous mode
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
    // This handles the synchronous mode
    else if (Current_Mode == 1)
    {
      for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
      {
	// Transmit all packets
	Packet_Put(CMD_ANALOG_INPUT, channelNb, Analog_Input[channelNb].value.s.Lo, Analog_Input[channelNb].value.s.Hi);
      }
    }
  }
}

/*! @brief Packet thread
 *
 *  @return void.
 */
static void PacketThread(void* arg)
{
  FTM_Set(&FTMChannel0);

  for (;;)
  {
    if (Packet_Get())
    {
      LEDs_On(LED_BLUE);
      FTM_StartTimer(&FTMChannel0);

      uint8_t PACKET_ACK_MASK = 0x80;
      uint8_t ack = 0;
      BOOL success;

      ack = Packet_Command & PACKET_ACK_MASK;
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
  }
}

/*! @brief Tower initialization thread
 *
 *  @return void.
 */
static void InitThread(void* arg)
{
  for (;;)
  {
    OS_DisableInterrupts(); // Disable interrupts

    OS_SemaphoreWait(InitSemaphore, 0);

    if (Packet_Init(BaudRate, CPU_BUS_CLK_HZ)
	&& Flash_Init()
	&& RTC_Init(CPU_BUS_CLK_HZ)
	&& PIT_Init(CPU_BUS_CLK_HZ)
	&& FTM_Init()
	&& LEDs_Init()
	&& Analog_Init(CPU_BUS_CLK_HZ))
    {
      // Allocates flash memory to NvTowerNumber and NvTowerMode
      Flash_AllocateVar((void*)&NvTowerNumber, sizeof(*NvTowerNumber));
      Flash_AllocateVar((void*)&NvTowerMode, sizeof(*NvTowerMode));

      // TowerNumber is set to default if cleared
      if (NvTowerNumber->l == 0xFFFF)
	Flash_Write16((uint16_t*)NvTowerNumber, TowerNumber);

      // TowerMode is set to default if cleared
      if (NvTowerMode->l == 0xFFFF)
	Flash_Write16((uint16_t*)NvTowerMode, TowerMode);

      // PIT setup
      PIT_Enable(bTRUE);
      PIT_Set(PITPeriod, bTRUE);

      StartupPackets();
      OS_EnableInterrupts();
      OS_ThreadDelete(0);
    }
  }
}

/*! @brief Initialises the hardware, sets up to threads, and starts the OS.
 *
 */
/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */
  OS_ERROR error;

  // Initialise low-level clocks etc using Processor Expert code
  PE_low_level_init();

  // Initialize the RTOS
  OS_Init(CPU_CORE_CLK_HZ, true);

  // Create semaphores for signaling between threads
  InitSemaphore = OS_SemaphoreCreate(1);
  DACSemaphore[0] = OS_SemaphoreCreate(0);
  DACSemaphore[1] = OS_SemaphoreCreate(0);

  // Create threads using OS_ThreadCreate();

  error = OS_ThreadCreate(InitThread,	// Tower setup thread
			  NULL,
			  &InitThreadStack[THREAD_STACK_SIZE - 1],
			  0); // Highest priority

  error = OS_ThreadCreate(RxThread,	// UART receive thread
			  NULL,
			  &RxThreadStack[THREAD_STACK_SIZE - 1],
			  1);

  error = OS_ThreadCreate(TxThread,	// UART transmit thread
			  NULL,
			  &TxThreadStack[THREAD_STACK_SIZE - 1],
			  3);

  error = OS_ThreadCreate(DACThread,	// Digital to analog thread
			  NULL,
			  &DACThreadStack[THREAD_STACK_SIZE - 1],
			  4);

  error = OS_ThreadCreate(RTCThread,	// RTC thread
			  NULL,
			  &RTCThreadStack[THREAD_STACK_SIZE - 1],
			  5);

  error = OS_ThreadCreate(PacketThread,	// Packet thread
			  NULL,
			  &PacketThreadStack[THREAD_STACK_SIZE - 1],
			  6);

  error = OS_ThreadCreate(FTMThread,	// FTM thread
  			  NULL,
  			  &FTMThreadStack[THREAD_STACK_SIZE - 1],
  			  8);

  error = OS_ThreadCreate(PITThread,	// PIT thread
			  NULL,
			  &PITThreadStack[THREAD_STACK_SIZE - 1],
			  10);

  // Start multithreading - never returns!
  OS_Start();
}

/*!
** @}
*/
