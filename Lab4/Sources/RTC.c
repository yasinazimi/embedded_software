/*! @file
 *
 *  @brief Routines for controlling the Real Time Clock (RTC) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the real time clock (RTC).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
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
/*!< Pointer to RTC user callback function */
static void (*RTC_Callback)(void *);
/*!< Pointer to the user arguments to use  with the user callback function */
static void* RTC_Arguments;

BOOL RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  // Globally accessible (userFunction)
  RTC_Callback = userFunction;
  // Globally accessible (userArguments)
  RTC_Arguments = userArguments;

  // Enable clock gate RTC
  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;

  // Enables oscillator for external signal
  RTC_CR |= RTC_CR_OSCE_MASK;
  // Enable 18pF load as per lab note hint
  RTC_CR = RTC_CR | RTC_CR_SC16P_MASK | RTC_CR_SC2P_MASK;

  // RTC Interrupt Enable Register
  // Enables seconds enable interrupt (on by default)
  RTC_IER |= RTC_IER_TSIE_MASK;
  // Disables Time Alarm Interrupt
  RTC_IER &= ~RTC_IER_TAIE_MASK;
  // Disables Overflow Interrupt
  RTC_IER &= ~RTC_IER_TOIE_MASK;
  // Disables Time Invalid Interrupt
  RTC_IER &= ~RTC_IER_TIIE_MASK;

  // Locks the control register
  RTC_LR &= ~RTC_LR_CRL_MASK;
  // Initialises the timer control
  RTC_SR |= RTC_SR_TCE_MASK;

  // NVIC Register Masks (RM: Page 95 - 98)
  // Clear any pending 'seconds' interrupts on RTC using IRQ value
  NVICICPR2 = (1<<(67 % 32));
  // Enable 'seconds' interrupts on RTC
  NVICISER2 = (1<<(67 % 32));

  // RTC successfully initialised
  return bTRUE;
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  // Intermediate value recording day count of TSR
  uint32_t daysTSR, TSRregOut, TSRregIn, hoursGMTtime;
  // GMT delay for Australia noted
  uint8_t GMTchangeHours = 10;
  // Disables the time counter
  RTC_SR &= ~RTC_SR_TCE_MASK;
  // Calculates the value of real time clock with imperfect error handling
  uint32_t timeSeconds = ((hours % 24) * 3600) + ((minutes % 60) * 60) + (seconds % 60);

  // Note TSR value at start of function
  TSRregIn = RTC_TSR;

  // Hours since 1/1/70 0:00 as a remainder of 24
  hoursGMTtime = (RTC_TSR / 3600) % 24;

  if(hoursGMTtime + GMTchangeHours > 23)
  {
    // Because Setting time in location on other side of GMT dateline
    daysTSR= (RTC_TSR / (24*60*60)) + 1;
  }
  else
  {
    // Because setting time in location at time where date is shared with date of GMT time
    daysTSR= (RTC_TSR / (24*60*60));
  }

  // Resets Time Seconds Register UNIX time
  RTC_TSR = timeSeconds + (daysTSR * 24 * 60 * 60) - (GMTchangeHours*60*60);
  TSRregOut = RTC_TSR;

  // Re-enables the time counter
  RTC_SR |= RTC_SR_TCE_MASK;
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  uint32_t UNIXtime;
  // AEST time is GMT + 10 hours
  uint8_t GMT = 10;

  // Un-required variable, declared for interpretation purposes only
  UNIXtime = RTC_TSR;
  // Updates the current time value of hours for Sydney (GMT+10)
  *hours = ((UNIXtime / 3600) + GMT) % 24;
  // Updates the current time value of minutes
  *minutes = (UNIXtime / 60) % 60;
  // Updates the current time value of seconds
  *seconds = UNIXtime % 60;
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
  // If pointer to function is initialised
  if(RTC_Callback)
    // Calls the RTC ISR callback function
    (*RTC_Callback)(RTC_Arguments);
}

/*!
 * @}
 */
