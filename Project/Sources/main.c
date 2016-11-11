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
#include "OS.h"
#include "Cpu.h"
#include "Events.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "AWG.h"
#include "SPI.h"
#include "PIT.h"
#include "FIFO.h"
#include "LEDs.h"
#include "UART.h"
#include "types.h"
#include "packet.h"
#include "analog.h"
#include "waveform.h"

#define NB_AWG_CHANNELS 2
#define PIT_PERIOD 10000000
#define STARTUP_COMMAND 0x60
#define THREAD_STACK_SIZE 100
#define PROTOCOL_FREQUENCY_OUTPUT 256
#define PROTOCOL_AMPLITUDE_OUTPUT 3276
#define PROTOCOL_OFFSET_OUTPUT 0


// Function Prototypes
static void PITThread(void* arg);
static void InitThread(void* arg);
static void PacketThread(void* arg);
void Channel_Init(const uint16_t sampleFrequency, const uint32_t moduleClk);
BOOL Channel_Control(const TFGControl control, const uint16union_t data);

static uint32_t BaudRate = 115200;		/*!< Baud rate for the tower */
static uint16_t SampleFrequency = 100;		/*!< Sample frequency for the waveform period */
static uint16_t SampleNb;			/*!< Number of samples per period */
static uint16_t SampleNbMax;			/*!< The maximum limit of the sample number */
static uint8_t CurrentChannel;			/*!< The channel currently being used */
TChannel Channel[NB_AWG_CHANNELS];		/*!< Number of digital output channels */
volatile uint16union_t *NvTowerNumber, *NvTowerMode;

// Thread stacks
static uint32_t PITThreadStack[THREAD_STACK_SIZE]    __attribute__ ((aligned(0x08)));	/*!< The stack for the PIT thread */
static uint32_t InitThreadStack[THREAD_STACK_SIZE]   __attribute__ ((aligned(0x08)));	/*!< The stack for the Tower Init thread */
static uint32_t PacketThreadStack[THREAD_STACK_SIZE] __attribute__ ((aligned(0x08)));	/*!< The stack for the Packet thread */
static uint32_t RxThreadStack[THREAD_STACK_SIZE]     __attribute__ ((aligned(0x08)));	/*!< The stack for the UART receive thread */
static uint32_t TxThreadStack[THREAD_STACK_SIZE]     __attribute__ ((aligned(0x08)));	/*!< The stack for the UART transmit thread */

// Global semaphores
extern OS_ECB* PITSemaphore;
static OS_ECB* ChannelOn[NB_AWG_CHANNELS];


/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  LEDs_Init();
  Analog_Init(CPU_BUS_CLK_HZ);

  // Initialize the RTOS
  OS_Init(CPU_CORE_CLK_HZ, true);

  // Create threads using OS_ThreadCreate();
  (void)OS_ThreadCreate(InitThread,
			NULL,
			&InitThreadStack[THREAD_STACK_SIZE - 1],
			0);
  (void)OS_ThreadCreate(RxThread,
			NULL,
			&RxThreadStack[THREAD_STACK_SIZE - 1],
			1);
  (void)OS_ThreadCreate(TxThread,
			NULL,
			&TxThreadStack[THREAD_STACK_SIZE - 1],
			2);
  (void)OS_ThreadCreate(PacketThread,
			NULL,
			&PacketThreadStack[THREAD_STACK_SIZE - 1],
			3);

  // Start multithreading
  OS_Start();
}

/*! @brief Tower initialization thread
 *
 *  @return void.
 */
static void InitThread(void* arg)
{
  OS_DisableInterrupts();

  Channel_Init(SampleFrequency, CPU_BUS_CLK_HZ);
  (void)Packet_Init(BaudRate, CPU_BUS_CLK_HZ);

  OS_EnableInterrupts();

  OS_ThreadDelete(OS_PRIORITY_SELF);
}

/*! @brief Packet thread
 *
 *  @return void.
 */
