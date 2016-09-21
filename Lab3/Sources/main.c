/* ###################################################################
**     Filename    : main.c
**     Project     : Lab3
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

#define FLASH_ERASED_MAP 0xFFFFFFFFU
#define CMD_TIME 0x0CU

static volatile uint16union_t *NvTowerNb;
static BOOL Allocate_varTest;

uint16_t *NvTowerNumber, *NvTowerMode;
uint32_t word1, word2;
uint16_t tMode, stuID;
uint8_t TowerMajorVersion =1;
uint8_t TowerMinorVersion= 0;

// Function prototype
void FTM0Callback(const TFTMChannel* const aFTMChannel);

//Declare functions
static BOOL PacketHandler(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL StartupCommand_4(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL ProgramByte_7(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL ReadByte_8(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL SpecialCommand_9(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL SpecialCommand_B(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL TowerNumberCommand_B(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL TowerNumberCommand_C(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
static BOOL TowerModeCommand_D(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);
void PITCallback(void* arg);
void RTCCallback(void* arg);

// Flash
BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data);
FCCOB_t  Flash_Erase_Sector( uint32_t address);
BOOL Flash_Erase(void);

// 1 second timer on FTM0 channel 0
TFTMChannel PacketTimer = {0,
			      CPU_MCGFF_CLK_HZ_CONFIG_0,
			      TIMER_FUNCTION_OUTPUT_COMPARE,
			      TIMER_OUTPUT_HIGH,
			      (void* )&FTM0Callback,
			      &PacketTimer};

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  BOOL success, suc2, suc3;

  static uint16_t hWord1, hWord2;
  static uint32_t word1, word2;

  uint8_t hr,min,sec;

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization ***/

  // Turn on orange LED as an indicator of UART and flash and LED initialise success

  EnterCritical();				// Enter critical component

  if(UART_Init(115200, CPU_BUS_CLK_HZ) && Flash_Init() && LEDs_Init()) LEDs_On(LED_ORANGE);

  // PIT initialisation for green LED toggle on 0.5s interrupts
  success=PIT_Init(CPU_BUS_CLK_HZ,&PITCallback,NULL);				// Initialise PIT0, interrupts enabled, timer not enabled at this point
  PIT_Set(500e6,bFALSE);							// Sets PIT0 count down for interrupts every 0.5s
  PIT_Enable(bTRUE);								// Start the PIT0

  RTC_Init(&RTCCallback, NULL);

  // Student ID
  tMode = 1;
  stuID = 3490;

  word1= _FW(FLASH_DATA_START);
  word2= _FW(FLASH_DATA_START+4);

  // Reads flash (8 bytes) to determine if is has been erased;
  if(word1 == FLASH_ERASED_MAP && FLASH_ERASED_MAP == word2)
  {
    if(Flash_AllocateVar((void*)&NvTowerNumber,sizeof(*NvTowerNumber)))
    {
      Flash_Write16((uint16_t*)NvTowerNumber, stuID);
    }
    if(Flash_AllocateVar((void*)&NvTowerMode,sizeof(*NvTowerMode)))
    {
      Flash_Write16((uint16_t*)NvTowerMode, tMode);
    }
  }

  FTM_Init();
  FTM_StartTimer(&PacketTimer);

  ExitCritical();				// Exit critical component

  // Sends start-up packet
  StartupCommand_4(0x04, 0, 0, 0);

  /* Write your code here */
  for (;;)
  {
    if(Packet_Get())
    {
      PacketHandler(Packet_Command,Packet_Parameter1,Packet_Parameter2, Packet_Parameter3);
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

/*! @brief Commands of Tower serial protocol packet acknowledgment are handled here
 * Function input is processed according to specifications for specific commands, sending
 * a packet response to the PC
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL PacketHandler(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  BOOL success = bFALSE; // Initially success flag set to false

  LEDs_On(LED_BLUE);						// Turns on BLUE LED
  PacketTimer.ioType.inputDetection = TIMER_OUTPUT_HIGH;	// Starts FTM0 timer
  FTM_StartTimer(&PacketTimer);

  // command = Packet_command & ACK_MASK
  switch (command)
  {
    // success = startupcommandX
    case 0x04: case 0x84:
      return StartupCommand_4(command, parameter1, parameter2, parameter3);
      break;
    case 0x07: case 0x87:
      return ProgramByte_7(command, parameter1, parameter2, parameter3);
      break;
    case 0x08: case 0x88:
      return ReadByte_8(command, parameter1, parameter2, parameter3);
      break;
    case 0x09: case 0x89:
      return SpecialCommand_9(command, parameter1, parameter2, parameter3);
      break;
    case 0x0B: case 0x8B:
      return TowerNumberCommand_B(command, parameter1, parameter2, parameter3);
      break;
    case 0x0C: case 0x8C:
      return TowerNumberCommand_C(command, parameter1, parameter2, parameter3);
     break;
    case 0x0D: case 0x8D:
      return TowerModeCommand_D(command, parameter1, parameter2, parameter3);
    default:
      if (command >> 7 == 1)							// Because command is invalid
      {
	Packet_Put(command & 0x7F, parameter1, parameter2, parameter3);		// Because we want to provide acknowledgment for a bad command and return an error
	return bFALSE;
      }
      else									// Because no acknowledgment, do nothing but return an error
      {
	return bFALSE;
      }
      break;
  }
}

/*! @brief Provides instructions for handling startup command 0x04 coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL StartupCommand_4(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  if (command == 0x04)
  {
    if (parameter1 == 0 && parameter2 == 0 && parameter3 == 0)
    {                                                                       	// Valid parameters, failure to output returns a bFALSE
      return (Packet_Put(0x04, 0,0,0) && Packet_Put(0x09,'v',TowerMajorVersion, TowerMinorVersion)
	&& Packet_Put(0x0B,1,0xFF & stuID, stuID >> 8)
	&& Packet_Put(0x0D,1,0xFF & tMode, tMode >> 8));
    }
    else                                                                       	// Invalid parameters flags a problem
    {
      return bFALSE;
    }
  }
  else                                                                        	// Command is 0x84
  {
    if (parameter1 == 0 && parameter2 == 0 && parameter3 ==0)               	// Because parameters are valid
    {
      if (Packet_Put(0x04, 0,0,0) && Packet_Put(0x09,'v',TowerMajorVersion, TowerMinorVersion)
	&& Packet_Put(0x0B,1,0xFF & stuID, stuID >> 8)
	&& Packet_Put(0x0D,1,0xFF & tMode, tMode >> 8))
      {
	Packet_Put(command, 0,0,0);
      }
      else
      {
	Packet_Put(0x04, 0,0,0);
      }
    }
    else                                                                     	// Invalid parameters flags a problem with acknowledgment
    {
      Packet_Put(command & 0x7F, parameter1, parameter2, parameter3);
      return bFALSE;
    }
  }
}

/*! @brief Provides instructions for handling Program Flash 0x07 coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL ProgramByte_7(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  BOOL WriteFlag=0;
  BOOL EraseFlag=0;
  uint8_t parameter3copy;
  uint32_t w1, w2;
  uint8_t b1;
  if (parameter1 < 8 && parameter2 == 0)
  {
    // Returns parameter3 data with parameter1 offset
    WriteFlag=Flash_Write8((uint8_t volatile *)(FLASH_DATA_START+parameter1),parameter3);
    parameter3copy=parameter3;
  }

  else if (parameter1 == 8 && parameter2 == 0)
  {
    EraseFlag=Flash_Erase();
    if (EraseFlag==1)
    {
      parameter3copy=0xFF;							// Data feedback for erase indication in case of acknowledgment
    }
    else
    {
      parameter3copy = parameter3;
    }
  }
  else
  {
    // Invalid parameters, nothing to do
  }

  // Perform acknowledgment tasks if necessary
  if (command >> 7 == 1)							// Need to perform acknowledgments
  {
    if (WriteFlag == 1 | EraseFlag == 1)					// Acknowledge Success
    {
      return Packet_Put(command, parameter1, parameter2, parameter3copy);
    }
    else									// NACK bit8 low
    {
      Packet_Put(command & 0x7F, parameter1, parameter2, parameter3copy);
      return bFALSE;								// Invalid parameters or bad write or bad erase
    }
  }
  else										// No acknowledgment required
  {
    if(WriteFlag == 1 | EraseFlag == 1)						// Success of function, no acknowledgment
    {
      return bTRUE;
    }
    else
    {
      return bFALSE;								// Invalid parameters or bad write or bad erase, no acknowledgment
    }
  }
}

/*! @brief Provides instructions for handling Read Flash 0x07 coming from PC to tower or tower to PC
 * @note Sends data read in address offset of parameter 1 to byte 4 of packet, provided parameter 2 value is 0
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL ReadByte_8(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  uint8_t readFlag = 0;
  uint8_t invalidFlag = 0;
  uint8_t data;
  if (parameter2 == 0)								// Valid parameters received, parameter3 can be anything
  {
    data=_FB(FLASH_DATA_START+parameter1);					// Reads byte with offset from startup flash memory
    readFlag=Packet_Put(command&0x7F, parameter1, parameter2, data);		// Read packet as per command
  }
  else
  {
    invalidFlag=1;
  }
  if (command >> 7 == 1)							// Do the acknowledgments
  {
    if (readFlag == bTRUE)
    {
      return Packet_Put(command, parameter1, parameter2, data);
    }
    else
    {
      Packet_Put(command&0x7F, parameter1, parameter2, data);			// Clear the top bit to indicate failure to read
      return bFALSE;
    }
  }
  else										// No acknowledgments needed
  {
    if (readFlag == bTRUE)
    {
      return bTRUE;
    }
    return bFALSE;
  }
}

/*! @brief Provides instructions for handling special command 0x09 coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL SpecialCommand_9(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  switch(parameter2)                                                           	// Switch used to allow function growth, parameter 2 easily
  // Differentiates between future command 0x09 protocols
  {
    case 'x':
      if(command >> 7 == 1)                                             	// Because we check for acknowledgment flag
      {
	if(parameter1 == 'v' && parameter3 == 0xD)                             	// Valid parameter ASCII values for 'v' and CR (carriage return)
	{
	  return (Packet_Put(0x09,'v',TowerMajorVersion,TowerMinorVersion)
	    && Packet_Put(command, parameter1, parameter2, parameter3));  	// Tower to PC Special-Tower Version command and acknowledgment package
	}
	else                                                                    // Because invalid function parameters with acknowledgment flag
	{
	  Packet_Put(command& 0x7F, parameter1, parameter2, parameter3);
	  return bFALSE;                                                      	// Flag an error after clearing ACK (acknowledgment) bit
	}
      }
      else                                                                	// No acknowledgment required
      {
	if(parameter1 == 'v' && parameter3 == 0xD)                          	// ASCII values for 'v' and CR (carriage return)
	{
	  return Packet_Put(0x09,'v',TowerMajorVersion,TowerMinorVersion);     	// Tower to PC Special - Tower Version command, no acknowledgment
	}
	else                                                                   	// Invalid function parameters
	{
	  return bFALSE;                                                       	// Flag an error without acknowledgment
	}
      }
      break;

    default:                                                                    // Invalid parameters lie here
      if(command >> 7 == 1)                                                  	// Check for acknowledgment flag
      {
	Packet_Put(command& 0x7F, parameter1, parameter2, parameter3);
	return bFALSE;                                                         	// Flag an error after clearing acknowledgment bit
      }
      else
      {
	return bFALSE;                                                         	// Flag an error
      }
      break;
  }
}

/*! @brief Provides instructions for handling tower number command 0x0B coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL TowerNumberCommand_B(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  switch(parameter1)
  {
    case 1:                                                                  	// Get Tower number, note parameter 2 and 3 can be any value
      if(parameter2 == 0 && parameter3 == 0)                                  	// Valid Get Tower Number call
      {
	if(command >> 7 == 1)                                               	// Check for acknowledgment flag
	{
	  return (Packet_Put(0x7F & command, parameter1, 0xFF & stuID, stuID >> 8)
	      && Packet_Put(command, parameter1, parameter2, parameter3));	// Get the tower number and send acknowledgment packet
	}
	else                                                               	// Send Tower to PC Tower version message, no acknowledgment
	{
	  return Packet_Put(command, parameter1, 0xFF & stuID, stuID >> 8);
	}
      }
      else									// Invalid Get Tower Number call
      {
	if(command >> 7 == 1)
	{
	  Packet_Put(0x7F & command, parameter1, parameter2, parameter3);    	// Output acknowledgment packet with top bit cleared
	  return bFALSE;                                                     	// Flag an error (invalid entry)
	}
	else                                                                   	// No acknowledgment
	{
	  return bFALSE;                                                    	// Flag an error (invalid entry)
	}
      }
      break;

    case 2:                                                               	// Set tower number, note parameter 2 and 3 can be any value
      stuID = (parameter3<<8) | parameter2;                            		// Noted parameter2 is LSB, parameter3 is MSB, value stored
      if(command >> 7 == 1)                                                  	// Check for acknowledgment flag
      {
	return (Packet_Put(0x7F & command, 1, parameter2, parameter3)
	  && Packet_Put(command, parameter1, parameter2, parameter3));       	// Set tower number via Tower to PC communications, send acknowledgment packet
      }
      else
      {
	return Packet_Put(command, 1, parameter2, parameter3);                 	// Set tower number via Tower to PC communications, no acknowledgment packet
      }
      break;

    default:                                                            	// Leaves room for other Special commands, invalid parameters lie here
      if(command >> 7 == 1)                                                 	// Check for acknowledgment flag
      {
	Packet_Put(command, 1, parameter2, parameter3);
	return bFALSE;                                                       	// Flag an error after providing acknowledgment packet with cleared bit 7 of command
      }
      else
      {
	return bFALSE;                                                       	// Flag an error
      }
  }
}

/*! @brief Provides instructions for handling tower mode command 0x0C coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL TowerNumberCommand_C(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  RTC_Set(parameter1,parameter2,parameter3);
  return bTRUE;
}



/*! @brief Provides instructions for handling tower mode command 0x0D coming from PC to tower
 * @param command is oldest char in the RxFIFO
 * @param parameter1 is 2nd oldest char in the RxFIFO
 * @param parameter2 is 3rd oldest char in the RxFIFO
 * @param parameter3 is 4th oldest char in the RxFIFO
 * @return BOOL - bTRUE indicates no problems encountered, bFALSE indicates error in putting packet or in parameters received
 */
static BOOL TowerModeCommand_D(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  BOOL invalidFlag = 0;
  BOOL successExec = 0;

  switch(parameter1)
  {
    case 1:									// Get tower mode
      successExec=Packet_Put(0x7F&command, parameter1, 0xFF & tMode, tMode>>8);
      break;
    case 2:									// Set tower mode
      tMode=(parameter3<<8) | parameter2;                            		// Noted parameter2 is LSB, parameter3 is MSB, value stored
      successExec = 1;
      break;
    default:
      invalidFlag = 1;
  }

  if(command >> 7 == 1)								// Acknowledgment flag exists
  {
    if (invalidFlag == 1)
    {
      return bFALSE;
    }
    return Packet_Put(command, parameter1, 0xFF & tMode, tMode >> 8);		// Leave ACK bit high to indicate success
  }
  else										// No acknowledgment required
  {
    if (invalidFlag = 1)
    {
      return bFALSE;
    }
    return bTRUE;
  }
}

/*! @brief User Callback function for PIT called in the PITISR()
 * @param arg is pointer to a argument
 * @note Called following PIT interrupt meaning PIT timer has counted down to zero
 */
void PITCallback(void* arg)
{
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;      						// Writes 1 to clear PIT Channel0 interrupt flag
  while (PIT_TFLG0==1)
  {
    ;										// Wait for the PIT interrupt flag to clear
  }
  LEDs_Toggle(LED_GREEN);							// Because want to blink every two times PIT_ISR() called
}

/*! @brief User Callback function for RTC referenced in the RTCISR()
 * @param arg is pointer to an argument
 * @note Called following RTC interrupt meaning PIT timer has counted down to zero
 */
void RTCCallback(void* arg)
{
  uint8_t hrs, mins, secs;							/*!< Variables declared for storing current time*/

  RTC_Get(&hrs, &mins, &secs);							// Gets current time
  Packet_Put(CMD_TIME, hrs, mins, secs);					// Updates time
  LEDs_Toggle(LED_YELLOW);							// Toggles yellow LED
}

/*! @brief User callback function for FTM0_ISR
 *  @param aFTMChannel Structure containing the parameters used for setting up the timer channel
 *  @note Called following FTM0 interrupt meaning FTM0 timer has counter down to zero
 */
void FTM0Callback(const TFTMChannel* const aFTMChannel)
{
  if (FTM0_CnSC(aFTMChannel->channelNb) & FTM_CnSC_CHF_MASK)			// Checks for the channel flag
  {
    PacketTimer.ioType.inputDetection = TIMER_OUTPUT_DISCONNECT;		// Holds the timer interrupt
    FTM_StartTimer(&PacketTimer);						// Updates time
    LEDs_Off(LED_BLUE);								// Turns off blue LED after the first packet has been received successfully
  }
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
