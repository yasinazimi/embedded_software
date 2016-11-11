/*! @file packet.c
 *
 *  @brief Contains the packet encoding and decoding for the serial port.
 *
 *  Implementation of functions for the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
/*!
 * @addtogroup Packet_module Packet module documentation
 * @{
 */
#include "UART.h"
#include "LEDs.h"
#include "packet.h"
#include "MK70F12.h"

TPacket Packet;
uint8_t const PACKET_ACK_MASK = 0x80;

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return BOOL - TRUE if the packet module was successfully initialized.
 */
BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  PacketPutSemaphore = OS_SemaphoreCreate(1);
  return UART_Init(baudRate, moduleClk);
}

/*! @brief Checks the packets
 *
 *  @return void.
 */
BOOL Packet_Validate(void)
{
  if (Packet_Checksum != (Packet_Command ^ Packet_Parameter1 ^ Packet_Parameter2 ^ Packet_Parameter3))
  {
    Packet_Command = Packet_Parameter1;
    Packet_Parameter1 = Packet_Parameter2;
    Packet_Parameter2 = Packet_Parameter3;
    Packet_Parameter3 = Packet_Checksum;

    return bFALSE;
  }
  return bTRUE;
}

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return BOOL - TRUE if a valid packet was received.
 */
BOOL Packet_Get(void)
{
  BOOL valid;

  for(uint8_t byte = 0; byte < 4; byte++)
    UART_InChar(&Packet.bytes[byte]);

  do
  {
    // Checks the packet
    UART_InChar(&Packet_Checksum);
    valid = Packet_Validate();
  } while (!valid);
}

/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return void.
 */
void Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  // Semaphore to wait for packet put
  OS_SemaphoreWait(PacketPutSemaphore, 0);

  // Packet commands
  UART_OutChar(command);
  UART_OutChar(parameter1);
  UART_OutChar(parameter2);
  UART_OutChar(parameter3);
  UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3);

  // Semaphore to signal packet put semaphore
  OS_SemaphoreSignal(PacketPutSemaphore);
}

/*!
 * @}
 */
