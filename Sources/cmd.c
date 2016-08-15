/*! @file
 *
 *  @brief Abstraction of commands for the tower into functions
 *
 *  @author Mohammad Yasin Azimi (11733490), Micheal Codner (11989668)
 *  @date 15-08-2016
 */
/*!
**  @addtogroup cmd_module CMD module documentation
**  @{
*/
#include "cmd.h"
#include "packet.h"

const uint8_t TOWER_VERSION_H = 1;
const uint8_t TOWER_VERISON_L = 0;

uint8_t towerNumberLsb = (CMD_SID & 0x00FF);
uint8_t towerNumberMsb = (CMD_SID & 0xFF00) >> 8;

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
** @}
*/
