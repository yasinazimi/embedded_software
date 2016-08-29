/* ###################################################################
**     Filename    : main.c
**     Project     : Lab1
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
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */

// CPU module - contains low level hardware initialization routines
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "Cpu.h"
#include "Events.h"
#include "LEDs.h"
#include "flash.h"
#include "packet.h"
#include "UART.h"
#include "cmd.h"

#define BLINKENLIGHTS

// Setting the baud rate and the module clock
const uint32_t BAUD_RATE = 115200;
const uint32_t MODULE_CLOCK = CPU_BUS_CLK_HZ;

// Acknowledgment bit mask
const uint8_t PACKET_ACK_MASK = 0x80;

// Handles incoming packets
void Packet_Handle()
{
  BOOL error = bTRUE;
  uint8_t data;
  // Mask out the acknowledgement, otherwise it goes to default
  switch(Packet_Command & ~PACKET_ACK_MASK)
  {
    case CMD_Receive_Get_Special_Start_Val:
      CMD_Send_Startup();
      CMD_Send_Special_Tower_Version();
      CMD_Send_Tower_Number();
      CMD_Send_Tower_Mode();
      error = bFALSE;
      break;
    case CMD_Receive_Flash_Program_Byte:
      error = !CMD_Receive_Flash_Program_Byte(Packet_Parameter1, Packet_Parameter3);
      break;
    case CMD_Receive_Flash_Read_Byte:
	  error = !CMD_Receive_Flash_Read_Byte(Packet_Parameter1, &data);
      if (error == bFALSE)
      {
        error = !CMD_Send_Flash_Read_Byte(Packet_Parameter1, data);
      }
	case CMD_Receive_Get_Version:
		error = !CMD_Send_Special_Tower_Version();
		break;
	case CMD_Receive_Tower_Number:
		if (Packet_Parameter1 == CMD_Tower_Number_Get)
		{
			error = !CMD_Send_Tower_Number();
		}
		else if (Packet_Parameter1 == CMD_Tower_Number_Set)
		{
			error = !CMD_Receive_Tower_Number(Packet_Parameter2, Packet_Parameter3);
		}
		break;
	case CMD_Receive_Tower_Mode:
		if (Packet_Parameter1 == CMD_Tower_Mode_Get)
		{
			error = !CMD_Send_Tower_Mode();
		}
		else if (Packet_Parameter1 == CMD_Tower_Number_Set)
		{
			error = !CMD_Receive_Tower_Mode(Packet_Parameter2, Packet_Parameter3);
		}
	    break;
	default:
		break;
  }
  
#ifdef BLINKENLIGHTS
  //Error visualization
  if (!error)
  {
    LEDs_Off(LED_YELLOW);
    LEDs_On(LED_GREEN);
  }
  else
  {
    LEDs_On(LED_YELLOW);
    LEDs_Off(LED_GREEN);
  }
#endif

  if (Packet_Command & PACKET_ACK_MASK)
  {
    uint8_t maskedPacket = 0;
    if (error)
    {
      maskedPacket = Packet_Command & ~PACKET_ACK_MASK;
    }
    else
    {
      maskedPacket = Packet_Command | PACKET_ACK_MASK;
    }
      Packet_Put(maskedPacket, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
  }
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
/*!
 * @brief The entry point into the program
 */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  Packet_Init(BAUD_RATE, MODULE_CLOCK);
  CMD_Init();
  LEDs_Init();
  Flash_Init();

  CMD_Send_Startup();
  CMD_Send_Special_Tower_Version();
  CMD_Send_Tower_Number();

  for (;;)
  {
    UART_Poll();
    if (Packet_Get())
    {
#ifdef BLINKENLIGHTS
      LEDs_Toggle(LED_BLUE);
#endif
      Packet_Handle();
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
