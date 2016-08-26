/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-24
 */

#include "flash.h"

BOOL Flash_Init(void)
{
  return bTRUE; // Temp until we implement the functions
}
 
BOOL Flash_AllocateVar(volatile void** variable, const uint8_t size)
{
  return bTRUE; // Temp until we implement the functions
}

BOOL Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  return bTRUE; // Temp until we implement the functions
}
 
BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  return bTRUE; // Temp until we implement the functions
}

BOOL Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  return bTRUE; // Temp until we implement the functions
}

BOOL Flash_Erase(void)
{
  return bTRUE; // Temp until we implement the functions
}