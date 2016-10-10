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
#include "Cpu.h"
#include "FIFO.h"
#include "PE_Types.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param FIFO A pointer to the FIFO that needs initializing.
 *  @return void
 */
void FIFO_Init(TFIFO * const FIFO)
{
  FIFO->Start = 0;	// Initialises head pointer location to 0
  FIFO->End = 0;	// Initialises tail pointer location to 0
  FIFO->NbBytes = 0;	// Initialises number of bytes stored in FIFO to 0
}

/*! @brief Put one character into the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct where data is to be stored.
 *  @param data A byte of data to store in the FIFO buffer.
 *  @return bool - TRUE if data is successfully stored in the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
BOOL FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  EnterCritical();			// Enter critical component
  if (FIFO->NbBytes == FIFO_SIZE)  	// Check if FIFO buffer is full
  {
    ExitCritical();			// Exit critical component
    return bFALSE;			// Returns 0 if buffer is full
  }
  else
  {
    FIFO->Buffer[FIFO->End] = data; 	// Place data in FIFO

    if (FIFO->End == FIFO_SIZE-1)	// Resets tail pointer location to 0 (if reached end of buffer)
    {
      FIFO->End = 0;                	// Circular buffer wraps around if at end of FIFO buffer
    }
    else
    {
      FIFO->End++;                  	// Next place in FIFO to write to
    }

    FIFO->NbBytes++; 			// Increment bytes in FIFO
    ExitCritical();			// Exit critical component
    return bTRUE;			// FIFO successfully received a byte
  }
}

/*! @brief Get one character from the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct with data to be retrieved.
 *  @param dataPtr A pointer to a memory location to place the retrieved byte.
 *  @return bool - TRUE if data is successfully retrieved from the FIFO.
 *  @note Assumes that FIFO_Init has been called.
 */
BOOL FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  EnterCritical();				// Enter critical component
  if (FIFO->NbBytes == 0) 			// Check if FIFO is empty
  {
    ExitCritical();				// Exit critical component
    return bFALSE;				// Returns 0 if buffer is empty
  }
  else
  {
    *dataPtr = FIFO->Buffer[FIFO->Start]; 	// Oldest data in FIFO put to dataPtr

    if (FIFO->Start == FIFO_SIZE-1) 		// Resets head pointer location to 0 (if reached end of buffer)
    {
      FIFO->Start = 0;				// Circular buffer wraps around if at start of FIFO buffer
    }
    else
    {
      FIFO->Start++; 				// Increment position of oldest in buffer
    }
    FIFO->NbBytes--; 				// Decrease count of FIFO bytes
    ExitCritical();				// Exit critical component
    return bTRUE;				// FIFO successfully released byte
  }
}

/*!
 * @}
*/
