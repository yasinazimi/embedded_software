/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  Implementations of functions which abstract communication with the computer into commands.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
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

static uint8_t TowerNumberLsb = (CMD_SID & 0x00FF);			// Least-significant-bit of the lower 2 bytes of student ID
static uint8_t TowerNumberMsb = (CMD_SID & 0xFF00) >> 8;	// Most-significant-bit of the lower 2 bytes of student ID

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
