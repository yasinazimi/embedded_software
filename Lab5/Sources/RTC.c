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
#include "types.h"
#include "MK70F12.h"

/*! @brief Initializes the RTC before first use.
 *
 *  Sets up the control register for the RTC and locks it.
 *  Enables the RTC and sets an interrupt every second.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return bool - TRUE if the RTC was successfully initialized.
 */
BOOL RTC_Init()
{
  RTCSemaphore = OS_SemaphoreCreate(0);

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

/*! @brief Sets the value of the real time clock.
 *
 *  @param hours The desired value of the real time clock hours (0-23).
 *  @param minutes The desired value of the real time clock minutes (0-59).
 *  @param seconds The desired value of the real time clock seconds (0-59).
 *  @note Assumes that the RTC module has been initialized and all input parameters are in range.
 */
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

/*! @brief Gets the value of the real time clock.
 *
 *  @param hours The address of a variable to store the real time clock hours.
 *  @param minutes The address of a variable to store the real time clock minutes.
 *  @param seconds The address of a variable to store the real time clock seconds.
 *  @note Assumes that the RTC module has been initialized.
 */
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

/*! @brief Interrupt service routine for the RTC.
 *
 *  The RTC has incremented one second.
 *  The semaphore signal will be called
 *  @note Assumes the RTC has been initialized.
 */
void __attribute__ ((interrupt)) RTC_ISR(void)
{
  OS_ISREnter();

  // Semaphore signals RTC semaphore
  OS_SemaphoreSignal(RTCSemaphore);

  OS_ISRExit();
}

/*!
 * @}
 */
