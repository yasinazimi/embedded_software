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
static uint8_t TowerNumberLsb = 0xA2;		// Least-significant-bit of the lower 2 bytes of student ID
static uint8_t TowerNumberMsb = 0x0D >> 8;	// Most-significant-bit of the lower 2 bytes of student ID

void CMD_Send_Startup()
{
  Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

void CMD_Send_Special_Tower_Version()
{
  Packet_Put(CMD_TX_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

void CMD_Send_Tower_Number()
{
  Packet_Put(CMD_TX_TOWER_NUMBER, 1, TowerNumberLsb, TowerNumberMsb);
}

void CMD_Receive_Tower_Number(uint8_t lsb, uint8_t msb)
{
  TowerNumberLsb = lsb;
  TowerNumberMsb = msb;
}

/*!
 * @}
*/
