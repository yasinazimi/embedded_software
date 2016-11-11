/*! @file
 *
 *  @brief Routines for setting up and reading from the ADC.
 *
 *  This contains the functions needed for accessing the external TWR-ADCDAC-LTC board.
 *
 *  @author PMcL
 *  @date 2016-11-09
 */
#ifndef ANALOG_H
#define ANALOG_H

// new types
#include "types.h"
#include "SPI.h"

#define LTC2704 4
#define ANALOG_WINDOW_SIZE 5
#define SET_ALL_DACS_BIPOLAR_FIRST_WORD 		0x2F  		// 8 zeros   (8 bits)  | 0010 command (4 bits) | 1111 address (4 bits)
#define SET_ALL_DACS_BIPOLAR_SECOND_WORD 		0x03      	// 12 zeroes (12 bits) | 0011 span (4 bits)
#define SET_ALL_DACS_TO_MIDSCALE_FIRST_WORD             0x3F  		// 8 zeros   (8 bits)  | 0011 command (4 bits) | 1111 address (4 bits)
#define SET_ALL_DACS_TO_MIDSCALE_SECOND_WORD            0x8000    	// 1000 0000 0000 0000 data codes (16 bits) mid-scale
#define UPDATE_ALL_DACS_FIRST_WORD                      0x4F  		// 8 zeros   (8 bits)  | 0100 command (4 bits) | 1111 address (4 bits)
#define UPDATE_ALL_DACS_SECOND_WORD                     0x0       	// 0000 0000 0000 0000 don't cares (16 bits)
#define SET_DAC_A_WRITE_B1_CODE_UPDATE_B2               0x70  		// 8 zeros   (8 bits)  | 0111 command (4 bits) | 0000 address (4 bits)
#define SET_DAC_B_WRITE_B1_CODE_UPDATE_B2               0x72  		// 8 zeros   (8 bits)  | 0111 command (4 bits) | 0010 address (4 bits)

#pragma pack(push)
#pragma pack(2)

typedef struct
{
  int16union_t value;                  /*!< The current "processed" analog value (the user updates this value). */
  int16union_t oldValue;               /*!< The previous "processed" analog value (the user updates this value). */
  int16_t values[ANALOG_WINDOW_SIZE];  /*!< An array of sample values to create a "sliding window". */
  int16_t* putPtr;                     /*!< A pointer into the array of the last sample taken. */
} TAnalogOutput;

#pragma pack(pop)

/*! @brief Sets up the ADC before first use.
 *
 *  @param moduleClk The module clock rate in Hz.
 *  @return BOOL - TRUE if the UART was successfully initialized.
 */
BOOL Analog_Init(const uint32_t moduleClock);

/*! @brief Puts a digital representation of the analog wave to the respective channel in the DSO.
 *
 *  @param channelNb is the number of the analog input channel to sample.
 *  @param data is the value of the analog output to write
 *  @return BOOL - TRUE if the channel was read successfully.
 */
BOOL Analog_Put(const uint8_t channelNb, const uint16_t value);

#endif
