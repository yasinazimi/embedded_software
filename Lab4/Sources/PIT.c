/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
 /*!
 * @addtogroup PIT_module PIT module documentation
 * @{
*/
#include "PIT.h"
#include "MK70F12.h"
#include "types.h"

/*!< Sets PIT count down value in PIT_Set according to arguments in PIT_Init() */
static uint32_t PITClockPeriod;
/*!< Pointer to user arguments for PIT_Callback (Private Global) */
static void* PIT_UserArguments;
/*!< Pointer to a function */
static void (*PIT_Callback)(void*);

BOOL PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
  BOOL success;
  // userArguments made global with a pointer
  PIT_UserArguments = userArguments;
  PIT_Callback = userFunction;

  //calculates clock perios in nanoseconds
  PITClockPeriod = 1000000000/moduleClk;

  // Use System Clock Gating Control Register 6 to Enable Periodic Interrupt timer using PIT bit 23 (clock gate control)
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  // Bits 0 & 1 only initialises PIT_MCR (Pit Module Control Register)
  // Enable clock for standard PIT Timers by clearing bit 1
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;
  // Enable timers to keep running in cases where debugging
  PIT_MCR |= PIT_MCR_FRZ_MASK;

  // Enable interrupts
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;     // Enable PIT Channel 0 interrupts

  // NVIC Register Masks (RM: Page 95 - 98)
  // Clear any pending interrupts on PIT Channel 0
  NVICICPR2 = (1<<(68 % 32));
  // Enable interrupts from PIT Channel 0
  NVICISER2 = (1<<(68 % 32));

  return bTRUE;
}

void PIT_Set(const uint32_t period, const BOOL restart)
{
  //disables PIT if a restart was requested
  if (restart)
  {
    PIT_Enable(bFALSE);
  }

  //sets timer value
  PIT_LDVAL0 |= (period/PITClockPeriod) - 1;
  //re-enables PIT if a reset was requested
  if (restart)
  {
    PIT_Enable(bTRUE);
    //enables timer interrupts
  }

  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
}

void PIT_Enable(const BOOL enable)
{
  if(enable)
  {
    // PIT Timer 0 is enabled
    PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;
  }
  else
  {
    // PIT Timer 0 is disabled
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;
  }
}

void __attribute__ ((interrupt)) PIT_ISR(void)
{
  // If pointer to this function has been initialised with an address of a function when PIT_Init called
  if (PIT_Callback)
    (*PIT_Callback) (PIT_UserArguments);
}

/*!
 * @}
*/
