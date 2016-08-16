/*! @file
 *
 *  @brief Computer command function implementation
 *
 *  Implementations of functions which abstract communication with the computer into commands.
 *
 *  @author Mohammad Yasin Azimi, Micheal Codner
 *  @date 2016-08-16
 */
/*!
 * @addtogroup CMD_module CMD module documentation
 * @{
*/
#include "cmd.h"
#include "packet.h"

const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;

uint8_t towerNumberLsb = 0xA2;		// Least-significant-bit of the lower 2 bytes of student ID
uint8_t towerNumberMsb = 0x0D;		// Most-significant-bit of the lower 2 bytes of student ID

void Send_Startup()
{
  Packet_Put(CMD_TX_TOWER_STARTUP, 0x0, 0x0, 0x0);
}

void Send_Special_Tower_Version()
{
  Packet_Put(CMD_TX_TOWER_VERSION, 'v', TOWER_VERSION_H, TOWER_VERISON_L);
}

void Send_Tower_Number()
{
  Packet_Put(CMD_TX_TOWER_NUMBER, 1, towerNumberLsb, towerNumberMsb);
}

void Receive_Tower_Number(uint8_t lsb, uint8_t msb)
{
  towerNumberLsb = lsb;
  towerNumberMsb = msb;
}

/*!
 * @}
*/
