/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  This contains the prototypes of functions for the data transmission between PC and Tower and vice versa.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-16
 */
/*!
 * @addtogroup CMD_module CMD module documentation
 * @{
*/
#ifndef CMD_H
#define CMD_H

#include "types.h"

 /*!
  * The PC will issue this command upon startup to retrieve
  * the state of the Tower to update the interface application.
  */
#define CMD_RX_GET_SPECIAL_START_VAL 0x04

/*!
 * Gets the version of the Tower software.
 */
#define CMD_RX_GET_VERSION 0x09

/*!
 * Get or set the Student ID associated with the Tower software.
 */
#define CMD_RX_TOWER_NUMBER 0x0B

/*!
 * The Tower will issue this command upon startup to allow
 * the PC to update the interface application and the Tower.
 * Typically, setup data will also be sent from the Tower to the PC.
 */
#define CMD_TX_TOWER_STARTUP 0x04

/*!
 * Sends the Tower version to the PC.
 */
#define CMD_TX_TOWER_VERSION 0x09

/*!
 * Sends the Tower number to the PC.
 */
#define CMD_TX_TOWER_NUMBER 0x0B

/*!
 * Packet parameter 1 to 'get' Tower number.
 */
#define CMD_TOWER_NUMBER_GET 1

/*!
 * Packet parameter 1 to 'set' Tower number.
 */
#define CMD_TOWER_NUMBER_SET 2

/*!
 * 11733490 = 0xB309F2
 * xxxx3490 = 0x0DA2 - use the lower 2 bytes as param1 and param2
 */
#define CMD_SID 0x0DA2

/*!
 * @brief Sends the startup packet to the computer.
 */
void CMD_Send_Startup();

/*!
 * @brief Sends the tower version to the computer.
 */
void CMD_Send_Special_Tower_Version();

/*!
 * @brief Sends the tower number to the computer.
 */
void CMD_Send_Tower_Number();

/*!
 * @brief Saves the Tower number to a buffer.
 * @param lsb Least significant byte of the Tower number
 * @param msb Most significant byte of the Tower number
 */
void CMD_Receive_Tower_Number(uint8_t lsb, uint8_t msb);

/*!
 * @}
*/

#endif
