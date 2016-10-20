/*! @file
 *
 *  @brief FIFO buffer function implementations.
 *
 *  Implementation of the FIFO module for handling buffer data.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup FIFO_module FIFO module documentation
 * @{
 */
#include "Cpu.h"
#include "FIFO.h"
#include "PE_Types.h"

void FIFO_Init(TFIFO * const FIFO)
{
  // Initialises head pointer location to 0
  FIFO->Start = 0;
  // Initialises tail pointer location to 0
  FIFO->End = 0;
  // Initialises number of bytes stored in FIFO to 0
  FIFO->NbBytes = 0;
}

BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  // Enter critical component
  EnterCritical();
  // Check if FIFO buffer is full
  if (FIFO->NbBytes == FIFO_SIZE)
  {
    // Exit critical component
    ExitCritical();
    // Returns 0 if buffer is full
    return bFALSE;
  }
  else
  {
    // Place data in FIFO
    FIFO->Buffer[FIFO->End] = data;

    // Resets tail pointer location to 0 (if reached end of buffer)
    if (FIFO->End == FIFO_SIZE-1)
    {
      // Circular buffer wraps around if at end of FIFO buffer
      FIFO->End = 0;
    }
    else
    {
      // Next place in FIFO to write to
      FIFO->End++;
    }

    // Increment bytes in FIFO
    FIFO->NbBytes++;
    // Exit critical component
    ExitCritical();
    // FIFO successfully received a byte
    return bTRUE;
  }
}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  // Enter critical component
  EnterCritical();
  // Check if FIFO is empty
  if (FIFO->NbBytes == 0)
  {
    // Exit critical component
    ExitCritical();
    // Returns 0 if buffer is empty
    return bFALSE;
  }
  else
  {
    // Oldest data in FIFO put to dataPtr
    *dataPtr = FIFO->Buffer[FIFO->Start];

    // Resets head pointer location to 0 (if reached end of buffer)
    if (FIFO->Start == FIFO_SIZE-1)
    {
      // Circular buffer wraps around if at start of FIFO buffer
      FIFO->Start = 0;
    }
    else
    {
      // Increment position of oldest in buffer
      FIFO->Start++;
    }

    // Decrease count of FIFO bytes
    FIFO->NbBytes--;
    // Exit critical component
    ExitCritical();
    // FIFO successfully released byte
    return bTRUE;
  }
}

/*!
 * @}
 */
