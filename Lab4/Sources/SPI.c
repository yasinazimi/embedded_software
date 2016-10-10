/*! @file
 *
 *  @brief I/O routines for K70 SPI interface.
 *
 *  Implementation of functions for operating the SPI (serial peripheral interface) module.
 *
 *  @author Mohammad Yasin Azimi, Scott Williams
 *  @date 2016-10-11
 */
/*!
 * @addtogroup SPI_module SPI module documentation
 * @{
 */
#include "SPI.h"
#include "MK70F12.h"

uint32_t SPI_SetBaudRate(const uint32_t BaudRate, const uint32_t moduleClock);

/*! @brief Sets up the SPI before first use.
 *
 *  @param aSPIModule is a structure containing the operating conditions for the module.
 *  @param moduleClock The module clock in Hz.
 *  @return BOOL - true if the SPI module was successfully initialized.
 */
BOOL SPI_Init(const TSPIModule* const aSPIModule, const uint32_t moduleClock)
{
  int pbr, br, pbrInd, brInd;
  uint32_t baudResult;

  // SPI clocked by internal bus clock. Module has internal divider, minimum divide by 2, max freq then bus clk/2
  // Requirement 3.1: K70 uses SPI2 from PTD11-15
  // DSPI? refers to this clock as system clock
  SIM_SCGC3 |= SIM_SCGC3_DSPI2_MASK; 		// Enable module clock for DSPI2  this is not SPI2
  SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; 		// Enable clock gate port D

  PORTD_PCR11 = PORT_PCR_MUX(2);		// Maps SPI2_PCS0 to PTD11
  PORTD_PCR12 = PORT_PCR_MUX(2);		// Maps SPI2_SCK  to PTD12
  PORTD_PCR13 = PORT_PCR_MUX(2);		// Maps SPI2_SOUT to PTD13
  PORTD_PCR14 = PORT_PCR_MUX(2);		// Maps SPI2_SIN  to PTD14
  PORTD_PCR15 = PORT_PCR_MUX(2);		// Maps SPI2_PCS1 to PTD15

  SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;		// Enable clock gate port e
  PORTE_PCR5 = PORT_PCR_MUX(1);  		// PTE5 is a GPIO
  PORTE_PCR27 = PORT_PCR_MUX(1);  		// PTE27 is a GPIO
  GPIOE_PDDR |= (1<<5) | (1<<27);		// Assign PTE5 and PTE27 as outputs via the port data direction register

  // isMaster
  if (aSPIModule->isMaster)
  {
    SPI2_MCR |= SPI_MCR_MSTR_MASK;  		// Enables master Mode
    SPI2_CTAR0 |= SPI_CTAR_PDT(0);         	// Pre-scaler 1 for delay between negation of PCS signal at end frame and assertion PCS at beginnign next frame (master Mode)
    SPI2_CTAR0 |= SPI_CTAR_CSSCK(0);       	// Pre-scaler 2 for PCS to SCK delay (Master mode)
  }
  else
  {
    SPI2_MCR &= ~SPI_MCR_MSTR_MASK;  		// Enables slave Mode
  }

  // continuousClock
  if (aSPIModule->continuousClock)
  {
    SPI2_MCR |= SPI_MCR_CONT_SCKE_MASK; 	// Continuous SCK enabled
  }
  else
  {
    SPI2_MCR &= ~SPI_MCR_CONT_SCKE_MASK; 	// Continuous SCK disabled
  }

  SPI2_MCR &= ~SPI_MCR_DCONF_MASK;       	// SPI Configuration as per hint provided
  SPI2_MCR |= SPI_MCR_FRZ_MASK;          	// Halt serial transfers in debug mode (from hint)
  SPI2_MCR &= ~SPI_MCR_MTFE_MASK;        	// Modified SPI transfer format disabled
  SPI2_MCR &= ~SPI_MCR_PCSSE_MASK;       	// PCS5 / ~PCSS used as peripheral chip select[5] signal
  SPI2_MCR &= ~SPI_MCR_ROOE_MASK;        	// Receive FIFO overflow overwrite enable: incoming data ignored
  SPI2_MCR |= SPI_MCR_PCSIS(1);          	// Inactive state of PCSx is high
  SPI2_MCR &= ~SPI_MCR_DOZE_MASK;         	// Doze mode disables the module
  SPI2_MCR |= SPI_MCR_DIS_RXF_MASK;      	// Disable recieve  data FIFO
  SPI2_MCR |= SPI_MCR_DIS_TXF_MASK;      	// Disable transmit data FIFO

  // Set some CTAR0 fixed fields as per hints
  SPI2_CTAR0 |= SPI_CTAR_DBR_MASK;       	// Baud rate computed normally with 50/50 duty cycle
  SPI2_CTAR0 |= SPI_CTAR_PCSSCK(0);      	// Pre-scaler value for assertion of PCS  and first edge SCK set to 1
  SPI2_CTAR0 |= SPI_CTAR_PASC(0);        	// Pre-scaler 1 for delay between last edge SCK and negation of PCS.

  SPI2_CTAR0 |= SPI_CTAR_ASC(0);         	// After SCK delay, one multiple of protocol clock period including also PASC
  SPI2_CTAR0 |= SPI_CTAR_DT(0);          	// Delay after transfer scaler

  // inactiveHighClock
  if (aSPIModule->inactiveHighClock)
  {
    SPI2_CTAR0 |= SPI_CTAR_CPOL_MASK; 		// Clock polarity inactive state value of SCK is high
  }
  else
  {
    SPI2_CTAR0 &= ~SPI_CTAR_CPOL_MASK; 		// Clock polarity inactive state value of SCK is low
  }

  // changedonLeadingClockEdge
  if (aSPIModule->changedOnLeadingClockEdge)
  {
    SPI2_CTAR0 |= SPI_CTAR_SLAVE_CPHA_MASK; 	// Data captured on leading edge of SCK and changed on following edge
  }
  else
  {
    SPI2_CTAR0 &= ~SPI_CTAR_SLAVE_CPHA_MASK; 	// Data changed on leading edge of SCK and captured on following edge
  }

  // LSBFirst
  if (aSPIModule->LSBFirst)
  {
    SPI2_CTAR0 |= SPI_CTAR_LSBFE_MASK;		// Data is transferred LSB first
  }
  else
  {
    SPI2_CTAR0 &= ~SPI_CTAR_LSBFE_MASK;		// Data is transferred MSB first
  }

  // fSCK=(Fprocessor/PBR) * ((1+DBR)/BR) for frequency serial clock = asPIModule->baudRate
  // 1,041,666.66=(25e6/3)*((1+0)/8)
  // Set CTAR register for SPI2 as per specs

  SPI2_PUSHR |= SPI_PUSHR_CTAS(0); 		// Assumes reset, confirms CTAR0 is used in master mode. By default, CTAR0 is also used in slave mode
  SPI2_CTAR0 |= SPI_CTAR_FMSZ(15); 		// 16 bit frame size is set

  baudResult = SPI_SetBaudRate(aSPIModule->baudRate, moduleClock);
}

