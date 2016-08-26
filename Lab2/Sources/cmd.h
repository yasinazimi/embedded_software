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
 * @brief Set up the tower number and mode flash allocation.
 * @note Requires the flash module to be started.
 */
BOOL CMD_Init();

/*!
 * @brief Sends the startup packet to the computer.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Send_Startup();

/*!
 * @brief Read a byte of flash at the specified address
 * @param offset The offset of the byte which is being transmitted.
 * @param data The actual byte.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Send_Flash_Read_Byte(const uint8_t offset, const uint8_t data);

/*!
 * @brief Sends the tower version to the computer.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Send_Special_Tower_Version();

/*!
 * @brief Sends the tower number to the computer.
 * @return BOOL TRUE if success.
 */
BOOL CMD_Send_Tower_Number();

/*!
 * @brief Get the tower mode and save to flash.
 * @return BOOL TRUE if success.
 */
BOOL CMD_Send_Tower_Mode();

/*!
 * @brief Programs a byte of flash at the specified offset.
 * @param offset Offset of the byte from the start of the sector.
 * @param data The byte to write.
 * @note An offset greater than 7 will erase the sector.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Receive_Flash_Program_Byte(const uint8_t offset, const uint8_t data);

/*!
 * @brief Read a byte of the flash.
 * @param offset Offset of the byte from the start of the sector.
 * @param data The address to read the value into.
 * @note An offset past the end of the flash will fail.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Receive_Flash_Read_Byte(const uint8_t offset, uint8_t * const data);

/*!
 * @brief Saves the tower number to a buffer.
 * @param lsb Least significant byte of the Tower number
 * @param msb Most significant byte of the Tower number
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Receive_Tower_Number(uint8_t lsb, uint8_t msb);

/*!
 * @brief Set the tower mode and save to flash.
 * @param lsb The least significant byte.
 * @param msb The most significant byte.
 * @return BOOL TRUE if the operation succeeded.
 */
BOOL CMD_Receive_Tower_Mode(const uint8_t lsb, const uint8_t msb);

/*!
 * @}
*/

#endif
