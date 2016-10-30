/*! @file
 *
 *  @brief This contains the functions in the program to read, write and erase Flash.
 *
 *  Implementation of Flash module for handling Flash communication.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup Flash_module Flash module documentation
 * @{
 */
#include "Flash.h"
#include "types.h"
#include "MK70F12.h"

#define FCMD_ERASE_SECTOR 0x09u 					/*!< Command value to erase sector */
#define MAX_BYTES (FLASH_DATA_END-FLASH_DATA_START+1)			/*!< Maximum number of bytes in the flash */

// Use main_memMap and initialise global array for flash memory map
extern BOOL main_memMap[FLASH_DATA_END-FLASH_DATA_START+1];

// Function prototypes
static BOOL WritePhrase(const uint32_t address, const uint64_t phrase);
static BOOL LaunchCommand(FCCOB_t* commonCommandObject);
FCCOB_t  Flash_Erase_Sector( uint32_t address);

// Erases sector
FCCOB_t fccob;

/*! @brief Launches the startup commands
 *
 *  @return BOOL - TRUE if the Flash was setup successfully.
 */
static BOOL LaunchCommand(FCCOB_t* commonCommandObject)
{
  static uint8_t accerrStatus, fpviolStatus;
  while (FTFE_FSTAT >> 7 == 0)
  {
    // Perform FCCOB Availability Check by checking status of CCIF bit
  }

  // Because we check for bit 4 FPVIOL (flash protection violatoin flag)
  fpviolStatus= (FTFE_FSTAT & 0x1F) >> 4;
  // Because we check for bit 5 Flash Access Error Flag (flash access error flag)
  accerrStatus= (FTFE_FSTAT & 0x3F) >> 5;

  if(fpviolStatus == 1 | accerrStatus == 1)
  {
    // Write 0x30 to FSTAT register
    FTFE_FSTAT = (FTFE_FSTAT_ACCERR_MASK | FTFE_FSTAT_FPVIOL_MASK);
  }

  // Flash CommandS
  FTFE_FCCOB0 = commonCommandObject->command;
  FTFE_FCCOB1 = commonCommandObject->flashAddress23_16;
  FTFE_FCCOB2 = commonCommandObject->flashAddress15_8;
  FTFE_FCCOB3 = commonCommandObject->flashAddress7_0;
  FTFE_FCCOB4 = commonCommandObject->dataByte4;
  FTFE_FCCOB5 = commonCommandObject->dataByte5;
  FTFE_FCCOB6 = commonCommandObject->dataByte6;
  FTFE_FCCOB7 = commonCommandObject->dataByte7;
  FTFE_FCCOB8 = commonCommandObject->dataByte0;
  FTFE_FCCOB9 = commonCommandObject->dataByte1;
  FTFE_FCCOBA = commonCommandObject->dataByte2;
  FTFE_FCCOBB = commonCommandObject->dataByte3;

  // Statements FCCOB register assignments
  // Ensure no more parameters required
  FTFE_FSTAT = FTFE_FSTAT_CCIF_MASK;
  // Clears the CCIF to launch the command
  while(!(FTFE_FSTAT & FTFE_FSTAT_CCIF_MASK))
  {
    // Wait for CCIF bit to clear
  }

  // Because we check for bit 4 FPVIOL (flash protection violation flag)
  fpviolStatus = (FTFE_FSTAT & 0x1F) >> 4;
  // Because we check for bit 5 Flash Access Error Flag (flash access error flag)
  accerrStatus = (FTFE_FSTAT & 0x3F) >> 5;
  if (fpviolStatus == 1 | accerrStatus == 1)
  {
    return bFALSE;
  }

  return bTRUE;
}

/*! @brief Enables the Flash module.
 *
 *  @return BOOL - TRUE if the Flash was setup successfully.
 */
BOOL Flash_Init(void)
{
  //
}

