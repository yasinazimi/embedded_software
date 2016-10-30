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
#include "OS.h"
#include "Cpu.h"
#include "FIFO.h"
#include "PE_Types.h"

/*! @brief Initialize the FIFO before first use.
 *
 *  @param FIFO A pointer to the FIFO that needs initializing.
 *  @return void.
 */
void FIFO_Init(TFIFO * const FIFO)
{
  // Creates semaphores for signaling between the threads
  FIFO->BufferAccess = OS_SemaphoreCreate(1);
  FIFO->SpaceAvailable = OS_SemaphoreCreate(FIFO_SIZE);
  FIFO->ItemsAvailable = OS_SemaphoreCreate(0);

  // FIFO initialization
  FIFO->Start = 0;
  FIFO->End = 0;
  FIFO->NbBytes = 0;
  FIFO ->Buffer[FIFO_SIZE];
}

/*! @brief Put one character into the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct where data is to be stored.
 *  @param data A byte of data to store in the FIFO buffer.
 *  @return void.
 *  @note Assumes that FIFO_Init has been called.
 */
void FIFO_Put(TFIFO * const FIFO, const uint8_t data)
{
  OS_ERROR error;

  // This waits for space to become available
  error = OS_SemaphoreWait(FIFO->SpaceAvailable, 0);
  // This obtains exclusive access to the FIFO
  error = OS_SemaphoreWait(FIFO->BufferAccess, 0);

  // Place data in FIFO
  FIFO->Buffer[FIFO->End] = data;
  // Increment bytes in FIFO
  FIFO->NbBytes++;
  // Increments tail pointer location by 1 (if reached end of buffer)
  FIFO->End = (FIFO->End + 1) % FIFO_SIZE;

  // This relinquishes exclusive access to the FIFO
  OS_SemaphoreSignal(FIFO->BufferAccess);
  // This increments the number of items available in FIFO
  OS_SemaphoreSignal(FIFO->ItemsAvailable);
}

/*! @brief Get one character from the FIFO.
 *
 *  @param FIFO A pointer to a FIFO struct with data to be retrieved.
 *  @param dataPtr A pointer to a memory location to place the retrieved byte.
 *  @return void.
 *  @note Assumes that FIFO_Init has been called.
 */
void FIFO_Get(TFIFO * const FIFO, uint8_t * const dataPtr)
{
  OS_ERROR error;

  // This waits for space to become available
  error = OS_SemaphoreWait(FIFO->ItemsAvailable, 0);
  // This obtains exclusive access to the FIFO
  error = OS_SemaphoreWait(FIFO->BufferAccess, 0);

  // Oldest data in FIFO put to dataPtr
  *dataPtr = FIFO->Buffer[FIFO->Start];
  // Decrement count of FIFO bytes
  FIFO->NbBytes--;
  //
  FIFO->Start = (FIFO->Start + 1) % FIFO_SIZE;

  // This relinquishes exclusive access to the FIFO
  OS_SemaphoreSignal(FIFO->BufferAccess);
  // This increments the number of items available in FIFO
  OS_SemaphoreSignal(FIFO->SpaceAvailable);
}

/*!
 * @}
 */
