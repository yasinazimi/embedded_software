/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  Implementation of functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
/*!
 * @addtogroup Analog_module Analog module documentation
 * @{
 */
#include "SPI.h"
#include "analog.h"
#include "PE_Types.h"

/*! @brief Sets up the ADC before first use.
 *
 *  @param moduleClk The module clock rate in Hz.
 *  @return BOOL - TRUE if the UART was successfully initialized.
 */
BOOL Analog_Init(const uint32_t moduleClock)
{
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

  // Sets all DACs to -10V to +10V bipolar range
  SPI_Exchange(SET_ALL_DACS_BIPOLAR_FIRST_WORD, NULL, 1, bTRUE);
  SPI_Exchange(SET_ALL_DACS_BIPOLAR_SECOND_WORD, NULL, 1, bFALSE);

  // Sets all DACs to mid-scale
  SPI_Exchange(SET_ALL_DACS_TO_MIDSCALE_FIRST_WORD, NULL, 1, bTRUE);
  SPI_Exchange(SET_ALL_DACS_TO_MIDSCALE_SECOND_WORD, NULL, 1, bFALSE);
  //SPI_Exchange(0xFFFF, NULL, 1, bFALSE);

  // Updates all DACs for both span and code
  SPI_Exchange(UPDATE_ALL_DACS_FIRST_WORD, NULL, 1, bTRUE);
  SPI_Exchange(UPDATE_ALL_DACS_SECOND_WORD, NULL, 1, bFALSE);

  return valid;
}

/*! @brief Puts a digital representation of the analog wave to the respective channel in the DSO.
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @param data is the value of the analog output to write
 *  @return BOOL - TRUE if the channel was read successfully.
 */
BOOL Analog_Put(const uint8_t channelNb, const uint16_t value)
{
  // Selects LTC2704 DAC
  SPI_SelectSlaveDevice(LTC2704);

  // Channel 0
  if (channelNb == 0)
    // Sets DAC A and writes to B1 and updates B2
    SPI_Exchange(SET_DAC_A_WRITE_B1_CODE_UPDATE_B2, NULL, 1, bTRUE);
  // Channel 1
  else if (channelNb == 1)
    // Sets DAC B and writes to B1 and updates B2
    SPI_Exchange(SET_DAC_B_WRITE_B1_CODE_UPDATE_B2, NULL, 1, bTRUE);

  // Updates the data value in analog put
  SPI_Exchange(value, NULL, 1, bFALSE);

  return bTRUE;
}

/*!
 * @}
 */
