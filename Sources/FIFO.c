/*! @file
 *
 *  @brief Routines to implement a FIFO buffer
 *
 *  @author Mohammad Yasin Azimi (11733490), Micheal Codner (11989668)
 *  @date 03-08-2016
 */
/*!
 ** @addtogroup fifo_module FIFO module documentation
 ** @{
 */

#include "FIFO.h"

void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  // Checks the position of FIFO in conjunction with the number of NbBytes in the cyclic buffer
  if (FIFO->NbBytes >= FIFO_SIZE)
  {
    return bFALSE;
  }
  // Puts the data in the buffer
  // Increment the number of End and NbBytes in the cyclic buffer
  // Checks the position of FIFO in conjunction with End
  FIFO->Buffer[FIFO->End] = data;
  FIFO->End++;
  FIFO->NbBytes++;
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
    return bFALSE;
  }

  // Increment the position of the oldest byte in the FIFO
  // Decrement the current number NbBytes from the FIFO
  *dataPtr = FIFO->Buffer[FIFO->Start];
  FIFO->Start++;
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
