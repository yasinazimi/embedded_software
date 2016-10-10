/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  Implementation of functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-11
 */
 /*!
 * @addtogroup Analog_module Analog module documentation
 * @{
*/
#include "SPI.h"
#include "analog.h"
#include "MK70F12.h"

#define SPI_COMMAND 0b0000000000000001                  // 0x8400
#define LTC1859_COMMAND 0b10000100                      // 0XC400

TAnalogInput InputChannel;
TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

/*! @brief Sets up the ADC before first use.
 *
 *  @param moduleClock The module clock rate in Hz.
 *  @return BOOL - true if the UART was successfully initialized.
 */
BOOL Analog_Init(const uint32_t moduleClock)
{
  // TSPIModule setup
  TSPIModule aSPIModule;
  uint8_t channelNb = 0;

  // TSPIModule Setup
  aSPIModule.isMaster                     = bTRUE;      // Master
  aSPIModule.continuousClock              = bFALSE;   	// Continuous clock
  aSPIModule.inactiveHighClock            = bFALSE;   	// Inactive clock
  aSPIModule.changedOnLeadingClockEdge    = bFALSE;   	// Leading clock edge
  aSPIModule.LSBFirst                     = bFALSE;    	// MSB
  aSPIModule.baudRate                     = 1000000;    // 1Mbit

  SPI_Init(&aSPIModule, moduleClock);                   // Calls SPI Module

  // Sets a value to the analog input structures
  for (channelNb; channelNb < ANALOG_NB_INPUTS; channelNb++)
  {
    Analog_Input[channelNb].value.l = 0;                // The current analog value is set to 0
    Analog_Input[channelNb].oldValue.l = 0;             // The previous analog value is set to 0
    for (uint8_t sampleCount = 0; sampleCount < ANALOG_WINDOW_SIZE; sampleCount++)
    {
      Analog_Input[channelNb].values[sampleCount] = 0; 	// The sliding window value is set to 0
    }
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  }
}

/*! @brief Takes a sample from an analog input channel.
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @return BOOL - true if the channel was read successfully.
 */
BOOL Analog_Get(const uint8_t channelNb)
{
  SPI_SelectSlaveDevice(7);				// Selects DEC7
  uint16_t address;

  switch (channelNb)
  {
    case 1:
      // Channel 0
      // Single-Ended Channel Selection (+ / - 10V)
      // SPI_COMMAND 0b0000000000000001: CONT(1)| CTAS(3) | EOQ(1) | CTCNT(1) | Reserved(2)| Reserved(2) | PCS(5)
      address = 0x8400;
      break;
    case 0:
      // Channel 1
      // Single-Ended Channel Selection (+ / - 10V)
      // LTC1859_COMMAND 0b10000100: Single-Ended | Odd sign | Select 1 | Select 0 | UNI | Gain | NAP | Sleep
      address = 0xC400;
      break;
    default:
      return bFALSE;
  }

  // SPI_ExchangeChar(address, NULL);
  SPI_ExchangeChar(address, Analog_Input[channelNb].putPtr);	// Exchange the SPI command bits for CH0, send first, ADC command to initiate conversation
  SPI_ExchangeChar(address, Analog_Input[channelNb].putPtr);	// Exchange the SPI command bits for CH1

  // Checks if the analog input values is accordance to the window size (array - 1)
  if (Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[ANALOG_WINDOW_SIZE-1]))
  {
    // The current value is set to zero which points to the required channel
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  }
  else
    Analog_Input[channelNb].putPtr++;

  return bTRUE;
}

/*!
 * @}
*/
