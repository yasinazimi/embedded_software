/*! @file
 *
 *  @brief FIFO buffer function implementations.
 *
 *  Implementation of the FIFO module for handling buffer data.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
/*!
 * @addtogroup FIFO_module FIFO module documentation
 * @{
*/
#include "FIFO.h"
#include "CPU.h"

void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
}

BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  // EnterCritical();
  if (FIFO->NbBytes == FIFO_SIZE)  		// Check if FIFO buffer is full
  {
    // ExitCritical();
    return bFALSE;
  }
  else
  {
    FIFO->Buffer[FIFO->End] = data; 		// Place data in FIFO
    if (FIFO->End == FIFO_SIZE-1)
    {
      FIFO->End = 0;                		// Circular buffer wraps around if at end of FIFO buffer
    }
    else
    {
      FIFO->End++;                  		// Next place in FIFO to write to
    }
    FIFO->NbBytes++; 				// Increment bytes in FIFO
    // ExitCritical();
    return bTRUE;
  }
}

BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  //EnterCritical();
  if (FIFO->NbBytes == 0) 			// Check if FIFO is empty
  {
    //ExitCritical();
    return bFALSE;
  }
  else
  {
    *dataPtr = FIFO->Buffer[FIFO->Start]; 	// Oldest data in FIFO put to dataPtr
    if (FIFO->Start == FIFO_SIZE-1) 		// Implement wrap around buffer
      FIFO->Start = 0;
    else
    FIFO->Start++; 				// Increment position of oldest in buffer
    FIFO->NbBytes--; 				// Decrease count of FIFO bytes
    //ExitCritical();
    return bTRUE;
  }
}

/*!
 * @}
*/
