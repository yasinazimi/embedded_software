/*! @file UART.c
 *
 *  @brief Implementation of functions to calculate the digital output of waveforms to the DSO.
 *
 *  Implementation of functions to output waveforms with variable frequency, amplitude and offse.t
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
/*!
 * @addtogroup Waveform_module Waveform module documentation
 * @{
 */
#include "types.h"
#include "waveform.h"

#define FQ12Notation 12

// This is half the period
static uint32_t squareLimit;
static uint32_t sawtoothLimit;

/*! @brief Sets up the waveforms for the square waveform.
 *
 *  @param sampleFrequency The samples taken at the particular frequency.
 *  @return void.
 */
void Square_Init(const uint16_t sampleFrequency)
{
  squareLimit = (sampleFrequency << FQ12Notation) / 2;
}

/*! @brief Calculates the digital output of the square waveform.
 *
 *  @param frequency The frequency in Q notation with 12 decimal accuracy.
 *  @param sampleNb The module clock rate in Hz.
 */
int32_t Waveform_Square(const uint16_t frequency, const uint16_t sampleNb)
{
  int32_t outcome;
  uint32_t samplePosition = sampleNb * frequency;

  // variable uses only 1 period
  samplePosition = samplePosition % (2 * squareLimit);

  // Calculates the output (-1 or +1)
  outcome = (1 << FQ12Notation);

  if (samplePosition >= squareLimit)
  {
    // Checks whether the output of the wave should be negative
    outcome = -outcome;
  }

  return outcome;
}

/*! @brief Sets up the waveforms for the sawtooth waveform.
 *
 *  @param sampleFrequency The samples taken at the particular frequency.
 *  @return void.
 */
void Sawtooth_Init(const uint16_t sampleFrequency)
{
  sawtoothLimit = (sampleFrequency) << FQ12Notation;
}

/*! @brief Calculates the digital output of the sawtooth waveform.
 *
 *  @param frequency The frequency in Q notation with 12 decimal accuracy.
 *  @param sampleNb The module clock rate in Hz.
 */
int32_t Waveform_Sawtooth(const uint16_t frequency, const uint16_t sampleNb)
{
  int32_t outcome;
  uint32_t samplePosition = sampleNb * frequency;

  // Variable uses only 1 period
  samplePosition = samplePosition % sawtoothLimit;

  // Calculates the output with the current sample position
  outcome = (2 * samplePosition) / (sawtoothLimit >> FQ12Notation);
  // Result is calculated with
  outcome -= (1 << FQ12Notation);

  return outcome;
}

/*!
 ** @}
 */
