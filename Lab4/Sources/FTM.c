/*! @file
 *
 *  @brief Routines for setting up the flexible timer module (FTM) on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the flexible timer module (FTM).
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup Timer_module Timer module documentation
 * @{
 */
#include "FTM.h"
#include "LEDs.h"
#include "IO_Map.h"
#include "MK70F12.h"

// Number of channels in flexible timer module
#define CHANNEL_COUNT 8

/*!< Pointer to FTM user callback function */
static void (*Callback[CHANNEL_COUNT])(void *);
/*!< Pointer to the user arguments to use with the user callback function */
static void* Arguments[CHANNEL_COUNT];

BOOL FTM_Init()
{
  // Enables the RTC module clock from SIM Register Masks
  SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

  if (FTM0_FMS | FTM_FMS_WPEN_MASK)
  {
    // Disable write protection
    FTM0_MODE |= FTM_MODE_WPDIS_MASK;
  }

  // Checks initial value of counter for space
  FTM0_CNTIN   &= ~FTM_CNTIN_INIT_MASK;
  // Set initial value to 0
  FTM0_CNTIN   &= ~FTM_CNTIN_INIT_MASK;
  // 16 bit counter
  FTM0_MOD     |= FTM_MOD_MOD_MASK;
  // Update counter with initial value, CNTIN
  FTM0_CNT     |= FTM_CNT_COUNT(1);
  // Use fixed frequency clock
  FTM0_SC      |= FTM_SC_CLKS(2);

  // Setup for only input capture / output compare
  // Up counting mode
  FTM0_SC      &= ~FTM_SC_CPWMS_MASK;
  FTM0_QDCTRL  &= ~FTM_QDCTRL_QUADEN_MASK;
  // No channels are linked
  FTM0_COMBINE  = 0;

  // Enable FTM
  FTM0_MODE    |= FTM_MODE_FTMEN_MASK;
  // Enable write protection
  FTM0_FMS     |= FTM_FMS_WPEN_MASK;

  // NVIC Register Masks (RM: Page 95 - 98)
  // Clears pending interrupts on FMT0 module
  NVICICPR1 = (1<<(62 % 32));
  // Enables interrupts on FTM0 module
  NVICISER1 = (1<<(62 % 32));

  // FTM successfully initialised
  return bTRUE;
}

BOOL FTM_Set(const TFTMChannel* const aFTMChannel)
{
  if (aFTMChannel->timerFunction == TIMER_FUNCTION_OUTPUT_COMPARE)
  {
    if (aFTMChannel->ioType.outputAction == TIMER_OUTPUT_DISCONNECT)
    {
      // Disable write protection
      if (FTM0_FMS | FTM_FMS_WPEN_MASK)
      {
        FTM0_MODE |= FTM_MODE_WPDIS_MASK;
      }

      // Output compare without setting or clearing a pin
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_MSB_MASK;
      FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_MSA_MASK;
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSB_MASK;
      FTM0_CnSC(aFTMChannel->channelNb) &= ~FTM_CnSC_ELSA_MASK;

      // Enable write protection
      FTM0_FMS |= FTM_FMS_WPEN_MASK;

      return bTRUE;
    }
  }
  return bFALSE;
}

BOOL FTM_StartTimer(const TFTMChannel* const aFTMChannel)
{
  uint16_t val;

  // Check if channel is set to output compare mode
  if (aFTMChannel->timerFunction != TIMER_FUNCTION_OUTPUT_COMPARE)
  {
    return bFALSE;
  }

  // Set time for interrupt to occur
  val = aFTMChannel->delayCount + FTM0_CNT;
  FTM0_CnV(aFTMChannel->channelNb) = FTM_CnV_VAL(val);

  // Clear status flag of channel if set
  if (FTM0_STATUS & (1 << aFTMChannel->channelNb))
  {
    FTM0_STATUS &= ~(1 << aFTMChannel->channelNb);
  }

  // Enable interrupt
  FTM0_CnSC(aFTMChannel->channelNb) |= FTM_CnSC_CHIE_MASK;

  // Store address for callback function and parameters
  Callback[aFTMChannel->channelNb] = aFTMChannel->userFunction;
  Arguments[aFTMChannel->channelNb] = aFTMChannel->userArguments;

  return bTRUE;
}

void __attribute__ ((interrupt)) FTM0_ISR(void)
{
  uint8_t channelNb;

  for(channelNb = 0; channelNb < CHANNEL_COUNT; channelNb++)
  {
    // Check if interrupt is enabled for channel and Check if the flag is set for that channel
    if ((FTM0_CnSC(channelNb) & FTM_CnSC_CHIE_MASK) && (FTM0_CnSC(channelNb) & FTM_CnSC_CHF_MASK))
    {
      // Disable interrupt
      FTM0_CnSC(channelNb) &= ~FTM_CnSC_CHIE_MASK;

      if (Callback[channelNb])
      {
        (*Callback[channelNb])(Arguments[channelNb]);
      }
    }
  }
  return;
}

/*!
 * @}
 */
