/*! @file
 *
 *  @brief Routines to implement packet encoding/decoding for serial port
 *
 *  Implementation of the packet module, handles 5 bytes packets at each state of the check-sum
 *
 *  @author Mohammad Yasin Azimi (11733490), Micheal Codner (11989668)
 *  @date 03-08-2016
 */
/*!
 ** @addtogroup packet_module Packet module documentation
 ** @{
 */

// new types
#include "packet.h"

#include "UART.h"

uint8_t packet_position = 0;	// Setting the initial position of the packet

uint8_t packet_checksum;	// Declaring the checksum

// Packet structure
uint8_t Packet_Command,		/*!< The packet's command */
	Packet_Parameter1, 	/*!< The packet's 1st parameter */
	Packet_Parameter2, 	/*!< The packet's 2nd parameter */
	Packet_Parameter3;	/*!< The packet's 3rdt parameter */

uint8_t PacketTest() {
  uint8_t calc_checksum = Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3;
  uint8_t ret_val = calc_checksum == packet_checksum;
}

BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  // Initialization of the BD
  UART_Init(baudRate, moduleClk);
}


BOOL Packet_Get(void)
{
  uint8_t uartData;
  if (!UART_InChar(&uartData))
  {
    return bFALSE;
  }

  switch (packet_position) {
    case 0:
      Packet_Command = uartData;
      packet_position++;
      return bFALSE;
    case 1:
      Packet_Parameter1 = uartData;
      packet_position++;
      return bFALSE;
    case 2:
      Packet_Parameter2 = uartData;
      packet_position++;
      return bFALSE;
    case 3:
      Packet_Parameter3 = uartData;
      packet_position++;
      return bFALSE;
    case 4:
      packet_checksum = uartData;
      if (PacketTest())
      {
	packet_position = 0;
	return bTRUE;
      }
      Packet_Command = Packet_Parameter1;
      Packet_Parameter1 = Packet_Parameter2;
      Packet_Parameter2 = Packet_Parameter3;
      Packet_Parameter3 = packet_checksum;
      return bFALSE;
  }
}


BOOL Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  UART_OutChar(command);
  UART_OutChar(parameter1);
  UART_OutChar(parameter2);
  UART_OutChar(parameter3);
  UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);
}

/*!
** @}
*/
