/*! @file
 *
 *  @brief Routines for setting up the flexible timer module (FTM) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the flexible timer module (FTM).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-09-20
 */
 /*!
 * @addtogroup Timer_module Timer module documentation
 * @{
*/
#include "FTM.h"
#include "LEDs.h"
#include "IO_Map.h"
#include "MK70F12.h"

#define CHANNEL_COUNT 8					// Number of channels in flexible timer module
#define FIXED_FREQUENCY_CLOCK 2				// Value for the system's fixed frequency clock

static void (*Callback)(void *);			/*!< Pointer to FTM user callback function */
static void* Arguments;         			/*!< Pointer to the user arguments to use with the user callback function */

BOOL Timer_Init()
{
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;			// Enables the RTC module clock from SIM Register Masks
  FTM0_SC |= FTM_SC_CLKS(FIXED_FREQUENCY_CLOCK);	// Enable FTM overflow interrupts, up counting mode

  return bTRUE;						// FTM successfully initialised
}

BOOL FTM_Set(const TFTMChannel* const aFTMChannel)
{
  if (aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)
  {
    // Channel Mode Select set for input
    FTM0_CnSC(aFTMChannel->channelNb) &= ~(FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK);
  }
  else
  {
    // Channel Mode Select set for output
    FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;
  }

  // See Reference Manual on page 1219
  switch (aFTMChannel->ioType.inputDetection)
  {
    case 1:							// Capture on rising edge only
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSB_MASK;	// (0) Lock
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;	// (1) Set
      break;
    case 2:							// Capture on falling edge only
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK;	// (1) Set
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSA_MASK;	// (0) Lock
      break;
    case 3:							// Capture on rising or falling edge
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSB_MASK;	// (1) Set
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_ELSA_MASK;	// (1) Set
      break;
    default:							// Pin not used for FTM, revert to GPIO
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSB_MASK;	// (0) Lock
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSA_MASK;	// (0) Lock
      break;
  }
  Callback = aFTMChannel->userFunction;				// Globally accessible (userFunction)
  Arguments = aFTMChannel->userArguments;			// Globally accessible (userArguments)
}

BOOL FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  // Checks whether a channel is valid
  if (aFTMChannel->channelNb < CHANNEL_COUNT)
  {
    if (aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
    {
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;			// Enables the Channel Interrupt Enable (CHIE) mask
      FTM0_CnV(aFTMChannel->channelNb) = FTM0_CNT + aFTMChannel->delayCount;	// Sets the channels initial count
      return bTRUE;								// Timer successfully initialised
    }
  }
}

void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  (*Callback)(Arguments);					// FTM ISR callback function
}

/*!
 * @}
*/
