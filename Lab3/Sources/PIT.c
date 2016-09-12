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

BOOL PIT_Init(const uint32_t moduleClk, void (*userFunction)(void*), void* userArguments)
{
  //
}

void PIT_Set(const uint32_t period, const BOOL restart)
{
  //
}

void PIT_Enable(const BOOL enable)
{
  //
}

void __attribute__ ((interrupt)) PIT_ISR(void)
{
  //
}

/*!
 * @}
*/
