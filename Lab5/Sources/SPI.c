/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  Implementation of functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-31
 */
/*!
 * @addtogroup SPI_module SPI module documentation
 * @{
 */
#include "SPI.h"
#include "MK70F12.h"

// Variable for transmit data
static uint32union_t SPITxData;

/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClk The module clock in Hz.
 *  @return BOOL - true if the SPI module was successfully initialized.
 */
BOOL SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  // Enable module clock for DSPI2  this is not SPI2
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK;
  // Enable clock gate port D
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;
  // Enable clock gate to PORTE
  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;

  // Maps SPI2_PCS0 to PTD11
  PORTD_PCR11 = PORT_PCR_MUX(2);
  // Maps SPI2_SCK  to PTD12
  PORTD_PCR12 = PORT_PCR_MUX(2);
  // Maps SPI2_SOUT to PTD13
  PORTD_PCR13 = PORT_PCR_MUX(2);
  // Maps SPI2_SIN  to PTD14
  PORTD_PCR14 = PORT_PCR_MUX(2);
  // Maps SPI2_PCS1 to PTD15
  PORTD_PCR15 = PORT_PCR_MUX(2);

  // PTE5 is a GPIO
  PORTE_PCR5 = PORT_PCR_MUX(1);
  // PTE27 is a GPIO
  PORTE_PCR27 = PORT_PCR_MUX(1);
  // Initially clear output
  GPIOE_PCOR  = ((1 << 5) | (1 << 27));
  // Sets pins as outputs
  GPIOE_PDDR |= ((1 << 5) | (1 << 27));

  SPI_SetBaudRate(moduleClock, aSPIModule->baudRate);

  //Enable module clocks
  SPI2_MCR &= ~SPI_MCR_MDIS_MASK;
  // Doze mode disables the module
  SPI2_MCR &= ~SPI_MCR_DOZE_MASK;
  // PCS5 / ~PCSS used as peripheral chip select[5] signal
  SPI2_MCR &= ~SPI_MCR_PCSSE_MASK;
  // Receive FIFO overflow overwrite enable: incoming data ignored
  SPI2_MCR &= ~SPI_MCR_ROOE_MASK;

  // Halt serial transfers in debug mode
  SPI2_MCR |= SPI_MCR_FRZ_MASK;
  // Set Chip select for ADC to inactive high
  SPI2_MCR |= SPI_MCR_PCSIS(1);
  // Disable transmit FIFO
  SPI2_MCR |= SPI_MCR_DIS_TXF_MASK;
  // Disable receive FIFO
  SPI2_MCR |= SPI_MCR_DIS_RXF_MASK;

  // Set some CTAR0 fixed fields as per hints
  // Baud rate computed normally with 50/50 duty cycle
  SPI2_CTAR0 |= SPI_CTAR_DBR_MASK;
  // Pre-scaler value for assertion of PCS  and first edge SCK set to 1
  SPI2_CTAR0 |= SPI_CTAR_PCSSCK(0);
  // Pre-scaler 1 for delay between last edge SCK and negation of PCS.
  SPI2_CTAR0 |= SPI_CTAR_PASC(0);

  // Set some CTAR0 fixed fields as per hints
  // Baud rate computed normally with 50/50 duty cycle
  SPI2_CTAR0 |= SPI_CTAR_DBR_MASK;
  // Pre-scaler value for assertion of PCS  and first edge SCK set to 1
  SPI2_CTAR0 |= SPI_CTAR_PCSSCK(0);
  // Pre-scaler 2 for PCS to SCK delay (Master mode)
  SPI2_CTAR0 |= SPI_CTAR_CSSCK(0);

  // Set 16 bit frame size
  SPI2_CTAR0 |= SPI_CTAR_FMSZ(BIT_FRAME);
  //SPI2_CTAR0 |= SPI_CTAR_ASC(0);          // After SCK delay, one multiple of protocol clock period including also PASC
  //SPI2_CTAR0 |= SPI_CTAR_DT(0);           // Delay after transfer scaler

  // isMaster
  if (aSPIModule->isMaster)
    // Enables master Mode
    SPI2_MCR |= SPI_MCR_MSTR_MASK;
  else
    // Enables slave Mode
    SPI2_MCR &= ~SPI_MCR_MSTR_MASK;

  // continuousClock
  if (aSPIModule->continuousClock)
    // Continuous SCK enabled
    SPI2_MCR |= SPI_MCR_CONT_SCKE_MASK;
  else
    // Continuous SCK disabled
    SPI2_MCR &= ~SPI_MCR_CONT_SCKE_MASK;

  // inactiveHighClock
  if (aSPIModule->inactiveHighClock)
    // Clock polarity inactive state value of SCK is high
    SPI2_CTAR0 |= SPI_CTAR_CPOL_MASK;
  else
    // Clock polarity inactive state value of SCK is low
    SPI2_CTAR0 &= ~SPI_CTAR_CPOL_MASK;


  // changedonLeadingClockEdge
  if (aSPIModule->changedOnLeadingClockEdge)
    // Data captured on leading edge of SCK and changed on following edge: Classic transfer format pg1848
    SPI2_CTAR0 |= SPI_CTAR_CPHA_MASK;
  else
    // Data changed on leading edge of SCK and captured on following edge
    SPI2_CTAR0 &= ~SPI_CTAR_CPHA_MASK;

  // LSBFirst
  if (aSPIModule->LSBFirst)
    // Data is transferred LSB first
    SPI2_CTAR0 |= SPI_CTAR_LSBFE_MASK;
  else
    // Data is transferred MSB first
    SPI2_CTAR0 &= ~SPI_CTAR_LSBFE_MASK;

  // Delay to allow capacitors to charge before transfer
  // Pre-scaler 2 for PCS to SCK delay (Master mode)
  SPI2_CTAR0 |= SPI_CTAR_CSSCK(3);
  // Pre-scaler value for assertion of PCS  and first edge SCK set to 1
  SPI2_CTAR0 |= SPI_CTAR_PCSSCK(3);

  SPITxData.s.Hi = 1;

  return bTRUE;
}