static void PacketThread(void* arg)
{
  BOOL valid;

  for(;;)
  {
    Packet_Get();
    switch (Packet_Command & ~(PACKET_ACK_MASK))
    {
      // Sends 0x60 as the AWG startup command
      case STARTUP_COMMAND:
        valid = Channel_Control(Packet_Parameter1, (uint16union_t)Packet_Parameter23);
        break;
      default:
	valid = bFALSE;
	break;
    }
    // Packet acknowledgment
    if (PACKET_ACK_MASK & Packet_Command)
    {
      if (!valid)
        Packet_Command &= ~(PACKET_ACK_MASK);
      else
      Packet_Put (Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
    }
  }
}

/*! @brief PIT thread
 *
 *  @return void.
 */
static void PITThread(void* arg)
{
  int16union_t digitalData[2];
  uint32_t PITperiodNS = PIT_PERIOD;
  PIT_Set(PITperiodNS, true);

  for (;;)
  {
    OS_SemaphoreWait(PITSemaphore, 0);
    SampleNb++;
    SampleNb = SampleNb % SampleNbMax;

    if (Channel[0].active)
    {
      // Change data on transmission
      digitalData[0].l = AWG_Output(Channel[0].output, SampleNb);
      // Transmit data to the digital output
      Analog_Put(0, digitalData[0].l);
      // Packet_Put(0x50, 0, data[0].s.Lo, data[0].s.Hi);
    }
    if (Channel[1].active)
    {
      // Change data on transmission and increment the number of samples
      digitalData[0].l = AWG_Output(Channel[1].output, SampleNb);
      // Transmit data to the digital output
      Analog_Put(1, digitalData[0].l);
      // Packet_Put(0x50, 0, data[0].s.Lo, data[0].s.Hi);
    }
  }
}

/*! @brief Initialises the channels to the appropriate outputs.
 *
 *  @param sampleFrequency The frequency samples taken at a period.
 *  @param moduleClock The module clock rate in Hz.
 *  @return void.
 */
void Channel_Init(const uint16_t sampleFrequency, const uint32_t moduleClk)
{
  PIT_Init(moduleClk);
  AWG_Init(sampleFrequency);
  PITSemaphore = OS_SemaphoreCreate(0);

  // Output values set to the channel
  for (uint8_t channelNb = 0; channelNb < NB_AWG_CHANNELS; channelNb++)
  {
    Channel[channelNb].active                 	= bFALSE;
    Channel[channelNb].output.waveformType    	= SINE_WAVE;
    Channel[channelNb].output.frequency.l 	= PROTOCOL_FREQUENCY_OUTPUT;
    Channel[channelNb].output.amplitude.l 	= PROTOCOL_AMPLITUDE_OUTPUT;
    Channel[channelNb].output.offset.l    	= PROTOCOL_OFFSET_OUTPUT;
    ChannelOn[channelNb]                  	= OS_SemaphoreCreate(0);
  }

  SampleNb = 0;
  CurrentChannel = 0;
  SampleNbMax = 10 * sampleFrequency;

  // Create threads
  (void)OS_ThreadCreate(PITThread,
                        (uint16_t *)&sampleFrequency,
                        &PITThreadStack[THREAD_STACK_SIZE - 1],
                        4);
}

/*! @brief Initialises the channels to the appropriate outputs.
 *
 *  @param control
 *  @param data
 *  @return void.
 */
BOOL Channel_Control(const TFGControl control, const uint16union_t data)
{
  BOOL valid;
  TChannel* channel;
  channel = &Channel[CurrentChannel];

  switch (control)
  {
    case STATUS_CHECK:
      valid = (data.l == 0);
      if (!valid)
        break;
      Packet_Put(STARTUP_COMMAND, STATUS_CHECK, CurrentChannel, 0);
      Packet_Put(STARTUP_COMMAND, WAVEFORM_CHANGE, channel->output.waveformType, 0);
      Packet_Put(STARTUP_COMMAND, FREQUENCY_CHANGE, channel->output.frequency.s.Lo, channel->output.frequency.s.Hi);
      Packet_Put(STARTUP_COMMAND, AMPLITUDE_CHANGE, channel->output.amplitude.s.Lo, channel->output.amplitude.s.Hi);
      Packet_Put(STARTUP_COMMAND, OFFSET_CHANGE, channel->output.offset.s.Lo, channel->output.offset.s.Hi);
      break;

    case WAVEFORM_CHANGE:
      valid = ((data.s.Lo <= ARBITRARY_WAVE) || (data.s.Hi == 0));
      if (!valid)
        break;
      channel->output.waveformType = data.s.Lo;
      break;

    case FREQUENCY_CHANGE:
      valid = (data.l <= (100 * 256));
      if (!valid)
        break;
      channel->output.frequency = data;
      break;

    case AMPLITUDE_CHANGE:
      valid = (data.l <= 32767);
      if (!valid)
        break;
      channel->output.amplitude = data;
      break;

    case OFFSET_CHANGE:
      valid = (((int16_t)data.l <=  32767) && ((int16_t)data.l >= -32767));
      if (!valid)
        break;
      channel->output.offset.l = (int16_t)data.l;
      break;

    case CHANNEL_START:
      valid = (data.l == 0) && (!channel->active);
      if (!valid)
        break;
      channel->active = bTRUE;
      break;

    case CHANNEL_STOP:
      valid = (data.l == 0) && (channel->active);
      if (!valid)
        break;
      channel->active = bFALSE;
      break;

    case CHANNEL_CHANGE:
      valid = ((data.s.Hi == 0) && (data.s.Lo < 2));
      if (!valid)
        break;
      CurrentChannel = data.s.Lo;
      break;

    default:
      valid = bFALSE;
  }

  return valid;
}

/*!
** @}
*/
