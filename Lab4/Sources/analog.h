/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author PMcL
 *  @date 2016-10-11
 */
#ifndef ANALOG_H
#define ANALOG_H

// new types
#include "types.h"
#include "SPI.h"

// Maximum number of channels
#define ANALOG_NB_INPUTS 2
#define ANALOG_WINDOW_SIZE 5

#pragma pack(push)
#pragma pack(2)

typedef struct
{
  int16union_t value;                  /*!< The current "processed" analog value (the user updates this value). */
  int16union_t oldValue;               /*!< The previous "processed" analog value (the user updates this value). */
  int16_t values[ANALOG_WINDOW_SIZE];  /*!< An array of sample values to create a "sliding window". */
  int16_t* putPtr;                     /*!< A pointer into the array of the last sample taken. */
} TAnalogInput;

#pragma pack(pop)

extern TAnalogInput Analog_Input[ANALOG_NB_INPUTS];

/*! @brief Sets up the ADC before first use.
 *
 *  @param moduleClk The module clock rate in Hz.
 *  @return BOOL - true if the UART was successfully initialized.
 */
BOOL Analog_Init(const uint32_t moduleClock);

/*! @brief Takes a sample from an analog input channel.
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @return BOOL - true if the channel was read successfully.
 */
BOOL Analog_Get(const uint8_t channelNb);

#endif
