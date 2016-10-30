/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  Implementation of functions for operating the LEDs.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup LED_module LEDs module documentation
 * @{
*/
#include "OS.h"
#include "LEDs.h"
#include "MK70F12.h"

/*! @brief Sets up the LEDs before first use.
 *
 *  @return BOOL - TRUE if the LEDs were successfully initialized.
 */
BOOL LEDs_Init(void)
{
  // Sets Port A bit control registers for PCR (Peripheral Control Register)
  // Initialises blue LED
  PORTA_PCR10 |= PORT_PCR_MUX(1);
  // Initialises orange LED
  PORTA_PCR11 |= PORT_PCR_MUX(1);
  // Initialises yellow LED
  PORTA_PCR28 |= PORT_PCR_MUX(1);
  // Initialises green LED
  PORTA_PCR29 |= PORT_PCR_MUX(1);

  // Sets LEDs as outputs
  // Sets orange LED as output
  GPIOA_PDDR |= LED_ORANGE;
  // Sets yellow LED as output
  GPIOA_PDDR |= LED_YELLOW;
  // Sets green LED as output
  GPIOA_PDDR |= LED_GREEN;
  // Sets blue LED as output
  GPIOA_PDDR |= LED_BLUE;

  // Turns Port A ON
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  // Turns off all LEDs for (Port Set Output Registers)
  // Turns off orange LED
  GPIOA_PSOR |= LED_ORANGE;
  // Turns off yellow LED
  GPIOA_PSOR |= LED_YELLOW;
  // Turns off green LED
  GPIOA_PSOR |= LED_GREEN;
  // Turns off blue LED
  GPIOA_PSOR |= LED_BLUE;

  return bTRUE;
}

/*! @brief Turns an LED on.
 *
 *  @param color The color of the LED to turn on.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_On(const TLED color)
{
  // Turns LEDs ON
  GPIOA_PCOR |= color;
}

/*! @brief Turns off an LED.
 *
 *  @param color THe color of the LED to turn off.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Off(const TLED color)
{
  // Turns LEDs OFF
  GPIOA_PSOR |= color;
}

/*! @brief Toggles an LED.
 *
 *  @param color THe color of the LED to toggle.
 *  @note Assumes that LEDs_Init has been called.
 */
void LEDs_Toggle(const TLED color)
{
  // Changes LED colors
  GPIOA_PTOR |= color;
}

/*!
 * @}
*/
