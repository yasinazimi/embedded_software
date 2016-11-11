/*! @file UART.c
 *
 *  @brief Implementation of I/O routines for UART communications on the TWR-K70F120M
 *
 *  Implementation of the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
/*!
 * @addtogroup UART_module UART module documentation
 * @{
 */
#include "OS.h"
#include "UART.h"
#include "FIFO.h"
#include "types.h"
#include "packet.h"
#include "MK70F12.h"
#include "PE_Types.h"

static TFIFO TxFIFO, RxFIFO;

/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return BOOL - TRUE if the UART was successfully initialized.
 */
BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  uint32_t brfa, sbr;

  // Initialises FIFO
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);

  // Create semaphores
  RxSemaphore = OS_SemaphoreCreate(0);
  TxSemaphore = OS_SemaphoreCreate(0);

  // UART setup
  // Enable system clock gate for UART2
  SIM_SCGC4 |= SIM_SCGC4_UART2_MASK;
  // Enable system clock gate for PORTE
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
  // Set pin control register 16 PORTE bits 8 and 9, to enable MUX alternative 3 (Transmitter)
  PORTE_PCR16 |= PORT_PCR_MUX(3);
  // Set pin control register 17 PORTE bits 8 and 9, to enable MUX alternative 3 (Receiver)
  PORTE_PCR17 |= PORT_PCR_MUX(3);

  //setting baud rate
  // To turn Transmitter Enable bit 3 (TE) off before setting baud rate
  UART2_C2 &= ~UART_C2_TE_MASK;
  // To turn Receiver Enable bit 2 (RE) off before setting baud rate
  UART2_C2 &= ~UART_C2_RE_MASK;

  // Calculate variables to configure baud rate
  sbr  = (uint16_t)(moduleClk / (16 * baudRate));
  // Calculate the baud rate fine adjust
  brfa = 2 * (moduleClk / baudRate) - 32 * sbr;

  // Because ((2^13)-1) didn't work, wanted to indicate maximum is limit of 13 bit unsigned number
  if(sbr > 8191)
  {
    // Baud rate is too low, meaning unsigned 13 bit SBR limit is exceeded, UART_Init fails
    return bFALSE;
  }
  else
  {
    // Reset register (debugging step), we may assume TE and RE bits are turned off)
    UART2_BDL = 0x0;
    // 'hi' because register has been reset, and prior error checking ensures UART baud rate bits 4:0 (SBR) will only be written to
    UART2_BDH = UART2_BDH | (sbr >> 8);
    // Because setting an 8 bit register
    UART2_BDL = UART2_BDL | ( (uint8_t)(sbr & 0xFF));
  }

  // C1 control registers
  UART2_C1 &= ~UART_C1_PT_MASK;         // Disabled
  UART2_C1 &= ~UART_C1_PE_MASK;         // Disabled
  UART2_C1 &= ~UART_C1_ILT_MASK;        // Disabled
  UART2_C1 &= ~UART_C1_WAKE_MASK;       // Disabled
  UART2_C1 &= ~UART_C1_M_MASK;          // Disabled
  UART2_C1 &= ~UART_C1_RSRC_MASK;       // Disabled
  UART2_C1 &= ~UART_C1_UARTSWAI_MASK;   // Disabled
  UART2_C1 &= ~UART_C1_LOOPS_MASK;      // Disabled

  // C2 control registers
  UART2_C2 &= ~UART_C2_SBK_MASK;        // Disabled
  UART2_C2 &= ~UART_C2_RWU_MASK;        // Disabled
  UART2_C2 &= ~UART_C2_ILIE_MASK;       // Disabled
  UART2_C2 &= ~UART_C2_TIE_MASK;        // Disabled

  // Enable interrupts
  UART2_C2 |= UART_C2_TCIE_MASK;       	// Disabled
  UART2_C2 |= UART_C2_RIE_MASK;         // Enable receive (RDRF) interrupt

  // NVIC Register Mask
  NVICICPR1 = (1<<(49 % 32));		// Clear any pending error status sources interrupts on UART2
  NVICISER1 = (1<<(49 % 32));		// Enable error status sources interrupts from UART2

  // UART receive and transmit
  UART2_C2 |= UART_C2_RE_MASK;          // Enable UART2 receive
  UART2_C2 |= UART_C2_TE_MASK;          // Enabled UART2 transmit

  return bTRUE;
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return void.
 */
void UART_InChar(uint8_t * const dataPtr)
{
  FIFO_Get(&RxFIFO, dataPtr);
}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return void.
 */
void UART_OutChar(const uint8_t data)
{
  FIFO_Put(&TxFIFO, data);
}

/*! @brief UART transmit thread
 *
 *  @return void
 */
void TxThread(void* arg)
{
  uint8_t txData;

  for (;;)
  {
    // Semaphore to wait for the transmit
    OS_SemaphoreWait (TxSemaphore, 0);
    // Put data in the UART2_D register
    FIFO_Get(&TxFIFO, &txData);
    UART2_D = txData;
    UART2_C2 |= UART_C2_TCIE_MASK;
  }
}

/*! @brief UART receive thread
 *
 *  @return void
 */
void RxThread(void* arg)
{
  for (;;)
  {
    // Semaphore to wait for the receive
    OS_SemaphoreWait(RxSemaphore, 0);
    // Puts bytes UART2_D register upon receive
    FIFO_Put(&RxFIFO, UART2_D);
    UART2_C2 |= UART_C2_RIE_MASK;
  }
}

/*! @brief Interrupt service routine for UART2
 *
 *  @return void
 *  @note vectors.c updated
 */
void __attribute__ ((interrupt)) UART_ISR(void)
{
  OS_ISREnter();

  // Receive a character
  if (UART2_C2 & UART_C2_RIE_MASK)
  {
    // Clear RDRF flag by reading the status register
    if (UART2_S1 & UART_S1_RDRF_MASK)
    {
      // Semaphore signals the receive
      OS_SemaphoreSignal(RxSemaphore);
      // Disable RIE after bytes have been received
      UART2_C2 &= ~UART_C2_RIE_MASK;
    }
  }

  // Transmit a character
  if (UART2_C2 & UART_C2_TCIE_MASK)
  {
    // Clear TDRE flag by reading the status register
    if (UART2_S1 & UART_S1_TC_MASK)
    {
      // Semaphore signals the receive
      OS_SemaphoreSignal(TxSemaphore);
      // Disable TCIE after bytes have been transmitted
      UART2_C2 &= ~UART_C2_TCIE_MASK;
    }
  }

  OS_ISRExit();
}

/*!
 * @}
 */