/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 */
void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  // Provided slave address is based on status of GPIO9,8,7, with 9 set high
  // Noted GPIO 7 corresponds PTE5, GPIO8 corresponds PTE27
  // GPIO 7,8,9 are on PCI express Tower System, connected to LTC1859, GPIO9 hard wired high
  switch (slaveAddress)
  {
    case 4:
    {
      // GPIO8 (PTE5) Low, GPIO 7 (PTE27) Low
      GPIOE_PCOR |= (1 << 5) | (1 << 27);
      break;
    }
    case 5:
    {
      // GPIO8 (PTE5) Low
      GPIOE_PCOR |= (1 << 5);
      // GPIO7 (PTE27) High
      GPIOE_PSOR |= (1 << 27);
      break;
    }
    case 6:
    {
      // GPIO8 (PTE5) High
      GPIOE_PSOR |= (1 << 5);
      // GPIO7 (PTE5) Low
      GPIOE_PCOR |= (1 << 27);
      break;
    }
    case 7:
    {
      // Selects LTC1859 as slave as per datasheet p17 chip select decoder truth table
      // GPIO8 (PTE5) High, GPIO 7 (PTE27) High
      GPIOE_PSOR |= (1 << 5) | (1 << 27);
      break;
    }
  }
  //SPI2_PUSHR |= SPI_PUSHR_PCS(slaveAddress);
}

/*! @brief Transmits a byte and retrieves a received byte from the SPI.
 *
 *  @param dataTx is a byte to transmit.
 *  @param dataRx points to where the received byte will be stored.
 */
void SPI_ExchangeChar(const uint16_t dataTx, uint16_t* const dataRx)
{

  // Transmit data and commands are written to the PUSHR and received data is read from the POPR
  SPITxData.s.Lo = dataTx;

  // While TFFF flag is 0
  while (!(SPI2_SR & SPI_SR_TFFF_MASK))
  {
    // is below code meant to be inside this loop
    // Wait until TFFF is 1, TxFIFO is not Full, ready to write to
  }

  // Push data and commands
  SPI2_PUSHR = SPITxData.l;
  // Write 1 to clear the TFFF flag
  SPI2_SR |= SPI_SR_TFFF_MASK;
  // Start transfer
  SPI2_MCR &= ~SPI_MCR_HALT_MASK;


  // While RDRF flag is 0
  while (!(SPI2_SR & SPI_SR_RFDF_MASK))
  {
    // wait
  }

  // Stop transfer
  SPI2_MCR |= SPI_MCR_HALT_MASK;

  if(dataRx)
    *dataRx = (uint16_t)SPI2_POPR*2;

  // Write 1 to clear Receive FIFO drain flag
  SPI2_SR |= SPI_SR_RFDF_MASK;
}

