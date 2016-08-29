/*! @file
 *
 *  @brief Routines to implement packet encoding and decoding for the serial port.
 *
 *  This contains the functions for implementing the Tower to PC Protocol 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-08-30
 */
/*!
 * @addtogroup Packet_module Packet module documentation
 * @{
*/
#ifndef PACKET_H
#define PACKET_H

// new types
#include "types.h"

// Packet structure
#pragma pack(push)
// Pack all the bytes that are continuous to make sure the bytes are in the correct memory location
#pragma pack(1)
// Struct contains only 3 bytes
typedef struct
{
  uint8_t command;			/*!< The packet's command. */
  union
  {
    struct
    {
      uint8_t parameters1;	/*!< The packet's 1st parameter. */
      uint8_t parameters2;	/*!< The packet's 2nd parameter. */
      uint8_t parameters3;	/*!< The packet's 3rd parameter. */
    } separate; 			/*!< Struct name. */
    struct
    {
      // Accesses parameters 1 and 2 bytes as 16 chunk
      uint16_t parameters12;
      uint8_t parameters3;
    } combined12;
    struct
    {
      // Accesses 8 byte chunk at the top and 16 byte chunk at the bottom
      uint8_t parameters1;
      uint16_t parameters23;
    } combined23;
  } parameters;
} TPacket;
#pragma pack(pop)

TPacket Packet;

uint8_t Packet_Command,		/*!< The packet's command */
		Packet_Parameter1, 	/*!< The packet's 1st parameter */
		Packet_Parameter2, 	/*!< The packet's 2nd parameter */
		Packet_Parameter3;	/*!< The packet's 3rdt parameter */

// Shortcut to accessing individual bytes as the structure of the packets are changed
// In the event of Tower sending 16bytes, this will write into both parameters 1 and 2
//#define Packet_Command     Packet.command
//#define Packet_Parameter1  Packet.parameters.separate.parameter1
//#define Packet_Parameter2  Packet.parameters.separate.parameter2
//#define Packet_Parameter3  Packet.parameters.separate.parameter3
//#define Packet_Parameter12 Packet.parameters.combined12.parameter12
//#define Packet_Parameter23 Packet.parameters.combined23.parameter23

// Acknowledgement bit mask
extern const uint8_t PACKET_ACK_MASK;

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz
 *  @return BOOL - TRUE if the packet module was successfully initialized.
 */
BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk);

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return BOOL - TRUE if a valid packet was received.
 */
BOOL Packet_Get(void);

/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return BOOL - TRUE if a valid packet was sent.
 */
BOOL Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3);

/*!
 * @}
*/

#endif
