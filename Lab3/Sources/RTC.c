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
static void (*RTC_Callback)(void *);			/*!< Pointer to RTC user callback function */
static void* RTC_Arguments;				/*!< Pointer to the user arguments to use  with the user callback function */

BOOL RTC_Init(void (*userFunction)(void*), void* userArguments)
{
  RTC_Callback = userFunction;				// Globally accessible (userFunction)
  RTC_Arguments = userArguments;			// Globally accessible (userArguments)

  SIM_SCGC6 |= SIM_SCGC6_RTC_MASK;      		// Enable clock gate RTC

  RTC_CR |= RTC_CR_OSCE_MASK;          			// Enables oscillator for external signal
  RTC_CR = RTC_CR | RTC_CR_SC16P_MASK | RTC_CR_SC2P_MASK; // Enable 18pF load as per lab note hint

  // RTC Interrupt Enable Register
  RTC_IER |= RTC_IER_TSIE_MASK;				// Enables seconds enable interrupt (on by default)
  RTC_IER &= ~RTC_IER_TAIE_MASK;			// Disables Time Alarm Interrupt
  RTC_IER &= ~RTC_IER_TOIE_MASK;			// Disables Overflow Interrupt
  RTC_IER &= ~RTC_IER_TIIE_MASK;			// Disables Time Invalid Interrupt

  RTC_LR &= ~RTC_LR_CRL_MASK;				// Locks the control register
  RTC_SR |= RTC_SR_TCE_MASK;				// Initialises the timer control

  // NVIC Register Masks (RM: Page 95 - 98)
  NVICICPR2 = (1<<(67 % 32));                   	// Clear any pending 'seconds' interrupts on RTC using IRQ value
  NVICISER2 = (1<<(67 % 32));                   	// Enable 'seconds' interrupts on RTC

  // Set to Unix time Just once, then comment out these lines for the next time the DATE and time needs to be set
  //RTC_SR &= ~RTC_SR_TCE_MASK;                    	// Disables the time counter
  //RTC_TSR = 1474148659;   //744am today
  //1474112580  //12 hours later
  //RTC_SR |= RTC_SR_TCE_MASK;                    	// Re-enables the time counter

  return bTRUE;						// RTC successfully initialised
}

void RTC_Set(const uint8_t hours, const uint8_t minutes, const uint8_t seconds)
{
  uint32_t daysTSR, TSRregOut, TSRregIn, hoursGMTtime;  // Intermediate value recording day count of TSR
  uint8_t GMTchangeHours = 10;               		// GMT delay for Australia noted
  RTC_SR &= ~RTC_SR_TCE_MASK;				// Disables the time counter
  uint32_t timeSeconds = ((hours % 24) * 3600) + ((minutes % 60) * 60) + (seconds % 60); // Calculates the value of real time clock with imperfect error handling

  TSRregIn = RTC_TSR;                                  	//Note TSR value at start of function

  hoursGMTtime = (RTC_TSR / 3600) % 24;          	//Hours since 1/1/70 0:00 as a remainder of 24
  if(hoursGMTtime + GMTchangeHours > 23)
  {
    daysTSR= (RTC_TSR / (24*60*60)) + 1;           	//Because Setting time in location on other side of GMT dateline
  }
  else
  {
    daysTSR= (RTC_TSR / (24*60*60));              	//Because setting time in location at time where date is shared with date of GMT time
  }

  RTC_TSR = timeSeconds + (daysTSR * 24 * 60 * 60) - (GMTchangeHours*60*60); // Resets Time Seconds Register UNIX time
  TSRregOut = RTC_TSR;					//for debugging purposes

  RTC_SR |= RTC_SR_TCE_MASK;                    	// Re-enables the time counter
}

void RTC_Get(uint8_t* const hours, uint8_t* const minutes, uint8_t* const seconds)
{
  uint32_t UNIXtime;
  uint8_t GMT = 10;                             	// AEST time is GMT + 10 hours

  UNIXtime = RTC_TSR;                            	// Un-required variable, declared for interpretation purposes only
  *hours = ((UNIXtime / 3600) + GMT) % 24;            	// Updates the current time value of hours for Sydney (GMT+10)
  *minutes = (UNIXtime / 60) % 60;			// Updates the current time value of minutes
  *seconds = UNIXtime % 60;		                // Updates the current time value of seconds
}

void __attribute__ ((interrupt)) RTC_ISR(void)
{
  if(RTC_Callback)                      		// If pointer to function is initialised
  (*RTC_Callback)(RTC_Arguments);			// Calls the RTC ISR callback function
}

/*!
 * @}
*/
