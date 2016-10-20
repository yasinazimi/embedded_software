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
#include "MK70F12.h"
#include "FIFO.h"

static uint32_t FractionOf64to32(uint32_t brfa,  uint32_t * sbrPtr);

BOOL UART_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  uint32_t brfa, sbr;

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
  UART2_C2 = UART2_C2 & ~(UART_C2_TE_MASK);
  // To turn Receiver Enable bit 2 (RE) off before setting baud rate
  UART2_C2 = UART2_C2 & ~(UART_C2_RE_MASK);

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
  UART2_C2 &= ~UART_C2_TCIE_MASK;       // Disabled
  UART2_C2 &= ~UART_C2_TIE_MASK;        // Disabled
  UART2_C2 |= UART_C2_RE_MASK;          // Enable UART2 receive
  UART2_C2 |= UART_C2_TE_MASK;          // Enabled UART2 transmit

  // Enable interrupts
  UART2_C2 |= UART_C2_RIE_MASK;         // Enable receive (RDRF) interrupt

  // NVIC Register Mask
  // Clear any pending error status sources interrupts on UART2
  NVICICPR1 = (1<<(49 % 32));
  // Enable error status sources interrupts from UART2
  NVICISER1 = (1<<(49 % 32));

  // Initializes the FIFO buffers
  FIFO_Init(&TxFIFO);
  FIFO_Init(&RxFIFO);

  return bTRUE;
}

BOOL UART_InChar(uint8_t * const dataPtr)
{
  // Noted error checking performed inside FIFO_Get which returns desired BOOLean output for UART_InChar
  return FIFO_Get(&RxFIFO, dataPtr);
}

BOOL UART_OutChar(const uint8_t data)
{
  BOOL success;
  // Disable TDRE interrupts prior to putting byte to the TxFIFO
  UART2_C2 &= ~UART_C2_TIE_MASK;
  success = FIFO_Put(&TxFIFO, data);
  // Enable TDRE interrupts after putting byte to TxFIFO
  UART2_C2 |= UART_C2_TIE_MASK;
  return success;
}

void __attribute__ ((interrupt)) UART_ISR(void)
{
  static uint8_t txData;
  uint8_t TEMPregisterRead;

  if (UART2_S1 & UART_C2_RIE_MASK)
  {

    if (UART2_S1 & UART_S1_RDRF_MASK)
    {
      // In case of receiving data register full, put the data from full UART input to RxFIFO
      FIFO_Put(&RxFIFO, UART2_D);
    }
  }

  // Register is empty
  if (UART2_S1 & UART_S1_TDRE_MASK)
  {
    // Checks if can write to an intermediate value then to UART output
    if (FIFO_Get(&TxFIFO, &txData))
    {
      UART2_D = txData;
      // In case transmit data register empty, put the data to from TxFIFO to UART output
      TEMPregisterRead = UART2_S1;
    }
    // Can't write to UART output
    else
    {
      // Disable TDRE interrupts until next OutChar call from TxFIFO, protects from an empty TxFIFO
      UART2_C2 &= ~UART_C2_TIE_MASK;
      TEMPregisterRead = UART2_S1;
    }
  }
}

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
