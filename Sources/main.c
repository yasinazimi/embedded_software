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
#include "packet.h"
#include "UART.h"
#include "cmd.h"

// Setting the baud rate and the module clock
const uint32_t BAUD_RATE = 38400;
const uint32_t MODULE_CLOCK = CPU_BUS_CLK_HZ;

// Acknowledgment bit mask
const uint8_t PACKET_ACK_MASK = 0x80;

void Packet_Handle()
{
  BOOL error = bTRUE;
  switch(Packet_Command & ~PACKET_ACK_MASK)	// Mask out the acknowledgement, otherwise it goes to default
  {
    case CMD_RX_GET_SPECIAL_START_VAL:
      Send_Startup();
      Send_Special_Tower_Version();
      Send_Tower_Number();
      error = bFALSE;
      break;
    case CMD_RX_GET_VERSION:
      Send_Special_Tower_Version();
      error = bFALSE;
      break;
    case CMD_RX_TOWER_NUMBER:
      if (Packet_Parameter1 == CMD_TOWER_NUMBER_GET)
      {
	Send_Tower_Number();
      }
      else if (Packet_Parameter1 == CMD_TOWER_NUMBER_SET)
      {
	Receive_Tower_Number(Packet_Parameter2, Packet_Parameter3);
      }
      error = bFALSE;
      break;
    default:
      break;
  }
  if (Packet_Command & PACKET_ACK_MASK)
    {
      uint8_t maskedPacket = 0;
      if (error == bTRUE)
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
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  Packet_Init(BAUD_RATE, MODULE_CLOCK);

  Send_Startup();
  Send_Special_Tower_Version();
  Send_Tower_Number();

  for (;;)
  {
      UART_Poll();
      if (Packet_Get())
      {
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
