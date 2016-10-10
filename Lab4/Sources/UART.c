/*! @file UART.c
 *
 *  @brief Implementation of I/O routines for UART communications on the TWR-K70F120M
 *
 *  Implementation of the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
/*!
 * @addtogroup UART_module UART module documentation
 * @{
*/
#include "UART.h"
#include "MK70F12.h"
#include "FIFO.h"

static uint32_t FractionOf64to32(uint32_t brfa,  uint32_t * sbrPtr);

/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return BOOL - TRUE if the UART was successfully initialized.
 */
BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  uint32_t brfa, sbr;

  // UART setup
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;			// Enable system clock gate for UART2
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;			// Enable system clock gate for PORTE
  PORTE_PCR16 |= PORT_PCR_MUX(3);			// Set pin control register 16 PORTE bits 8 and 9, to enable MUX alternative 3 (Transmitter)
  PORTE_PCR17 |= PORT_PCR_MUX(3);			// Set pin control register 17 PORTE bits 8 and 9, to enable MUX alternative 3 (Receiver)

  //setting baud rate
  UART2_C2 = UART2_C2 & ~(UART_C2_TE_MASK);       	// To turn Transmitter Enable bit 3 (TE) off before setting baud rate
  UART2_C2 = UART2_C2 & ~(UART_C2_RE_MASK);          	// To turn Receiver Enable bit 2 (RE) off before setting baud rate

  //Determine SBR and BRFA
  sbr = (moduleClk / baudRate) / 16;                    // Integer division gives floor value required for SBR, in special case with remainder 63/64 it will increment by one in if statement below
  brfa = ( ( (64 * moduleClk) / baudRate) / 16 ) % 64;  // Because Baud Rate Fine Adjust is temporarily set as fraction of 64

  // Baud rate fine adjust
  brfa = FractionOf64to32(brfa, &sbr);                  // Calls a function that helps to simplify UART_Init
  UART2_C4 = UART2_C4 | (uint8_t)brfa;                 	// UART2_C4 reset default values assumed,  used only for bits 4-0 for Baud Rate Fine Adjust (BRFA)

  if(sbr > 8191)
  {                                 			// Because ((2^13)-1) didn't work, wanted to indicate maximum is limit of 13 bit unsigned number
    return bFALSE;                                	// Baud rate is too low, meaning unsigned 13 bit SBR limit is exceeded, UART_Init fails
  }
  else
  {
    UART2_BDL = 0x0;                                 	// Reset register (debugging step), we may assume TE and RE bits are turned off)
    UART2_BDH = UART2_BDH | (sbr >> 8);               	// 'hi' because register has been reset, and prior error checking ensures UART baud rate bits 4:0 (SBR) will only be written to
    UART2_BDL = UART2_BDL | ( (uint8_t)(sbr & 0xFF)); 	// Because setting an 8 bit register
  }

  // C1 control registers
  UART2_C1 &= ~UART_C1_PT_MASK; 			// Disabled
  UART2_C1 &= ~UART_C1_PE_MASK; 			// Disabled
  UART2_C1 &= ~UART_C1_ILT_MASK;			// Disabled
  UART2_C1 &= ~UART_C1_WAKE_MASK; 			// Disabled
  UART2_C1 &= ~UART_C1_M_MASK;      			// Disabled
  UART2_C1 &= ~UART_C1_RSRC_MASK; 			// Disabled
  UART2_C1 &= ~UART_C1_UARTSWAI_MASK; 			// Disabled
  UART2_C1 &= ~UART_C1_LOOPS_MASK; 			// Disabled

  // C2 control registers
  UART2_C2 &= ~UART_C2_SBK_MASK; 			// Disabled
  UART2_C2 &= ~UART_C2_RWU_MASK; 			// Disabled
  UART2_C2 &= ~UART_C2_ILIE_MASK; 			// Disabled
  UART2_C2 &= ~UART_C2_TCIE_MASK; 			// Disabled
  UART2_C2 &= ~UART_C2_TIE_MASK; 			// Disabled
  UART2_C2 |= UART_C2_RE_MASK; 				// Enable UART2 receive
  UART2_C2 |= UART_C2_TE_MASK; 				// Enabled UART2 transmit

  // Enable interrupts
  UART2_C2 |= UART_C2_RIE_MASK; 			// Enable receive (RDRF) interrupt

  // NVIC Register Mask
  NVICICPR1 = (1<<(49 % 32));                   	// Clear any pending error status sources interrupts on UART2
  NVICISER1 = (1<<(49 % 32));                   	// Enable error status sources interrupts from UART2

  // Initializes the FIFO buffers
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);
  return bTRUE;
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return BOOL - TRUE if the receive FIFO returned a character.
 *  @note Assumes that UART_Init has been called.
 */
BOOL UART_InChar(uint8_t * const dataPtr)
{
  return FIFO_Get(&RxFIFO, dataPtr);                 	// Noted error checking performed inside FIFO_Get which returns desired BOOLean output for UART_InChar
}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return BOOL - TRUE if the data was placed in the transmit FIFO.
 *  @note Assumes that UART_Init has been called.
 */
BOOL UART_OutChar(const uint8_t data)
{
  //todo: entercrit
  BOOL success;
  UART2_C2 &= ~UART_C2_TIE_MASK; 			// Disable TDRE interrupts prior to putting byte to the TxFIFO
  success = FIFO_Put(&TxFIFO, data);
  UART2_C2 |= UART_C2_TIE_MASK;                 	// Enable TDRE interrupts after putting byte to TxFIFO
  return success;
}

/*! @brief Interrupt service routine for UART2
 *  @return void
 *  @note vectors.c updated
 */
void __attribute__ ((interrupt)) UART_ISR(void)
{
  static uint8_t txData;
  uint8_t TEMPregisterRead;

  if (UART2_S1 & UART_C2_RIE_MASK)
    {
  if (UART2_S1 & UART_S1_RDRF_MASK)			// It is known that RDRF interrupts enabled in UART_Init
   {
     FIFO_Put(&RxFIFO, UART2_D);			// In case of receiving data register full, put the data from full UART input to RxFIFO
   }
}

  if (UART2_S1 & UART_S1_TDRE_MASK)			// Register is empty
  {
    if (FIFO_Get(&TxFIFO, &txData))			// Checks if can write to an intermediate value then to UART output
    {
      UART2_D = txData;
      TEMPregisterRead = UART2_S1;			// In case transmit data register empty, put the data to from TxFIFO to UART output
    }
    else						// Can't write to UART output
    {
      UART2_C2 &= ~UART_C2_TIE_MASK;			// Disable TDRE interrupts until next OutChar call from TxFIFO, protects from an empty TxFIFO
      TEMPregisterRead = UART2_S1;
    }
  }
}

static uint32_t FractionOf64to32(uint32_t brfa,  uint32_t * sbrPtr)
{
  if( brfa == 63 )
  {
    brfa = 0;                                 		// For special case decimal exceeds 63/64, round up SBR to next full integer
    *sbrPtr = *sbrPtr + 1;
  }
  else
  {
    if(brfa % 2 == 0)
    {
      brfa = brfa / 2;                           	// Because want to change fraction of 64 to fraction of 32
    }
    else
    {
      brfa = (brfa / 2) + 1;                     	// Round up fraction of 32 for accuracy
    }
  }
  return brfa;
}

/*!
 * @}
*/
