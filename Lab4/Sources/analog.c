/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  Implementation of functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup Analog_module Analog module documentation
 * @{
 */
#include "SPI.h"
#include "analog.h"
#include "median.h"
#include "MK70F12.h"

TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

BOOL Analog_Init(const uint32_t moduleClock)
{
  // TSPIModule setup
  BOOL valid;
  TSPIModule aSPIModule;

  // TSPIModule Setup
  aSPIModule.isMaster                     = bTRUE;      // Master
  aSPIModule.continuousClock              = bFALSE;     // Continuous clock
  aSPIModule.inactiveHighClock            = bFALSE;     // Inactive clock
  aSPIModule.changedOnLeadingClockEdge    = bFALSE;     // Leading clock edge
  aSPIModule.LSBFirst                     = bFALSE;     // MSB
  aSPIModule.baudRate                     = 1000000;    // 1Mbit

  // Call SPI Module
  valid = SPI_Init(&aSPIModule, moduleClock);


  // Sets a value to the analog input structures
  for (uint8_t channelNb = 0; channelNb < ANALOG_NB_INPUTS; channelNb++)
  {
    // The current analog value is set to 0
    Analog_Input[channelNb].value.l = 0;
    // The previous analog value is set to 0
    Analog_Input[channelNb].oldValue.l = 0;

    for (uint8_t sampleCount = 0; sampleCount < ANALOG_WINDOW_SIZE; sampleCount++)
    {
      // The sliding window value is set to 0
      Analog_Input[channelNb].values[sampleCount] = 0;
    }
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  }
  return valid;
}

//static void wait(void)
//{
//  for (uint32_t i = 0; i < 1000; i++);
//}

BOOL Analog_Get(const uint8_t channelNb)
{
  // Selects DEC7
  SPI_SelectSlaveDevice(7);
  // Channel address
  uint16_t address;

  switch (channelNb)
  {
    case 0:
      // Channel 0
      // Single-Ended Channel Selection (+ / - 10V)
      address = 0x8400;
      break;
    case 1:
      // Channel 1
      // Single-Ended Channel Selection (+ / - 10V)
      address = 0xC400;
      break;
    default:
      return bFALSE;
  }

  uint16_t *value;

  // Checks if the analog input values is accordance to the window size (array - 1)
  if (Analog_Input[channelNb].putPtr == &(Analog_Input[channelNb].values[ANALOG_WINDOW_SIZE - 1]))
  {
    // The current value is set to zero which points to the required channel
    Analog_Input[channelNb].putPtr = &(Analog_Input[channelNb].values[0]);
  }
  else
  {
    Analog_Input[channelNb].putPtr++;
  }

  // Send command
  // SPI_ExchangeChar(address, (void *)0);
  SPI_ExchangeChar(address, Analog_Input[channelNb].putPtr);

  // Receive response to command
  // SPI_ExchangeChar(address, value);
  SPI_ExchangeChar(address, Analog_Input[channelNb].putPtr);

  //updates the old Vale to the value previously sampled
  Analog_Input[channelNb].oldValue = Analog_Input[channelNb].value;

  // stores the median value of the array
  Analog_Input[channelNb].value.l = Median_Filter(Analog_Input[channelNb].values, ANALOG_WINDOW_SIZE);

  return bTRUE;
}

/*!
 * @}
 */
