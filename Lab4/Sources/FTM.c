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

static void (*Callback[CHANNEL_COUNT])(void *);			/*!< Pointer to FTM user callback function */
static void* Arguments[CHANNEL_COUNT];         			/*!< Pointer to the user arguments to use with the user callback function */

/*! @brief Sets up the FTM before first use.
 *
 *  Enables the FTM as a free running 16-bit counter.
 *  @return BOOL - TRUE if the FTM was successfully initialized.
 */
BOOL FTM_Init()
{
  // RTC Clock
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;			// Enables the RTC module clock from SIM Register Masks
  FTM0_SC |= FTM_SC_CLKS(FIXED_FREQUENCY_CLOCK);	// Enable FTM overflow interrupts, up counting mode

  // FTM0 Register instances (RM: Page 1217 - 1223)
  FTM0_CNTIN = ~FTM_CNTIN_INIT_MASK;			// Checks initial value of counter for space
  FTM0_CNT = ~FTM_CNT_COUNT_MASK;			// Checks counter value
  FTM0_MOD = FTM_MOD_MOD_MASK;				// Initialises FTM counter by writing to CNT

  // NVIC Register Masks (RM: Page 95 - 98)
  NVICICPR1 = (1<<(62 % 32));                   	// Clears pending interrupts on FMT0 module
  NVICISER1 = (1<<(62 % 32));                   	// Enables interrupts on FTM0 module

  return bTRUE;						// FTM successfully initialised
}

/*! @brief Sets up a timer channel.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *    channelNb is the channel number of the FTM to use.
 *    delayCount is the delay count (in module clock periods) for an output compare event.
 *    timerFunction is used to set the timer up as either an input capture or an output compare.
 *    ioType is a union that depends on the setting of the channel as input capture or output compare:
 *      outputAction is the action to take on a successful output compare.
 *      inputDetection is the type of input capture detection.
 *    userFunction is a pointer to a user callback function.
 *    userArguments is a pointer to the user arguments to use with the user callback function.
 *  @return BOOL - TRUE if the timer was set up successfully.
 *  @note Assumes the FTM has been initialized.
 */
BOOL FTM_Set(const TFTMChannel* const aFTMChannel)
{
  if (aFTMChannel->timerFunction == TIMER_FUNCTION_INPUT_CAPTURE)
  {
    // Channel Mode Select set for input
    FTM0_CnSC(aFTMChannel->channelNb) &= ~(FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK);
  }
  else
  {
    // Channel(n) Status and Controls (RM: Page 1219 - 1223)
    FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;	// Sets control for output
    FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;	// Channel Mode Select set for output
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

  if (FTM0_CnSC(aFTMChannel->channelNb) & FTM_CnSC_CHF_MASK)
    FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;//clear channel flag by writing 0 to it

  Callback[aFTMChannel->channelNb] = aFTMChannel->userFunction;				// Globally accessible (userFunction)
  Arguments[aFTMChannel->channelNb] = aFTMChannel->userArguments;			// Globally accessible (userArguments)
}

/*! @brief Starts a timer if set up for output compare.
 *
 *  @param aFTMChannel is a structure containing the parameters to be used in setting up the timer channel.
 *  @return BOOL - TRUE if the timer was started successfully.
 *  @note Assumes the FTM has been initialized.
 */
BOOL FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  uint16_t val;

  if (aFTMChannel->channelNb < CHANNEL_COUNT)			// Checks whether a channel is valid
  {
    if (aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
    {
      if (FTM0_CnSC(aFTMChannel->channelNb) & FTM_CnSC_CHF_MASK)
        FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_CHF_MASK;//clear channel flag by writing 0 to it

      val = FTM0_CNT + aFTMChannel->delayCount;
      FTM0_CnV(aFTMChannel->channelNb) = FTM_CnV_VAL(val);	// Sets the channels initial count
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;			// Enables the Channel Interrupt Enable (CHIE) mask
      return bTRUE;								// Timer successfully initialised
    }
  }
}

/*! @brief Interrupt service routine for the FTM.
 *
 *  If a timer channel was set up as output compare, then the user callback function will be called.
 *  @note Assumes the FTM has been initialized.
 */
void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  uint8_t channelNb;

  for(channelNb = 0; channelNb < 8; channelNb++)
  {
    if (FTM0_CnSC(channelNb) & FTM_CnSC_CHF_MASK)	        // Checks for the channel flag
      {


	if (FTM0_CnSC(channelNb) & FTM_CnSC_CHIE_MASK)
	{
	    FTM0_CnSC(channelNb) &= ~FTM_CnSC_CHIE_MASK; //disable interrupts
	    (*Callback[channelNb])(Arguments[channelNb]);//callback function
	}



      }

      				// FTM ISR callback function
  }

}

/*!
 * @}
*/
