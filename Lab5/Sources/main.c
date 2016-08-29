/*!
** @file
** @version 1.0
** @brief  Main module.
**
**   This file contains the high-level code for the project.
**   It initialises appropriate hardware subsystems,
**   creates application threads, and then starts the OS.
**
**   An example of two threads communicating via a semaphore
**   is given that flashes the orange LED. These should be removed
**   when the use of threads and the RTOS is understood.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */


// CPU module - contains low level hardware initialization routines
#include "Cpu.h"

// Simple OS
#include "OS.h"

// Arbitrary thread stack size - big enough for stacking of interrupts and OS use.
#define THREAD_STACK_SIZE 100

static const uint16_t LED_TIME_ON  = 50;  /*!< The LED on time in units of OS clock ticks. */
static const uint16_t LED_TIME_OFF = 50;  /*!< The LED off time in units of OS clock ticks. */

static uint32_t LEDOnThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));  /*!< The stack for the LED On thread. */
static uint32_t LEDOffThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08))); /*!< The stack for the LED Off thread. */

// ----------------------------------------
// Global Semaphores
// ----------------------------------------

static OS_ECB *TurnLEDOnSemaphore;    /*!< Binary semaphore for turning the LED on */
static OS_ECB *TurnLEDOffSemaphore;   /*!< Binary semaphore for turning the LED off */

/*! @brief Waits for a signal to turns the blue LED on, then waits a half a second, then signals for the blue LED to be turned off.
 *
 *  @param pData is not used but is required by the OS to create a thread.
 *  @note Assumes that LEDInit has been called successfully.
 */
static void LEDOnThread(void* pData)
{
  for (;;)
  {
    // Wait here until signaled that we can turn the LED on
    (void)OS_SemaphoreWait(TurnLEDOnSemaphore, 0);

    // Turn LED on
    GPIOA_PCOR = (1 << 10);

    // Wait for the required on time
    OS_TimeDelay(LED_TIME_ON);
    // Signal that the LED can now be turned off
    (void)OS_SemaphoreSignal(TurnLEDOffSemaphore);
  }
}

/*! @brief Waits for a signal to turns the blue LED off, then waits a half a second, then signals for the blue LED to be turned on.
 *
 *  @param pData is not used but is required by the OS to create a thread.
 *  @note Assumes that LEDInit has been called successfully.
 */
static void LEDOffThread(void* pData)
{

  for (;;)
  {
    // Wait here until signaled that we can turn the LED off
    (void)OS_SemaphoreWait(TurnLEDOffSemaphore, 0);

    // Turn LED off
    GPIOA_PSOR = (1 << 10);

    // Wait for the required off time
    OS_TimeDelay(LED_TIME_OFF);
    // Signal that the LED can now be turned on
    (void)OS_SemaphoreSignal(TurnLEDOnSemaphore);
  }
}

/*! @brief Initialises the hardware to support the blue LED.
 *
 */
void LEDInit(void)
{
  // Enable clock gate for Port A to enable pin routing
  SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;

  // Set up the port pin so that initially the LED is off
  GPIOA_PSOR = (1 << 10);

  // Set up the port pin to be an output
  GPIOA_PDDR |= (1 << 10);

  // Set Pin Control Register so we get access to GPIO with high drive strength
  PORTA_PCR10 = PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK;
}


/*! @brief Initialises the hardware, sets up to threads, and starts the OS.
 *
 */
/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  OS_ERROR error;

  // Initialise low-level clocks etc using Processor Expert code
  PE_low_level_init();

  // Initialise the hardware - in this case for the blue LED
  LEDInit();

  // Initialize the RTOS
  OS_Init(CPU_CORE_CLK_HZ, true);

  // Create semaphores for signaling between threads
  TurnLEDOnSemaphore = OS_SemaphoreCreate(1);
  TurnLEDOffSemaphore = OS_SemaphoreCreate(0);

  // Create threads using OS_ThreadCreate();
  error = OS_ThreadCreate(LEDOnThread,
                  NULL,
                  &LEDOnThreadStack[THREAD_STACK_SIZE - 1],
                  0); // Highest priority
  error = OS_ThreadCreate(LEDOffThread,
                  NULL,
                  &LEDOffThreadStack[THREAD_STACK_SIZE - 1],
                  1); // Second-highest priority

  // Start multithreading - never returns!
  OS_Start();
}

/*!
** @}
*/
