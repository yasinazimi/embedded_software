/*! @file
 *
 *  @brief Routines to implement a FIFO buffer.
 *
 *  This contains the structure and "methods" for accessing a byte-wide FIFO.
 *
 *  @author PMcL
 *  @date 2015-07-23
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
  if (FIFO->NbBytes >= FIFO_SIZE)
  {
    return bFALSE;
  }
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
  *dataPtr = FIFO->Buffer[FIFO->Start];
  FIFO->Start++;
  FIFO->NbBytes--;
  if (FIFO->Start >= FIFO_SIZE)
  {
    FIFO->Start = 0;
  }
  return bTRUE;
}
