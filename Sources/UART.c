/*! @file
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M
 *
 *  UART module implementation for handling UART communication
 *
 *  @author Mohammad Yasin Azimi (11733490), Micheal Codner (11989668)
 *  @date 03-08-2016
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
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;	// Enable system clock gate for PORTE
  PORTE_PCR16 |= PORT_PCR_MUX(3);	// Set pin control register 16 PORTE bits 8 and 9, to enable MUX alternative 3 (Transmitter)
  PORTE_PCR17 |= PORT_PCR_MUX(3);	// Set pin control register 17 PORTE bits 8 and 9, to enable MUX alternative 3 (Receiver)


  //ENABLING C1 CONTROL REGISTERS
    /*
     ** In this section we need to set or clear each bit in each register
     ** of the UART. See page 1910 for the C1 explanation.
     ** EXAMPLE:
     ** UART_C1 &= ~UART_C1_PT_MASK;  this is a 0
     ** UART_C1 |= UART_C1_PT_MASK;  this is a 1
     **
     */

    UART2_C1 &= ~UART_C1_PT_MASK; 	// Disabled
    UART2_C1 &= ~UART_C1_PE_MASK; 	// Disabled
    UART2_C1 &= ~UART_C1_ILT_MASK;	// Disabled
    UART2_C1 &= ~UART_C1_WAKE_MASK; 	// Disabled
    UART2_C1 &= ~UART_C1_M_MASK;        // Disabled
    UART2_C1 &= ~UART_C1_RSRC_MASK; 	// Disabled
    UART2_C1 &= ~UART_C1_UARTSWAI_MASK; // Disabled
    UART2_C1 &= ~UART_C1_LOOPS_MASK; 	// Disabled

    //ENABLING C2 CONTROL REGISTERS
    UART2_C2 &= ~ UART_C2_SBK_MASK; 	// Disabled
    UART2_C2 &= ~ UART_C2_RWU_MASK; 	// Disabled
    UART2_C2 |= UART_C2_RE_MASK; 	// Enabled - Receiver
    UART2_C2 |= UART_C2_TE_MASK; 	// Enabled - Transmitter
    UART2_C2 &= ~ UART_C2_ILIE_MASK; 	// Disabled
    UART2_C2 &= ~ UART_C2_RIE_MASK; 	// Disabled
    UART2_C2 &= ~ UART_C2_TCIE_MASK; 	// Disabled
    UART2_C2 &= ~ UART_C2_TIE_MASK; 	// Disabled

    UART2_C4 |= UART_C4_BRFA_MASK;

  // Requested Baud Rate setup

  uint16_t setting  = (uint16_t)(moduleClk/(baudRate * 16));	// Setting the baud rate which is synchronized with the module clock
  uint8_t setting_high = (setting & 0x1F00) >> 8;		// Setting the the upper 8 bits of the modulus counter
  uint8_t setting_low = (setting & 0xFF);			// Masked to get lower 8 bits of the baud rate

  // U-g

  UART2_BDH = setting_high & 0x1F;	// Buffers the high half of the new value
  UART2_BDL = setting_low;		// Reset to a nonzero value (fraction of 4/32)

  // Initializes the FIFO buffers
  FIFO_Init(&RxFIFO);
  FIFO_Init(&TxFIFO);

  return bTRUE;
}

// Fetching the next byte from the receive FIFO
BOOL UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);
}

// Transmit the next byte in the transmit FIFO
BOOL UART_OutChar(const uint8_t data)
{
  return FIFO_Put(&TxFIFO, data);
}

//
void UART_Poll(void)
{
  // Checks for the status of the transmit register
  if (UART2_S1 & UART_S1_TDRE_MASK)
  {
    FIFO_Get(&TxFIFO, &UART2_D);
  }

  // Checks for the status of the receive register
  if (UART2_S1 & UART_S1_RDRF_MASK)
  {
    FIFO_Put(&RxFIFO, UART2_D);
  }
}

/*!
** @}
*/
