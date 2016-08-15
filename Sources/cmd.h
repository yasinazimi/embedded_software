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
#ifndef CMD_H
#define CMD_H

#include "types.h"

 /*!
  * The PC will issue this command upon startup
  * to retrieve the state of the Tower to update
  * the interface application.
  * */
#define CMD_RX_GET_SPECIAL_START_VAL 0x04

/*!
 * Get the version of the Tower software.
 */
#define CMD_RX_GET_VERSION 0x09

/*!
 * Get or set the Student ID associated with
 * the Tower software.
 */
#define CMD_RX_TOWER_NUMBER 0x0B

/*!
 * The Tower will issue this command upon startup to
 * allow the PC to update the interface application
 * and the Tower. Typically, setup data will also be
 * sent from the Tower to the PC.
 */
#define CMD_TX_TOWER_STARTUP 0x04

/*!
 * Send the tower version to the PC.
 */
#define CMD_TX_TOWER_VERSION 0x09

/*!
 * Send the tower number to the PC.
 */
#define CMD_TX_TOWER_NUMBER 0x0B

/*!
 * Packet parameter 1 to get tower number.
 */
#define CMD_TOWER_NUMBER_GET 1

/*!
 * Packet parameter 1 to set tower number.
 */
#define CMD_TOWER_NUMBER_SET 2

/*!
 * 11989668 = 0xB6F2A4. Use the lower 2 bytes as param1 and param2
 */
#define CMD_SID 0xF2A4

/*!
 * @brief Sends the startup packet to the computer.
 */
void Send_Startup();

/*!
 * @brief Sends the tower version to the computer.
 */
void Send_Special_Tower_Version();

/*!
 * @brief Sends the tower number to the computer.
 */
void Send_Tower_Number();

/*!
 * @brief Saves the tower number to a buffer.
 * @param lsb Least significant byte of the Tower number
 * @param msb Most significant byte of the Tower number
 */
void Receive_Tower_Number(uint8_t lsb, uint8_t msb);

#endif
/*!
** @}
*/
