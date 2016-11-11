/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Mohammad Yasin Azimi
 *  @date 2016-11-09
 */
 /*!
 * @addtogroup PIT_module PIT module documentation
 * @{
*/
#include "OS.h"
#include "PIT.h"
#include "LEDs.h"
#include "types.h"
#include "MK70F12.h"

static uint32_t PITClockPeriod;

/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz. NOTE DUE TO INTEGER MATHS AND REQUIREMENT
 *  THAT MODULECLK HAS PERIOD IN NANOSECONDS, MODULE CLOCK RATE MUST BE DIVISIBLE BY 1e6.
 *  @return BOOL - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
BOOL PIT_Init(const uint32_t moduleClk)
{
  BOOL success;

  // PIT semaphore for sampling
  PITSemaphore = OS_SemaphoreCreate(0);

  // calculates clock period in nanoseconds
  PITClockPeriod = 1000000000 / moduleClk;

  // Use System Clock Gating Control Register 6 to Enable Periodic Interrupt timer using PIT bit 23 (clock gate control)
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  // Enable clock for standard PIT Timers by clearing bit 1
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;
  // Enable timers to keep running in cases where debugging
  PIT_MCR |= PIT_MCR_FRZ_MASK;

  // Enable PIT Channel 0 interrupts
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;

  // Clear any pending interrupts on PIT Channel 0
  NVICICPR2 = (1<<(68 % 32));
  // Enable interrupts from PIT Channel 0
  NVICISER2 = (1<<(68 % 32));

  return bTRUE;
}

/*! @brief Sets the value of the desired period of the PIT.
 *
 *  @param period The desired value of the timer period in nanoseconds.
 *  @param restart TRUE if the PIT is disabled, a new value set, and then enabled.
 *                 FALSE if the PIT will use the new value after a trigger event.
 *  @note The function will enable the timer and interrupts for the PIT.
 */
void PIT_Set(const uint32_t period, const BOOL restart)
{
  // Disables PIT if a restart was requested
  if (restart)
  {
    PIT_Enable(bFALSE);
  }

  //sets timer value
  PIT_LDVAL0 |= (period/PITClockPeriod) - 1;

  //re-enables PIT if a reset was requested
  if (restart)
    PIT_Enable(bTRUE);

  // Enable TIE MASK
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK;
}

/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
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

/*! @brief Interrupt service routine for the PIT.
 *
 *  The periodic interrupt timer has timed out.
 *  The user callback function will be called.
 *  @note Assumes the PIT has been initialized.
 */
void __attribute__ ((interrupt)) PIT_ISR(void)
{
  OS_ISREnter();

  // Enable TIF MASK
  PIT_TFLG0 |= PIT_TFLG_TIF_MASK;
  // Semaphore signals PIT semaphore
  OS_SemaphoreSignal(PITSemaphore);

  OS_ISRExit();
}

/*!
 * @}
*/
