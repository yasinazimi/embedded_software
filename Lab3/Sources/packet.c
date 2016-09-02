/*! @file
 *
 *  @brief Routines to implement packet encoding/decoding for serial port.
 *
 *  Implementation of the packet module for handling 5 bytes packets.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
/*!
 * @addtogroup Packet_module Packet module documentation
 * @{
*/
#include "packet.h"

#include "UART.h"

static uint8_t State = 0; 	/*!< The index of the byte in the packet, 5 bytes = 1 packet */
static uint8_t Checksum;  	/*!< The received checksum of the packet */

/*!< The packet's command struct */
TPacket Packet;

// Calculates the checksum by XORing parameters 1, 2 and 3
uint8_t PacketTest()
{
  /*!< Calculate the checksum of the packet */
  uint8_t calc_checksum = Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3;
  uint8_t ret_val = calc_checksum == Checksum;
  return ret_val;
}

// Initializes baud rate and module clock
bool Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  return UART_Init(baudRate, moduleClk);
}

bool Packet_Get(void)
{
  /*!< Store the byte received */
  uint8_t uartData;
  // Attempt to receive a byte and continue if successful
  if (!UART_InChar(&uartData))
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

bool Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
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
  // Returns true to indicate 'Packet Put' to TxFIFO was a success
  return UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);
}

/*!
 * @}
*/
