/*! @file
 *
 *  @brief Routines to access the LEDs on the TWR-K70F120M.
 *
 *  This contains the functions for operating the LEDs.
 *
 *  @author Mohammad Yasin Azimi, Michael Codner
 *  @date 2016-08-24
 */

#include "LEDs.h"
#include "MK70F12.h"

BOOL LEDs_Init(void)
{
  // Setting Port A bit control registers for PCR (Peripheral Control Register)
  PORTA_PCR10 |= PORT_PCR_MUX(1); // Initialises blue LED
  PORTA_PCR11 |= PORT_PCR_MUX(1); // Initialises orange LED
  PORTA_PCR28 |= PORT_PCR_MUX(1); // Initialises yellow LED
  PORTA_PCR29 |= PORT_PCR_MUX(1); // Initialises green LED

  //Set LEDs as outputs
  GPIOA_PDDR |= LED_ORANGE; // Sets orange LED as output
  GPIOA_PDDR |= LED_YELLOW; // Sets yellow LED as output
  GPIOA_PDDR |= LED_GREEN;    // Sets green LED as output
  GPIOA_PDDR |= LED_BLUE;   // Sets blue LED as output

  // Turn Port A ON
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  //Turn off all LEDs for (Port Set Output Registers)
  GPIOA_PSOR |= LED_ORANGE; // Turn off orange LED
  GPIOA_PSOR |= LED_YELLOW; // Turn off yellow LED
  GPIOA_PSOR |= LED_GREEN;    // Turn off green LED
  GPIOA_PSOR |= LED_BLUE;   // Turn off blue LED

  return bTRUE;
}

// Turns LEDs ON
void LEDs_On(const TLED color)
{
  GPIOA_PCOR |= color;
}

// Turns LEDs OFF
void LEDs_Off(const TLED color)
{
  GPIOA_PSOR |= color;
}

// Changes LED colors
void LEDs_Toggle(const TLED color)
{
  GPIOA_PTOR |= color;
}

/*!
 * @}
*/
