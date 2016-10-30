/*! @file
 *
 *  @brief Routines for erasing and writing to the Flash.
 *
 *  This contains the functions needed for accessing the internal Flash.
 *
 *  @author PMcL
 *  @date 2016-10-31
 */
#ifndef FLASH_H
#define FLASH_H

// new types
#include "types.h"

// FLASH data access
#define _FB(flashAddress)  *(uint8_t  volatile *)(flashAddress) // Byte
#define _FH(flashAddress)  *(uint16_t volatile *)(flashAddress) // Word
#define _FW(flashAddress)  *(uint32_t volatile *)(flashAddress) // Longword
#define _FP(flashAddress)  *(uint64_t volatile *)(flashAddress) // Phrase

// Address of the start of the Flash block we are using for data storage
#define FLASH_DATA_START 0x00080000LU
// Address of the end of the Flash block we are using for data storage
#define FLASH_DATA_END   0x00080007LU
// The number of bytes in the flash block (currently 8)
#define FLASH_DATA_SIZE ((FLASH_DATA_END-FLASH_DATA_START)+1)

typedef struct
{
  uint8_t command;        // FCMD is flash command 0xXX defines FTFE
  uint8_t flashAddress23_16;
  uint8_t flashAddress15_8;
  uint8_t flashAddress7_0;
  uint8_t dataByte0;
  uint8_t dataByte1;
  uint8_t dataByte2;
  uint8_t dataByte3;
  uint8_t dataByte4;
  uint8_t dataByte5;
  uint8_t dataByte6;
  uint8_t dataByte7;
} FCCOB_t;                // Flash Common Command Object

/*! @brief Enables the Flash module.
 *
 *  @return BOOL - TRUE if the Flash was setup successfully.
 */
BOOL Flash_Init(void);
 
/*! @brief Allocates space for a non-volatile variable in the Flash memory.
 *
 *  @param variable is the address of a pointer to a variable that is to be allocated space in Flash memory.
 *         The pointer will be allocated to a relevant address:
 *         If the variable is a byte, then any address.
 *         If the variable is a half-word, then an even address.
 *         If the variable is a word, then an address divisible by 4.
 *         This allows the resulting variable to be used with the relevant Flash_Write function which assumes a certain memory address.
 *         e.g. a 16-bit variable will be on an even address
 *  @param size The size, in bytes, of the variable that is to be allocated space in the Flash memory. Valid values are 1, 2 and 4.
 *  @return BOOL - TRUE if the variable was allocated space in the Flash memory.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_AllocateVar(volatile void** variable, const uint8_t size);

/*! @brief Writes a 32-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write32(volatile uint32_t* const address, const uint32_t data);
 
/*! @brief Writes a 16-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data);

/*! @brief Writes an 8-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write8(volatile uint8_t* const address, const uint8_t data);

/*! @brief Erases the entire Flash sector.
 *
 *  @return BOOL - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Erase(void);

#endif
