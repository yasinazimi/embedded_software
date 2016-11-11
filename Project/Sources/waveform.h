/*! @file
 *
 *  @brief Routine to calculate the digital output of multiple waveforms with variable frequency, amplitude and offset.
 *
 *  This contains the functions used to digital output waveforms.
 *
 *  @author PMcL
 *  @date 2016-11-09
 */
#ifndef WAVEFORM_H
#define WAVEFORM_H

// New types
#include "types.h"

/*! @brief Sets up the waveforms for the square waveform.
 *
 *  @param sampleFrequency The samples taken at the particular frequency.
 *  @return void.
 */
void Square_Init(const uint16_t sampleFrequency);

/*! @brief Calculates the digital output of the square waveform.
 *
 *  @param frequency The frequency in Q notation with 12 decimal accuracy.
 *  @param sampleNb The module clock rate in Hz.
 */
int32_t Waveform_Square(const uint16_t frequency, const uint16_t sampleNb);

/*! @brief Sets up the waveforms for the sawtooth waveform.
 *
 *  @param sampleFrequency The samples taken at the particular frequency.
 *  @return void.
 */
void Sawtooth_Init(const uint16_t sampleFrequency);

/*! @brief Calculates the digital output of the sawtooth waveform.
 *
 *  @param frequency The frequency in Q notation with 12 decimal accuracy.
 *  @param sampleNb The module clock rate in Hz.
 */
int32_t Waveform_Sawtooth(const uint16_t frequency, const uint16_t sampleNb);

#endif
