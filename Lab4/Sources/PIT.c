/*! @file
 *
 *  @brief Routines for controlling Periodic Interrupt Timer (PIT) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the periodic interrupt timer (PIT).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
 /*!
 * @addtogroup PIT_module PIT module documentation
 * @{
*/
#include "PIT.h"
#include "MK70F12.h"
#include "types.h"

static uint32_t PIT_moduleClk; 			/*!< Sets PIT count down value in PIT_Set according to arguments in PIT_Init() */
static void* PIT_UserArguments; 		/*!< Pointer to user arguments for PIT_Callback (Private Global) */
static void (*PIT_Callback)(void*); 		/*!< Pointer to a function */

/*! @brief Sets up the PIT before first use.
 *
 *  Enables the PIT and freezes the timer when debugging.
 *  @param moduleClk The module clock rate in Hz. NOTE DUE TO INTEGER MATHS AND REQUIREMENT
 *  THAT MODULECLK HAS PERIOD IN NANOSECONDS, MODULE CLOCK RATE MUST BE DIVISIBLE BY 1e6.
 *  @param userFunction is a pointer to a user callback function.
 *  @param userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return BOOL - TRUE if the PIT was successfully initialized.
 *  @note Assumes that moduleClk has a period which can be expressed as an integral number of nanoseconds.
 */
BOOL PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
  BOOL success;
  PIT_UserArguments = userArguments; 		// userArguments made global with a pointer
  PIT_Callback = userFunction;
  PIT_moduleClk = moduleClk;			// Saves PIT_Init() argument for use in PIT_Set()

  // Use System Clock Gating Control Register 6 to Enable Periodic Interrupt timer using PIT bit 23 (clock gate control)
  SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

  // Bits 0 & 1 only initialises PIT_MCR (Pit Module Control Register)
  PIT_MCR &= ~PIT_MCR_MDIS_MASK;               	// Enable clock for standard PIT Timers by clearing bit 1
  PIT_MCR |= PIT_MCR_FRZ_MASK;                	// Enable timers to keep running in cases where debugging
  PIT_Enable(bTRUE);

  // Enable interrupts
  PIT_TCTRL0 |= PIT_TCTRL_TIE_MASK; 		// Enable PIT Channel 0 interrupts

  // NVIC Register Masks (RM: Page 95 - 98)
  NVICICPR2 = (1<<(68 % 32));                   // Clear any pending interrupts on PIT Channel 0
  NVICISER2 = (1<<(68 % 32));                   // Enable interrupts from PIT Channel 0

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
  uint64_t  countdownValue, moduleClkMHz;

  moduleClkMHz = PIT_moduleClk / 1e6;
  countdownValue = (((period*moduleClkMHz)) / 1e3);

  // Noted integer calculations required
  if (restart == bTRUE) 			// PIT is disabled, a new countdown value set, and then enabled.
  {
    PIT_Enable(bFALSE);
    PIT_LDVAL0 = (uint32_t)countdownValue;	// Loads new countdown value to the PIT0 as per example p1339 K70 Resource
    PIT_Enable(bTRUE); 				// Because PIT0  starts counting down towards 0
  }
  else 						// Loads up the new value to the PIT0 after PIT0 trigger event
  {
    PIT_LDVAL0 = (uint32_t)countdownValue;
  }
}

/*! @brief Enables or disables the PIT.
 *
 *  @param enable - TRUE if the PIT is to be enabled, FALSE if the PIT is to be disabled.
 */
void PIT_Enable(const BOOL enable)
{
  if(enable == bFALSE)
  {
    PIT_TCTRL0 &= ~PIT_TCTRL_TEN_MASK;		// PIT Timer 0 is disabled
  }
  else
  {
    PIT_TCTRL0 |= PIT_TCTRL_TEN_MASK;		// PIT Timer 0 is enabled
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
  if (PIT_Callback)				// If pointer to this function has been initialised with an address of a function when PIT_Init called
  {
    (*PIT_Callback) (PIT_UserArguments);
  }
}

/*!
 * @}
*/
