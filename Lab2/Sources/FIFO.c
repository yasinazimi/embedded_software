/*! @file
 *
 *  @brief FIFO buffer function implementations.
 *
 *  Implementation of the FIFO module for handling buffer data.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-22
 */
/*!
 * @addtogroup FIFO_module FIFO module documentation
 * @{
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
  if (FIFO->NbBytes >= FIFO_SIZE)	// Checks the position of FIFO in conjunction with the number of NbBytes in the cyclic buffer
  {
    return bFALSE;
  }

  FIFO->Buffer[FIFO->End] = data;	// Puts the data in the buffer
  FIFO->End++;				// Increment the number of End in the cyclic buffer
  FIFO->NbBytes++;			// Increment the number of NbBytes in the cyclic buffer

  if (FIFO->End >= FIFO_SIZE)		// Checks the position of FIFO in conjunction with End
  {
    FIFO->End = 0;
  }
  return bTRUE;
}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  if (!FIFO->NbBytes)			// Checks whether FIFO does not match the current number of bytes in the buffer
  {
    return bFALSE;
  }

  *dataPtr = FIFO->Buffer[FIFO->Start];	// Pointer to the FIFO buffer
  FIFO->Start++;			// Increments the position of the oldest byte in the FIFO
  FIFO->NbBytes--;			// Decrements the current number of NbBytes from the FIFO

  if (FIFO->Start >= FIFO_SIZE)		// Checks the current position of FIFO
  {
    FIFO->Start = 0;
  }
  return bTRUE;
}

/*!
 * @}
*/
