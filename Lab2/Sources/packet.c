/*! @file
 *
 *  @brief Routines to implement packet encoding/decoding for serial port.
 *
 *  Implementation of the packet module for handling 5 bytes packets.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-23
 */
/*!
 * @addtogroup Packet_module Packet module documentation
 * @{
*/
#include "packet.h"

#include "cmd.h"
#include "UART.h"

static uint8_t State = 0;	/*!< The index of the byte in the packet, 5 bytes = 1 packet */
static uint8_t Checksum;	/*!< The received checksum of the packet */

// Packet structure
uint8_t Packet_Command,		/*!< The packet's command */
	Packet_Parameter1, 	/*!< The packet's 1st parameter */
	Packet_Parameter2, 	/*!< The packet's 2nd parameter */
	Packet_Parameter3;	/*!< The packet's 3rd parameter */

// Calculates the checksum by XORing parameters 1, 2 and 3
uint8_t PacketTest()
{
  uint8_t calc_checksum = Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3; /*!< Calculate the checksum of the packet */
  uint8_t ret_val = calc_checksum == Checksum;
  return ret_val;
}

// Initializes baud rate and module clock
BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  return UART_Init(baudRate, moduleClk);
}

BOOL Packet_Get(void)
{
  uint8_t uartData; 		/*!< Store the byte received */
  if (!UART_InChar(&uartData))	// Attempt to receive a byte and continue if successful
  {
    return bFALSE;
  }
  switch (State)
  {
    case 0:
      Packet_Command = uartData;
      State++;
      return bFALSE;
    case 1:
      Packet_Parameter1 = uartData;
      State++;
      return bFALSE;
    case 2:
      Packet_Parameter2 = uartData;
      State++;
      return bFALSE;
    case 3:
      Packet_Parameter3 = uartData;
      State++;
      return bFALSE;
    case 4:
      Checksum = uartData;
      if (PacketTest())
      {
	State = 0;
	return bTRUE;
      }
      Packet_Command = Packet_Parameter1;
      Packet_Parameter1 = Packet_Parameter2;
      Packet_Parameter2 = Packet_Parameter3;
      Packet_Parameter3 = Checksum;
      return bFALSE;
  }
}

BOOL Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  if (!UART_OutChar(command))
  {
    return bFALSE;
  }
  if (!UART_OutChar(parameter1))
  {
    return bFALSE;
  }
  if (!UART_OutChar(parameter2))
  {
    return bFALSE;
  }
  if (!UART_OutChar(parameter3))
  {
    return bFALSE;
  }
  return UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);
}

/*!
 * @}
*/
