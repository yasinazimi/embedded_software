/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  This contains the functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author PMcL
 *  @date 2016-11-09
 */
#ifndef SPI_H
#define SPI_H

#include "types.h"
#include "MK70F12.h"

#define BIT_FRAME 15

// new types
#include "types.h"

typedef struct
{
  BOOL isMaster;                   /*!< A BOOLean value indicating whether the SPI is master or slave. */
  BOOL continuousClock;            /*!< A BOOLean value indicating whether the clock is continuous. */
  BOOL inactiveHighClock;          /*!< A BOOLean value indicating whether the clock is inactive low or inactive high. */
  BOOL changedOnLeadingClockEdge;  /*!< A BOOLean value indicating whether the data is clocked on even or odd edges. */
  BOOL LSBFirst;                   /*!< A BOOLean value indicating whether the data is transferred LSB first or MSB first. */
  uint32_t baudRate;               /*!< The baud rate in bits/sec of the SPI clock. */
} TSPIModule;

/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClk The module clock in Hz.
 *  @return BOOL - TRUE if the SPI module was successfully initialized.
 */
BOOL SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock);
 
/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 * @return void.
 */
void SPI_SelectSlaveDevice(const uint8_t slaveAddress);

/*! @brief Transmits a byte and retrieves a received byte from the SPI.
 *
 *  @param dataTx is a byte to transmit.
 *  @param dataRx points to where the received byte will be stored.
 *  @param ctas Selects either CTAR0 or CTAR1
 *  @param continuousPCS Sets the transaction to have a continuously asserted peripheral chip select signal.
 *  @return void.
 */
void SPI_Exchange(const uint16_t dataTx, uint16_t* const dataRx, uint8_t const ctas, BOOL const continuousPCS);

/*! @brief Set baud rate for SPI
 *
 *  @param BaudRate The desired baud rate in bits/sec.
 *  @param moduleClock The module clock in Hz.
 */
static uint32_t SPI_SetBaudRate(const uint32_t BaudRate, const uint32_t moduleClock);

#endif
