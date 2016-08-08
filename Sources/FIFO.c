/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  @author Mohammad Yasin Azim [11733490], Micheal Codner [11989668]
 *  @date 2015-07-23
 */
/*!
 ** @addtogroup fifo_module FIFO module documentation
 ** @{
 */

#include "FIFO.h"

void FIFO_Init(TFIFO * const FIFO)
{
  // Initializing FIFO
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  // Checks the position of FIFO in conjunction with the number of NbBytes in the cyclic buffer
  if (FIFO->NbBytes >= FIFO_SIZE)
  {
      // Error
    return bFALSE;
  }
  // Puts the data in the buffer
  FIFO->Buffer[FIFO->End] = data;
  // Increment the number of End and NbBytes in the cyclic buffer
  FIFO->End++;
  FIFO->NbBytes++;
  // Checks the position of FIFO in conjunction with End
  if (FIFO->End >= FIFO_SIZE)
  {
    FIFO->End = 0;
  }
  return bTRUE;
}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  if (!FIFO->NbBytes)
  {
      // Error
    return bFALSE;
  }

  *dataPtr = FIFO->Buffer[FIFO->Start];
  // Increment the position of the oldest byte in the FIFO
  FIFO->Start++;
  // Decrement the current number NbBytes from the FIFO
  FIFO->NbBytes--;
  //
  if (FIFO->Start >= FIFO_SIZE)
  {
    FIFO->Start = 0;
  }
  return bTRUE;
}

/*!
 ** @}
 */
