/*! @file packet.c
 *
 *  @brief Contains the packet encoding and decoding for the serial port.
 *
 *  Implementation of functions for the "Tower to PC Protocol" 5-byte packets.
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
EnterCritical();
static uint8_t packetFiniteState = 0;
switch (packetFiniteState)
{
  case 0:
    if (UART_InChar(&Packet_Command))
      packetFiniteState = 1;			// Because could not "Get" Packet_Command from RxFIFO
    break;
  case 1:
    if (UART_InChar(&Packet_Parameter1)==bTRUE)
      packetFiniteState = 2;			// Because could not "Get" Packet_Parameter1 from RxFIFO
    break;
  case 2:
    if (UART_InChar(&Packet_Parameter2)==bTRUE)
      packetFiniteState = 3;			// Because could not "Get" Packet_Parameter2 from RxFIFO
    break;
  case 3:
    if (UART_InChar(&Packet_Parameter3)==bTRUE)
      packetFiniteState = 4;			// Because could not "Get" Packet_Parameter3 from RxFIFO
    break;
  case 4:
    if (UART_InChar(&Packet_Checksum)==bTRUE)
      packetFiniteState = 5;			// Because could not "Get" Packet_Checksum from RxFIFO
    break;
  case 5:
    if ( (((Packet_Command ^ Packet_Parameter1) ^ Packet_Parameter2) ^ Packet_Parameter3) == Packet_Checksum  )
    {
      packetFiniteState = 0;			// Because we want to reset the finite state machine
      ExitCritical();
      return bTRUE;				// Because we were able to "Get" correctly structured packet form RxFIFO
    }
    else
    {
      packetFiniteState = 4;
      Packet_Command=Packet_Parameter1;
      Packet_Parameter1=Packet_Parameter2;
      Packet_Parameter2=Packet_Parameter3;
      Packet_Parameter3=Packet_Checksum;
    }
    break;
}
ExitCritical();
return bFALSE;
}

BOOL Packet_Put(const uint8_t command, const uint8_t parameter1, const uint8_t parameter2, const uint8_t parameter3)
{
  BOOL succC, succP1, succP2, succP3, succPC;
  uint8_t packetChecksum = (((command ^ parameter1) ^ parameter2) ^ parameter3);

  succC=UART_OutChar(command);
  succP1=UART_OutChar(parameter1);
  succP2=UART_OutChar(parameter2);
  succP3=UART_OutChar(parameter3);
  succPC=UART_OutChar(packetChecksum);
  return(succC && succP1 && succP2 && succP3 && succPC);
}

/*!
 * @}
*/