/*! @brief Set baud rate for SPI
 *
 *  @param BaudRate The desired baud rate in bits/sec.
 *  @param moduleClock The module clock in Hz.
 */
static uint32_t SPI_SetBaudRate(const uint32_t BaudRate, const uint32_t moduleClock)
{
  uint32_t brScaler[16] = {2, 4, 6, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
  uint32_t pbrPrescaler[4] = {2,3,5,7};
  // 64 bit signed integer used in all calculations
  uint32_t brDesired = (uint64_t)BaudRate;
  // 64 bit signed integer used in all calculations
  uint32_t modClk = (uint64_t)moduleClock;
  uint8_t abc;
  uint32_t thisResult;
  // Array Format: 0: baud rate difference, 1:brIndex, 2:pbrIndex, 3: (0 is < baud 1 is >baud)
  uint32_t closestBrArr[4];
  uint8_t brIndexCnt = 0;
  uint8_t pbrIndexCnt = 0;
  // Initialise baud rate difference to the highest possible value
  closestBrArr[0] = 0xFFFFFFFF;

  while (brIndexCnt<16)
  {
    thisResult = modClk / (brScaler[brIndexCnt]*pbrPrescaler[pbrIndexCnt]);
    // solution
    if (thisResult == brDesired)
    {
      // BR bits 3:0 set to brIndexCnt
      SPI2_CTAR0 |= SPI_CTAR_BR(brIndexCnt);
      // PBR bits 17-16 set to pbrIndexCnt
      SPI2_CTAR0 |= SPI_CTAR_PBR(pbrIndexCnt);
      return thisResult;
    }
    //less than desired baud rate
    else if (thisResult < brDesired)
    {
      if ((brDesired - thisResult) < closestBrArr[0])
      {
        // Store closest error to desired baud rate (slow)
        closestBrArr[0] = brDesired-thisResult;
        // Store index count to possibly set BR bits
        closestBrArr[1] = brIndexCnt;
        // Store index count to possibly set PBR bits
        closestBrArr[2] = pbrIndexCnt;
        // Indicates thisResult < brDesired
        closestBrArr[3] = 0;
      }
    }
    // more than desired baud rate
    else
    {
      if ((thisResult - brDesired) < closestBrArr[0] )
      {
        // Store closest errot to desired baud rate (fast)
        closestBrArr[0] = thisResult - brDesired;
        // Store index count to possibly set BR bits
        closestBrArr[1] = brIndexCnt;
        // Store index count to possibly set PBR bits;
        closestBrArr[2] = pbrIndexCnt;
        // Indicates thisResult > brDesired
        closestBrArr[3] = 1;
      }
    }
    // Accumulate the pbrIndex
    ++pbrIndexCnt;

    if(pbrIndexCnt == 4)
    {
      // Accumulate brIndex because all pbrIndexes have been tested for previous brIndex
      ++brIndexCnt;
      // Reset the pbrIndex
      pbrIndexCnt = 0;
    }
  }

  // BR bits 3:0 set to applicable brIndexCnt
  SPI2_CTAR0 |= SPI_CTAR_BR(closestBrArr[1]);
  // PBR bits 17-16 set to applicable pbrIndexCnt
  SPI2_CTAR0 |= SPI_CTAR_PBR(closestBrArr[2]);

  // If selected baud  > desired baud
  if (closestBrArr[3])
    // Return baud rate (faster than desired)
    return closestBrArr[0]+brDesired;

  // Return baud rate (slower than desired)
  return brDesired - closestBrArr[0];
}

/*!
 * @}
 */
