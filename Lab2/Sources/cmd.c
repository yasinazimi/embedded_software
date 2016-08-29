/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  Implementations of functions which abstract communication with the computer into commands.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-08-30
 */
/*!
 * @addtogroup CMD_module CMD module documentation
 * @{
*/
#include "cmd.h"

#include "flash.h"
#include "types.h"
#include "packet.h"

#define CMD_SID 0x0DA2

/*!
 * Tower software version V01.00
 */
const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;

static uint16union_t volatile *TowerNumber;
static uint16union_t volatile *TowerMode;

/*!
 * Tower will update interface application on the PC and the Tower.
 */
const uint8_t CMD_TX_TOWER_STARTUP = 0x04;

/*!
 * Sends the Tower version to the PC.
 */
const uint8_t CMD_TX_TOWER_VERSION = 0x09;

/*!
 * Sends the Tower number to the PC.
 */
const uint8_t CMD_TX_TOWER_NUMBER = 0x0B;

/*!
 * Packet parameter 1 to 'get' Tower number.
 */
const uint8_t CMD_TOWER_NUMBER_GET = 1;

/*!
 * Packet parameter 1 to 'set' Tower number.
 */
const uint8_t CMD_TOWER_NUMBER_SET = 2;

/*!
 * Program a byte of flash
 */
const uint8_t CMD_RX_FLASH_PROGRAM_BYTE = 0x7;

/*!
 * Read a byte of flash
 */
const uint8_t CMD_RX_FLASH_READ_BYTE = 0x8;

/*!
 * Get or set the tower mode
 */
const uint8_t CMD_RX_TOWER_MODE = 0xd;

/*!
 * Send the result of a flash read operation
 */
const uint8_t CMD_TX_FLASH_READ_BYTE = 0x8;

/*!
* Send the tower mode to the PC application
*/
const uint8_t CMD_TX_TOWER_MODE = 0xd;

/*!
 * Packet parameter 1 to get tower mode.
 */
const uint8_t CMD_TOWER_MODE_GET = 1;

/*!
 * Packet parameter 1 to set tower mode.
 */
const uint8_t CMD_TOWER_MODE_SET = 2;

// 
// End of globals
// 

// 
BOOL CMD_Init() 
{
  BOOL allocSpace = Flash_AllocateVar((volatile void **) &TowerNumber, sizeof(uint16union_t));
  BOOL allocMode = Flash_AllocateVar((volatile void **) &TowerMode, sizeof(uint16union_t));
  if (allocSpace == bTRUE && allocMode == bTRUE)
  {
    if (TowerNumber->l == 0xFFFF)
    {
      Flash_Write16((uint16_t volatile *) TowerNumber, CMD_SID);
    }
    if (TowerMode->l == 0xFFFF)
    {
      Flash_Write16((uint16_t volatile *) TowerMode, 0x1);
    }
    return bTRUE;
  }
  return bFALSE;
}

// 
BOOL CMD_Send_Startup()
{
  return Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

// 
BOOL CMD_Send_Special_Tower_Version()
{
  return Packet_Put(CMD_TX_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

// 
BOOL CMD_Send_Tower_Number()
{
  return Packet_Put(CMD_TX_TOWER_NUMBER, 1, TowerNumber->s.Lo, TowerNumber->s.Hi);
}

// 
BOOL CMD_Send_Flash_Read_Byte(const uint8_t offset, const uint8_t data)
{
  return bTRUE;
}

// 
BOOL CMD_Send_Tower_Mode()
{
  return Packet_Put(CMD_TX_TOWER_MODE, 0x1, TowerMode->s.Lo, TowerMode->s.Hi);
}

// 
BOOL CMD_Receive_Flash_Program_Byte(const uint8_t offset, const uint8_t data)
{
  // Change 8 to FLASH_DATA_SIZE for dynamicness
  if (offset > 8)
  {
    return bFALSE;
  }
  // Change 8 to FLASH_DATA_SIZE for dynamicness
  if (offset == 8)
  {
    return Flash_Erase();
  }
  uint8_t *address = (uint8_t *)(FLASH_DATA_START + offset);
  return Flash_Write8(address, data);
}

// 
BOOL CMD_Receive_Flash_Read_Byte(const uint8_t offset, uint8_t * const data)
{
  if (offset > (FLASH_DATA_SIZE - 1))
  {
    return bFALSE;
  }
  *data = _FB(FLASH_DATA_START + offset);
  return bTRUE;
}

// 
BOOL CMD_Receive_Tower_Number(uint8_t lsb, uint8_t msb)
{
  uint16union_t temp;
  temp.s.Hi = msb;
  temp.s.Lo = lsb;
  return Flash_Write16((uint16_t volatile *) TowerNumber, temp.l);
}

// 
BOOL CMD_Receive_Tower_Mode(const uint8_t lsb, const uint8_t msb)
{
  uint16union_t temp;
  temp.s.Hi = msb;
  temp.s.Lo = lsb;
  return Flash_Write16((uint16_t volatile *) TowerMode, temp.l);
}

/*!
 * @}
*/