// Returns a FCCOB
FCCOB_t  Flash_Erase_Sector( uint32_t address)
{
  FCCOB_t fccob;
  // Erases sector
  fccob.command = FCMD_ERASE_SECTOR;
  // Bits 32_16 of start location
  fccob.flashAddress23_16 = (uint8_t)(address >> 16);
  // Bits 15_8 of start location
  fccob.flashAddress15_8 = (uint8_t)(address >> 8);
  // Bits 7_0 of start location
  fccob.flashAddress7_0 = (uint8_t)address;

  return fccob;
}

/*! @brief Erases the entire Flash sector.
 *
 *  @return BOOL - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Erase(void)
{
  FCCOB_t fccob;
  fccob= Flash_Erase_Sector(FLASH_DATA_START);
  // Confirms Flash erase worked
  return  LaunchCommand(&fccob);
}

/*! @brief Write phrase to flash.
 *
 *  @return BOOL - TRUE if the Flash "data" sector was erased successfully.
 *  @note Assumes Flash has been initialized.
 */
static BOOL WritePhrase(const uint32_t address, const uint64_t phrase)
{
  static uint8arr_t phrase_t;
  uint32union_t addressHalves = (uint32union_t)address;
  uint16union_t addressTop16 = (uint16union_t)addressHalves.s.Hi;
  uint16union_t addressBottom16 = (uint16union_t)addressHalves.s.Lo;
  // Erase sector 0 (4KiB) in preparation of a write to Volatile Flash Storage
  Flash_Erase();

  // Prepare the Flash Common Command Object type structure for a write to flash
  fccob.command = 0x07;
  // Internal command to program phrase
  fccob.flashAddress23_16 = addressTop16.s.Lo;
  fccob.flashAddress15_8 = addressBottom16.s.Hi;

  // Assign the FCCOB_t as described p789-790 K70 manual
  fccob.flashAddress7_0 = addressBottom16.s.Lo;
  fccob.dataByte0=(uint8_t)(phrase>>56);
  fccob.dataByte1=(uint8_t)(phrase>>48);
  fccob.dataByte2=(uint8_t)(phrase>>40);
  fccob.dataByte3=(uint8_t)(phrase>>32);
  fccob.dataByte4=(uint8_t)(phrase>>24);
  fccob.dataByte5=(uint8_t)(phrase>>16);
  fccob.dataByte6=(uint8_t)(phrase>>8);
  fccob.dataByte7=(uint8_t)(phrase);
  return LaunchCommand(&fccob);
}

/*! @brief Writes a 32-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 32-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 4-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write32(volatile uint32_t* const address, const uint32_t data)
{
  /*!< Stores the hi and lo parts of the phrase (64bit) */
  uint64union_t phrase;
  /*!< Stores data in starting address */
  uint32_t address32bit = (uint32_t)address;

  if (address32bit % 8 == 0)
  {
    // Merges data in address32bit+4 with current word
    phrase.s.Lo = data;
    phrase.s.Hi = _FW(address32bit+4);
    // Returns phrase (64bit)
    return WritePhrase(address32bit, (uint64_t)phrase.l);
  }
  // Sets a phrase
  else if (address32bit % 4 == 0)
  {
    phrase.s.Lo = _FW(address32bit-4);
    phrase.s.Hi = data;
    // Merges data in address32bit-4 with current word
    // Returns phrase (64bit)
    return WritePhrase(address32bit-4, (uint64_t)phrase.l);
  }
  else
  {
    //
  }
  //debug alignment error
  return bFALSE;
}

/*! @brief Writes a 16-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 16-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if address is not aligned to a 2-byte boundary or if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write16(volatile uint16_t* const address, const uint16_t data)
{
  /*!< Stores the hi and lo parts of the longword (32bit) */
  uint32union_t word;
  /*!< Stores data in starting address */
  uint32_t address32bit=(uint32_t)address;

  // Sets a word
  if (address32bit %4 == 0)
  {
    // Merges data in address32bit+2 with current half word
    word.s.Lo = data;
    word.s.Hi = _FH(address32bit+2);
    // Returns word (32bit)
    return Flash_Write32(&_FW(address32bit),word.l);
  }
  else if (address32bit % 2 == 0)
  {
    // Merges data in address32bit-2 with current half word
    word.s.Lo = _FH(address32bit-2);
    word.s.Hi = data;
    // Returns word (32bit)
    return Flash_Write32(&_FW(address32bit-2),word.l);
  }
  else
  {
    //
  }
  //debug alignment error
  return bFALSE;
}

