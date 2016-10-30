/*! @file packet.c
 *
 *  @brief Contains the packet encoding and decoding for the serial port.
 *
 *  Implementation of functions for the "Tower to PC Protocol" 5-byte packets.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
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

/*! @brief Initializes the packets by calling the initialization routines of the supporting software modules.
 *
 *  @param baudRate The desired baud rate in bits/sec.
 *  @param moduleClk The module clock rate in Hz.
 *  @return BOOL - TRUE if the packet module was successfully initialized.
 */
BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  // Packet put semaphore created and set to 1
  PacketPutSemaphore = OS_SemaphoreCreate(1);
  // Packet semaphore created and set to 0
  PacketSemaphore = OS_SemaphoreCreate(0);

  return UART_Init(baudRate, moduleClk);
}

/*! @brief Attempts to get a packet from the received data.
 *
 *  @return BOOL - TRUE if a valid packet was received.
 */
BOOL Packet_Get(void)
{
  static uint8_t position = 0;

  // State cases
  switch (position)
  {
    case 0:
      UART_InChar(&Packet_Command);
      // Because could not "Get" Packet_Command from RxFIFO
      position++;
      return bFALSE;
      break;
    case 1:
      UART_InChar(&Packet_Parameter1);
      // Because could not "Get" Packet_Parameter1 from RxFIFO
      position++;
      return bFALSE;
      break;
    case 2:
      UART_InChar(&Packet_Parameter2);
      // Because could not "Get" Packet_Parameter2 from RxFIFO
      position++;
      return bFALSE;
      break;
    case 3:
      UART_InChar(&Packet_Parameter3);
      // Because could not "Get" Packet_Parameter3 from RxFIFO
      position++;
      return bFALSE;
      break;
    case 4:
      UART_InChar(&Packet_Checksum);
      // Because could not "Get" Packet_Checksum from RxFIFO
      position++;
      return bFALSE;
      break;
    case 5:
      if ((((Packet_Command ^ Packet_Parameter1) ^ Packet_Parameter2) ^ Packet_Parameter3) == Packet_Checksum )
      {
	// Because we want to reset the finite state machine
	position = 0;
	// Because we were able to "Get" correctly structured packet form RxFIFO
	return bTRUE;
      }
      else
      {
	Packet_Command = Packet_Parameter1;
	Packet_Parameter1 = Packet_Parameter2;
	Packet_Parameter2 = Packet_Parameter3;
	Packet_Parameter3 = Packet_Checksum;
      }
      return bFALSE;
      break;
    default:
      return bFALSE;
  }
  return bFALSE;
}

/*! @brief Builds a packet and places it in the transmit FIFO buffer.
 *
 *  @return BOOL - TRUE if a valid packet was sent.
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
