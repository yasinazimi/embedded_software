/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the real time clock (RTC).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
 /*!
 * @addtogroup FRTC_module RTC module documentation
 * @{
*/
#include "MK70F12.h"
#include <stdint.h>
#include <types.h>
#include "RTC.h"
#include "LEDs.h"

bool RTC_Init(void (*userFunction)(void*), void* userArguments)
{
	//
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
	//
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
	//
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
	//
}

/*!
** @}
*/

