/*! @file UART.c
 *
 *  @brief Implementation of I/O routines for UART communications on the TWR-K70F120M
 *
 *  Implementation of the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup UART_module UART module documentation
 * @{
 */
#include "UART.h"
#include "FIFO.h"
#include "types.h"
#include "MK70F12.h"
#include "packet.h"

static TFIFO RxFIFO, TxFIFO;

/*! @brief Sets up the UART interface before first use.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return BOOL - TRUE if the UART was successfully initialized.
 */
BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  uint32_t brfa, sbr;

  TxSemaphore = OS_SemaphoreCreate(0);
  RxSemaphore = OS_SemaphoreCreate(0);

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

  //Determine SBR and BRFA
  // Integer division gives floor value required for SBR, in special case with remainder 63/64 it will increment by one in if statement below
  sbr = (moduleClk / baudRate) / 16;
  // Because Baud Rate Fine Adjust is temporarily set as fraction of 64
  brfa = ( ( (64 * moduleClk) / baudRate) / 16 ) % 64;

  // Baud rate fine adjust
  // Calls a function that helps to simplify UART_Init
  brfa = FractionOf64to32(brfa, &sbr);
  // UART2_C4 reset default values assumed,  used only for bits 4-0 for Baud Rate Fine Adjust (BRFA)
  UART2_C4 = UART2_C4 | (uint8_t)brfa;

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
  // Clear any pending error status sources interrupts on UART2
  NVICICPR1 = (1<<(49 % 32));
  // Enable error status sources interrupts from UART2
  NVICISER1 = (1<<(49 % 32));

  // Initializes the FIFO buffers
  //  FIFO_Init(&TxFIFO);
  //  FIFO_Init(&RxFIFO);

  UART2_C2 |= UART_C2_RE_MASK;          // Enable UART2 receive
  UART2_C2 |= UART_C2_TE_MASK;          // Enabled UART2 transmit

  FIFO_Init(&RxFIFO);
  FIFO_Init(&TxFIFO);

  return bTRUE;
}

/*! @brief Get a character from the receive FIFO if it is not empty.
 *
 *  @param dataPtr A pointer to memory to store the retrieved byte.
 *  @return BOOL - TRUE if the receive FIFO returned a character.
 *  @note Assumes that UART_Init has been called.
 */
void UART_InChar(uint8_t * const dataPtr)
{
  // Noted error checking performed inside FIFO_Get which returns desired BOOLean output for UART_InChar
  FIFO_Get(&RxFIFO, dataPtr);
}

/*! @brief Put a byte in the transmit FIFO if it is not full.
 *
 *  @param data The byte to be placed in the transmit FIFO.
 *  @return BOOL - TRUE if the data was placed in the transmit FIFO.
 *  @note Assumes that UART_Init has been called.
 */
void UART_OutChar(const uint8_t data)
{
  //
  FIFO_Put(&TxFIFO, data);
}

/*! @brief UART transmit thread
 *  @return void
 */
void TxThread(void* arg)
{
  uint8_t txData;
  // FIFO_Get (&TxFIFO, UART2_D);

  for (;;)
  {
    // Semaphore to wait for the transmit
    OS_SemaphoreWait (TxSemaphore, 0);

    // Obtains bytes for transmit
    FIFO_Get(&TxFIFO, &txData);
    // UART2_D register
    UART2_D = txData;
    // Enable TCIE MASK
    UART2_C2 |= UART_C2_TCIE_MASK;
  }
}

/*! @brief UART receive thread
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
    // Semaphore signals the packet semaphore
    OS_SemaphoreSignal(PacketSemaphore);
    // Enable RIE MASK for receive
    UART2_C2 |= UART_C2_RIE_MASK;
  }
}

/*! @brief Interrupt service routine for UART2
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

/*! @brief Calculation of the baud rate
 *
 *  @param brfa The baud rate fine adjust
 *  @param sbrPtr The baud rate pointer
 */
static uint32_t FractionOf64to32(uint32_t brfa,  uint32_t * sbrPtr)
{
  if( brfa == 63 )
  {
    // For special case decimal exceeds 63/64, round up SBR to next full integer
    brfa = 0;
    *sbrPtr = *sbrPtr + 1;
  }
  else
  {
    if(brfa % 2 == 0)
    {
      // Because want to change fraction of 64 to fraction of 32
      brfa = brfa / 2;
    }
    else
    {
      // Round up fraction of 32 for accuracy
      brfa = (brfa / 2) + 1;
    }
  }
  return brfa;
}

/*!
 * @}
 */