/*! @brief Selects the current slave device
 *
 * @param slaveAddress The slave device address.
 */
void SPI_SelectSlaveDevice(const uint8_t slaveAddress)
{
  // Provided slave address is based on status of GPIO9,8,7, with 9 set high
  // Noted GPIO 7 corresponds PTE5, GPIO8 corresponds PTE27
  switch (slaveAddress)                     	// GPIO 7,8,9 are on PCI express Tower System, connected to LTC1859, GPIO9 hard wired high
  {
    case 4:
    {
      GPIOE_PCOR |= (1 << 5) | (1 << 27);  	// GPIO8 (PTE5) Low, GPIO 7 (PTE27) Low
      break;
    }
    case 5:
    {
      GPIOE_PCOR |= (1 << 5);           	// GPIO8 (PTE5) Low
      GPIOE_PSOR |= (1 << 27);              	// GPIO7 (PTE27) High
      break;
    }
    case 6:
    {
      GPIOE_PSOR |= (1 << 5);             	// GPIO8 (PTE5) High
      GPIOE_PCOR |= (1 << 27);             	// GPIO7 (PTE5) Low
      break;
    }
    case 7:					// Selects LTC1859 as slave as per datasheet p17 chip select decoder truth table
    {
      GPIOE_PSOR |= (1 << 5) | (1 << 27);   	// GPIO8 (PTE5) High, GPIO 7 (PTE27) High
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
  while (!(SPI2_SR & SPI_SR_TFFF_MASK))     	// While TFFF flag is 0
  {
    // is below code meant to be inside this loop
    // Wait until TFFF is 1, TxFIFO is not Full, ready to write to
  }

  SPI2_PUSHR |= SPI_PUSHR_PCS(1);              	// Push
  SPI2_PUSHR |= SPI_PUSHR_TXDATA(dataTx);
  SPI2_SR |= SPI_SR_TFFF_MASK;              	// Write 1 to clear the TFFF flag

  while(!(SPI2_SR & SPI_SR_RFDF_MASK))
  {
    // Wait
  }

  if (!(SPI2_SR & SPI_SR_RFDF_MASK))            	// If RxFiFO drain flag
  {
    *dataRx = SPI2_POPR; 			// Load the data pointer from POPR to extract information
    //Attempts to pop data from empty RX FIRO ignored, RX FIFO counter remains unchanged, data read is undetermined
    //RXFIFO not empty, RXFIFO drain flag RFDF in SR set
  }

  SPI2_SR |= SPI_SR_RFDF_MASK;              // Write 1 to clear the RxFIFO drain flag, now RxData has been transferred
}

/*! @brief Sets the baud rate to closest to the desired baud rate for SPI peripheral
 *
 * @param BaudRate is desired baud rate
 * @param moduleClock The module clock in Hz.
 */
uint32_t SPI_SetBaudRate(const uint32_t BaudRate, const uint32_t moduleClock)
{
  uint32_t brScaler[16] = {2, 4, 6, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768};
  uint32_t pbrPrescaler[4] = {2,3,5,7};
  uint32_t brDesired = (uint64_t)BaudRate;  	// 64 bit signed integer used in all calculations
  uint32_t modClk = (uint64_t)moduleClock;  	// 64 bit signed integer used in all calculations
  uint8_t abc;
  uint32_t thisResult;
  uint32_t closestBrArr[4];                     // Array Format: 0: baud rate difference, 1:brIndex, 2:pbrIndex, 3: (0 is < baud 1 is >baud)
  uint8_t brIndexCnt = 0;
  uint8_t pbrIndexCnt = 0;
  closestBrArr[0] = 0xFFFFFFFF;               	// Initialise baud rate difference to the highest possible value

  while (brIndexCnt<16)
  {
    thisResult = modClk / (brScaler[brIndexCnt]*pbrPrescaler[pbrIndexCnt]);
    if (thisResult == brDesired)//we have a solution
    {
      SPI2_CTAR0 |= SPI_CTAR_BR(brIndexCnt); 	// BR bits 3:0 set to brIndexCnt
      SPI2_CTAR0 |= SPI_CTAR_PBR(pbrIndexCnt);  // PBR bits 17-16 set to pbrIndexCnt
      return thisResult;
    }
    else if (thisResult < brDesired)//less than desired baud rate
    {
      if ((brDesired - thisResult) < closestBrArr[0])
      {
	closestBrArr[0] = brDesired-thisResult;	// Store closest error to desired baud rate (slow)
	closestBrArr[1] = brIndexCnt;       	// Store index count to possibly set BR bits
	closestBrArr[2] = pbrIndexCnt;          // Store index count to possibly set PBR bits
	closestBrArr[3] = 0;                    // Indicates thisResult < brDesired
      }
    }
    else // more than desired baud rate
    {
      if ((thisResult - brDesired) < closestBrArr[0] )
      {
	closestBrArr[0] = thisResult - brDesired;   	// Store closest errot to desired baud rate (fast)
	closestBrArr[1] = brIndexCnt;        	// Store index count to possibly set BR bits
	closestBrArr[2] = pbrIndexCnt;          // Store index count to possibly set PBR bits;
	closestBrArr[3] = 1;                    // Indicates thisResult > brDesired
      }
    }
    ++pbrIndexCnt;                        	// Accumulate the pbrIndex

    if(pbrIndexCnt == 4)
    {
      ++brIndexCnt;                             // Accumulate brIndex because all pbrIndexes have been tested for previous brIndex
      pbrIndexCnt = 0;                          // Reset the pbrIndex
    }
  }

  SPI2_CTAR0 |= SPI_CTAR_BR(closestBrArr[1]);   // BR bits 3:0 set to applicable brIndexCnt
  SPI2_CTAR0 |= SPI_CTAR_PBR(closestBrArr[2]);  // PBR bits 17-16 set to applicable pbrIndexCnt

  if (closestBrArr[3])                        	// If selected baud  > desired baud
    return closestBrArr[0]+brDesired;           // Return baud rate (faster than desired)
  return brDesired - closestBrArr[0];           // Return baud rate (slower than desired)
}

/*!
 * @}
 */
