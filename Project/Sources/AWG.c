/*! @file
 *
 *  @brief Routines for calculating the digital output of a waveform to the DSO.
 *
 *  Implementation of functions for displaying waveforms.
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
/*!
 * @addtogroup AWG_module AWG module documentation
 * @{
*/
#include "AWG.h"
#include "types.h"
#include "waveform.h"

static uint8_t value;
static int32_t outcome;

/*! @brief Initialises all the waveforms being used.
 *
 *  @param sampleFrequency The frequency sample being stored.
 *  @return void.
 */
void AWG_Init(const uint16_t sampleFrequency)
{
  Square_Init(sampleFrequency);
  Sawtooth_Init(sampleFrequency);
}

/*! @brief Digital outputs the required waveform.
 *
 *  @param aAWGSettings Struct containing the parameters of the waveform.
 *  @param sampleNb The number of samples based on the period.
 */
int16_t AWG_Output(const TAWGSettings aAWGSettings, const uint16_t sampleNb)
{
  // Switch case to switch between the different waveforms
  switch (aAWGSettings.waveformType)
  {
    case SQUARE_WAVE:
      outcome = Waveform_Square((uint32_t)(aAWGSettings.frequency.l << 4), sampleNb);
      break;
    case SAWTOOTH_WAVE:
      outcome = Waveform_Sawtooth((uint32_t)(aAWGSettings.frequency.l << 4), sampleNb);
      break;
  }

  // Period sampling
  SamplePeriod(aAWGSettings, sampleNb);

  // Waveform magnitude
  MangnitudeCheck();

  // Return the outcome on success
  return ((int16_t)outcome);
}

/*! @brief Calculates the sampling output of the waveform.
 *
 *  @param aAWGSettings Struct containing the parameters of all the waveform.
 *  @param sampleNb The number of samples based on the period.
 *  @return void.
 */
void SamplePeriod(const TAWGSettings aAWGSettings, const uint16_t sampleNb)
{
  if (((sampleNb % 100) < 50) && (outcome < 0))
    value = 1;

  // The final output is calculated in volts
  outcome = (outcome * aAWGSettings.amplitude.l);
  outcome += (aAWGSettings.offset.l << 12);
  outcome /= 16;
  outcome = outcome >> 12;

  if (((sampleNb % 100) < 50) && (outcome < 0))
    value = 1;
}

/*! @brief Checks if the magnitude of the waveform is larger than 10V.
 *
 *  @return void.
 */
void MangnitudeCheck()
{
  // Checks if the final result is greater than the positive waveform range of 32767
  if (outcome > POSITIVE_WAVEFORM_RANGE)
    outcome = POSITIVE_WAVEFORM_RANGE;
  // Checks if the final result is less than the positive waveform range of -32767
  else if (outcome < NEGATIVE_WAVEFORM_RANGE)
    outcome = NEGATIVE_WAVEFORM_RANGE;
}

/*!
** @}
*/
