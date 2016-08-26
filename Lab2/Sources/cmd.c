/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  Implementations of functions which abstract communication with the computer into commands.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-24
 */
/*!
 * @addtogroup CMD_module CMD module documentation
 * @{
*/
#include "cmd.h"
#include "packet.h"

const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;
/*!
 * The Tower will issue this command upon startup to allow
 * the PC to update the interface application and the Tower.
 * Typically, setup data will also be sent from the Tower to the PC.
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
 * 11733490 = 0xB309F2
 * xxxx3490 = 0x0DA2 - use the lower 2 bytes as param1 and param2
 */
static uint8_t TowerNumberLsb = 0xA2;   // Least-significant-bit of the lower 2 bytes of student ID
static uint8_t TowerNumberMsb = 0x0D >> 8;  // Most-significant-bit of the lower 2 bytes of student ID

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

BOOL CMD_Init() 
{
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Send_Startup()
{
  return Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

BOOL CMD_Send_Special_Tower_Version()
{
  return Packet_Put(CMD_TX_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

BOOL CMD_Send_Tower_Number()
{
  return Packet_Put(CMD_TX_TOWER_NUMBER, 1, TowerNumberLsb, TowerNumberMsb);
}

BOOL CMD_Send_Flash_Read_Byte(const uint8_t offset, const uint8_t data)
{
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Send_Tower_Mode()
{
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Receive_Flash_Program_Byte(const uint8_t offset, const uint8_t data)
{
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Receive_Flash_Read_Byte(const uint8_t offset, uint8_t * const data)
{
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Receive_Tower_Number(uint8_t lsb, uint8_t msb)
{
  TowerNumberLsb = lsb;
  TowerNumberMsb = msb;
  return bTRUE; // Temp until we implement writing to flash
}

BOOL CMD_Receive_Tower_Mode(const uint8_t lsb, const uint8_t msb)
{
  return bTRUE; // Temp until we implement writing to flash
}

/*!
 * @}
*/