/*! @brief Writes an 8-bit number to Flash.
 *
 *  @param address The address of the data.
 *  @param data The 8-bit data to write.
 *  @return BOOL - TRUE if Flash was written successfully, FALSE if there is a programming error.
 *  @note Assumes Flash has been initialized.
 */
BOOL Flash_Write8(volatile uint8_t* const address, const uint8_t data)
{
  /*!< Stores the hi and lo parts of the half word (16bit) */
  uint16union_t halfword;
  /*!< Stores data in starting address */
  uint32_t address32bit = (uint32_t) address;

  // Data from the address needs to be put to the lower part of stored variable halfword
  if (address32bit % 2 == 0)
  {
    // Merges data in address32bit+1 with current byte
    halfword.s.Lo = data;
    halfword.s.Hi = _FB((address32bit+1));
    // Returns half word (16bit)
    return Flash_Write16 (&_FH((address32bit)), halfword.l);
  }
  else
  {
    // Merges data in address32bit-1 with current byte
    halfword.s.Lo = _FB(address32bit-1);
    halfword.s.Hi = data;
    // Returns half word (16bit)
    return Flash_Write16 (&_FH(address32bit-1), halfword.l);
  }
}

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
BOOL Flash_AllocateVar(volatile void** variable, uint8_t size)
{
  // Use main_memMap and initialise global array for flash memory map to zero
  static BOOL memMap[(MAX_BYTES)];
  // Not a static so it initilises to zero;
  uint8_t memMapIndex = 0;

  switch (size)
  {
    case 1:
    {
      while(memMapIndex<MAX_BYTES)
      {
        if (memMap[memMapIndex]==bFALSE)
        {
          // Assign address of flash write commence to variable
          *(volatile uint8_t**)variable = &_FB(FLASH_DATA_START+memMapIndex);
          // Mark part of memory map used
          memMap[memMapIndex] = bTRUE;
          return bTRUE;
        }
        else
        {
          memMapIndex++;
        }
      }
      return bFALSE;
    }

    case 2:
    {
      // Ensure if statement below does not over each array size
      while(memMapIndex<MAX_BYTES-1)
      {
        if ((memMap[memMapIndex] == bFALSE)
          && (memMap[memMapIndex+1] == bFALSE)
          && (memMapIndex % 2 == 0))
        {
          // Assign address of flash write commence to variable
          *(volatile uint16_t**)variable=&_FH(FLASH_DATA_START+memMapIndex);
          // Mark parts of memory map used
          memMap[memMapIndex] = bTRUE;
          memMap[memMapIndex+1] = bTRUE;
          // Exit loop
          return bTRUE;
        }
        else
        {
          memMapIndex++;
        }
      }
      // This means no 2 adjacent memory locations that start on an even number exist
      return bFALSE;
    }

    case 4:
    {
      while(memMapIndex<MAX_BYTES-3)
      {
        if ((memMap[memMapIndex] == bFALSE)
          && (memMap[memMapIndex+1] == bFALSE)
          && (memMap[memMapIndex+2] == bFALSE)
          && (memMap[memMapIndex+3] == bFALSE)
          && (memMapIndex % 4 == 0))
        {
          // Assign address of flash write commence to variable
          *(volatile uint32_t**)variable=&_FW(FLASH_DATA_START+memMapIndex);
          // Mark parts of memory map used
          memMap[memMapIndex] = bTRUE;
          memMap[memMapIndex+1] = bTRUE;
          memMap[memMapIndex+2] = bTRUE;
          memMap[memMapIndex+3] = bTRUE;
          // Exit loop
          return bTRUE;
        }
        else
        {
          memMapIndex++;
        }
      }
      // This means no 2 adjacent memory locations that start on an even number exist
      return bFALSE;
    }

    default:
      // Because invalid byte size
      return bFALSE;
  }
}

/*!
 * @}
 */
