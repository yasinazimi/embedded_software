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
#include "packet.h"
#include "UART.h"
#include "FIFO.h"
#include "CPU.h"

static uint8_t Packet_Checksum;
TPacket Packet;

BOOL Packet_Init(const uint32_t baudRate, const uint32_t moduleClk)
{
  //Because we want to set up the Tower Module to send and receive packets
  return UART_Init(baudRate, moduleClk);
}

BOOL Packet_Get(void)
{
  // Enter critical component
  EnterCritical();
  static uint8_t packetFiniteState = 0;

  // State cases
  switch (packetFiniteState)
  {
    case 0:
      if (UART_InChar(&Packet_Command))
        // Because could not "Get" Packet_Command from RxFIFO
        packetFiniteState = 1;
      break;
    case 1:
      if (UART_InChar(&Packet_Parameter1)==bTRUE)
        // Because could not "Get" Packet_Parameter1 from RxFIFO
        packetFiniteState = 2;
      break;
    case 2:
      if (UART_InChar(&Packet_Parameter2)==bTRUE)
        // Because could not "Get" Packet_Parameter2 from RxFIFO
        packetFiniteState = 3;
      break;
    case 3:
      if (UART_InChar(&Packet_Parameter3)==bTRUE)
        // Because could not "Get" Packet_Parameter3 from RxFIFO
        packetFiniteState = 4;
      break;
    case 4:
      if (UART_InChar(&Packet_Checksum)==bTRUE)
        // Because could not "Get" Packet_Checksum from RxFIFO
        packetFiniteState = 5;
      break;
    case 5:
      if ( (((Packet_Command ^ Packet_Parameter1) ^ Packet_Parameter2) ^ Packet_Parameter3) == Packet_Checksum  )
      {
        // Because we want to reset the finite state machine
        packetFiniteState = 0;
        // Exit critical component
        ExitCritical();
        // Because we were able to "Get" correctly structured packet form RxFIFO
        return bTRUE;
      }
      else
      {
        packetFiniteState = 4;
        Packet_Command = Packet_Parameter1;
        Packet_Parameter1 = Packet_Parameter2;
        Packet_Parameter2 = Packet_Parameter3;
        Packet_Parameter3 = Packet_Checksum;
      }
      break;
  }
  ExitCritical();

  return bFALSE;
}

BOOL Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  // Check if all packet parameters were successfully moved into the transmit FIFO buffer
  if (UART_OutChar(command)
      && UART_OutChar(parameter1)
      && UART_OutChar(parameter2)
      && UART_OutChar(parameter3)
      && UART_OutChar(command ^ parameter1 ^ parameter2 ^ parameter3))
  {
    return bTRUE;
  }

  return bFALSE;
}

/*!
 * @}
 */
