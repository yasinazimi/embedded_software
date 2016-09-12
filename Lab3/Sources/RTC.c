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
 * @addtogroup RTC_module RTC module documentation
 * @{
*/
#include "RTC.h"
#include "LEDs.h"
#include <types.h>
#include <stdint.h>
#include "MK70F12.h"

// Declare pointers
static void (*Callback)(void *);	/*!< Pointer to RTC user callback function */
static void* Arguments;			/*!< Pointer to the user arguments to use  with the user callback function */

BOOL RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  Callback = userFunction;		// Globally accessible (userFunction)
  Arguments = userArguments;		// Globally accessible (userArguments)

  // RTC Interrupt Enable Register
  RTC_IER |= RTC_IER_TSIE_MASK;		// Enables interrupt
  RTC_IER &= ~RTC_IER_TAIE_MASK;	// Disables Time Alarm Interrupt
  RTC_IER &= ~RTC_IER_TOIE_MASK;	// Disables Overflow Interrupt
  RTC_IER &= ~RTC_IER_TIIE_MASK;	// Disables Time Invalid Interrupt

  // Clears the error if the Invalid Timer flag is set
  if (RTC_SR & RTC_SR_TIF_MASK)
  {
    RTC_TSR = 0;			// Timer Second Register (TSR) is set to zero for reset
  }

  RTC_LR &= ~RTC_LR_CRL_MASK;		// Locks the control register
  RTC_SR |= RTC_SR_TCE_MASK;		// Initialises the timer control

  // NVIC Register Masks
  NVICICPR2 |= (1<<3); 			// Clears pending interrupts on UART2
  NVICISER2 |= (1<<3); 			// Enables interrupts on RTC module

  return bTRUE;				// RTC successfully initialised
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  RTC_SR &= ~RTC_SR_TCE_MASK;									// Disables the time counter
  uint32_t timeSeconds = ((hours % 24) * 3600) + ((minutes % 60) * 60) + (seconds % 60);	// Calculates the value of real time clock
  RTC_TSR = timeSeconds;									// Puts the value of the time to the RTC_TSR(Time Seconds Register)
  RTC_SR |= RTC_SR_TCE_MASK;									// Re-enables the time counter
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  uint32_t currentTime = RTC_TSR;	// Reads the current time from the RTC_TSR
  *hours = currentTime / 3600;		// Updates the current time value of hours
  *minutes = currentTime / 60 % 60;	// Updates the current time value of minutes
  *seconds = currentTime % 60;		// Updates the current time value of seconds
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
  (*Callback)(Arguments);		// Calls the RTC ISR callback function
}

/*!
 * @}
*/

