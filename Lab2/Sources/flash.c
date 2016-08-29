/*! @file
 *
 *  @brief This contains the functions in the program to read, write and erase Flash.
 *
 *  Implementation of Flash module for handling Flash communication
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-08-30
 */
/*!
 * @addtogroup Flash_module Flash module documentation
 * @{
*/
#include "Flash.h"
#include "types.h"
#include "MK70F12.h"

BOOL Flash_Init(void)
{
  return bTRUE; // Temporary until function is implemented
}
 
BOOL Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  return bTRUE; // Temporary until function is implemented
}

// Flash has been initialised and this puts a 32-bit integer to Flash
BOOL Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  size_t index = (size_t) address - FLASH_DATA_START;
  if (index >= FLASH_DATA_SIZE || index < 0)
  {
    // Out of range
    return bFALSE;
  }
  if (index % 4 != 0)
  {
    // Not aligned
    return bFALSE;
  }
  index /= 4;
  uint64_t tempPhrase;
  ReadPhrase(&tempPhrase);
  uint32_t *psuedoArray = (uint32_t *) &tempPhrase;
  psuedoArray[index] = data;
  WritePhrase(tempPhrase);
  return bTRUE;
}
 
BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  return bTRUE; // Temporary until function is implemented
}

BOOL Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  return bTRUE; // Temporary until function is implemented
}

BOOL Flash_Erase(void)
{
  return bTRUE; // Temporary until function is implemented
}

/*!
 * @}
*/