/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  @author Mohammad Yasin Azimi [11733490], Micheal Codner [11989668]
 *  @date 2015-07-23
 */
/*!
 ** @addtogroup uart_module UART module documentation
 ** @{
 */

// new types
#include "UART.h"
#include "MK70F12.h"


BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  // UART setup

  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;	// Enable system clock gate for UART2
  SIM_SCGC5 |= SIM_SCGC5_UART2_MASK;	// Enable system clock gate for PORTE
  PORTE_PCR16 |= PORT_PCR_MUX(3);	// Set pin control register 16 PORTE bits 8 and 9, to enable MUX alternative 3 (Transmitter)
  PORTE_PCR17 |= PORT_PCR_MUX(3);	// Set pin control register 17 PORTE bits 8 and 9, to enable MUX alternative 3 (Receiver)
  UART2_C1 = 0;				// Clears UART2 control register 1
  UART2_C2 |= UART_C2_TE_MASK;		// Enable UART2 for Transmit
  UART2_C2 |= UART_C2_RE_MASK;		// Enable UART2 for Receive

  // Requested Baud Rate setup

  uint16_t setting  = (uint16_t)(moduleClk/(baudRate * 16));	// Setting the baud rate which is synchronized with the module clock
  uint8_t setting_high = (setting & 0x1F00) >> 8;		// Setting the the upper 8 bits of the modulus counter
  uint8_t setting_low = (setting & 0xFF);			// Masked to get lower 8 bits of the baud rate
  // Updating the 13-bit baud rate setting
  UART2_BDH = (UART2_BDH & 0XC0) | (setting_high & 0x1F);	// Buffers the high half of the new value
  UART2_BDL = setting_low;					// Reset to a nonzero value


}


BOOL UART_InChar(uint8_t * const dataPtr)
{

}


BOOL UART_OutChar(const uint8_t data)
{

}


void UART_Poll(void)
{

}
