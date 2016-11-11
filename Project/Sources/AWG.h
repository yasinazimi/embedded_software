/*! @file
 *
 *  @brief Routines for calculating the digital output of a waveform to the DSO.
 *
 *  This contains the functions to output a waveform with variable amplitude, frequency and offset.
 *
 *  @author PMcL
 *  @date 2016-11-09
 */
#ifndef AWG_H
#define AWG_H

#include "types.h"

#define POSITIVE_WAVEFORM_RANGE 32767
#define NEGATIVE_WAVEFORM_RANGE -32767

typedef enum
{
  STATUS_CHECK		= 0,
  WAVEFORM_CHANGE    	= 1,
  FREQUENCY_CHANGE   	= 2,
  AMPLITUDE_CHANGE   	= 3,
  OFFSET_CHANGE      	= 4,
  CHANNEL_START   	= 5,
  CHANNEL_STOP    	= 6,
  CHANNEL_CHANGE  	= 7
}TFGControl;

typedef enum
{
  SINE_WAVE	  	= 0,
  SQUARE_WAVE    	= 1,
  TRIANGLE_WAVE  	= 2,
  SAWTOOTH_WAVE  	= 3,
  NOISE_WAVE     	= 4,
  ARBITRARY_WAVE 	= 5
}TWaveform;

typedef struct
{
  TWaveform     	waveformType;
  uint16union_t 	frequency;
  uint16union_t 	amplitude;
  int16union_t  	offset;
}TAWGSettings;

typedef struct
{
  BOOL     		active;
  TAWGSettings		output;
}TChannel;

/*! @brief Initialises all the waveforms being used.
 *
 *  @param sampleFrequency The frequency sample based on the sample.
 *  @return void.
 */
void AWG_Init(const uint16_t sampleFrequency);

/*! @brief Digital outputs the required waveform.
 *
 *  @param aAWGSettings Struct containing the parameters of the waveform.
 *  @param sampleNb The number of samples based on the period.
 */
int16_t AWG_Output(const TAWGSettings aAWGSettings, const uint16_t sampleNb);

/*! @brief Calculates the sampling output of the waveform.
 *
 *  @param aAWGSettings Struct containing the parameters of the waveform.
 *  @param sampleNb The number of samples based on the period.
 *  @return void.
 */
void SamplePeriod(const TAWGSettings aAWGSettings, const uint16_t sampleNb);

/*! @brief Checks if the magnitude of the waveform is larger than 10V.
 *
 *  @return void.
 */
void MangnitudeCheck();

#endif
