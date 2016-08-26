/*! @file
 *
 *  @brief Functions which abstract communication with the computer into commands.
 *
 *  This contains the prototypes of functions for the data transmission between PC and Tower and vice versa.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-24
 */
/*!
 * @addtogroup CMD_module CMD module documentation
 * @{
*/
#ifndef CMD_H
#define CMD_H

#include "types.h"
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
