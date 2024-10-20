#include <Arduino.h>

#include <stdio.h>
#include <math.h>
#include "stdlib.h"
#include "rp2040/hardware_regs/include/hardware/regs/dma.h"
#include "hardware/pio.h"
#include "Awg.pio.h" // Our assembled PIO program
#define PI 3.14159

void setup()
{
  // ** Initialize variables
  //stdio_init_all();        // set up to print out
  uint OUT_PIN_NUMBER = 8; // Start of output pin group
  uint NPINS = 8;          // Number of output pins
  uint bufdepth = 4096;    // the number of samples in the AWG buffer table, must be a power of 2 (i.ie, 2,4,8,16,32,64...)
  float factor;
  uint cyclecount;
  uint cyclestep;
  uint cyclelength;
  uint numcycle;  // The number of cycles contained in the buffer, ie. 4 means four full cycles in the buffer
  float numfloat; // this is a floating point variable to hold the results of the numcycle calc before truncation
  uint SM_CLK_FREQ = 125000000;
  uint wavefreq;
  char str[10];
  // define the waveform buffer to hold the waveform
  uint8_t awg_buff[4096] __attribute__((aligned(4096)));

  // ** Select wave parameters
  char type = 'S'; //'S' for sine, 'T' for triangle, 'P' for square (pulse) wave
  uint freq = 4000000;
  float duty = .5;  // 0 to 1
  int polarity = 1; // -1 for inverted signal from pico, will invert again from amplifier

  // ** Select between high speed and variable speed SM_CLK_FREQ
  if (freq > 1800000)
  {
    // ** Determine number of cycles in wavetable and the State Machine Clock Frequency
    numfloat = (float)freq * bufdepth / SM_CLK_FREQ; // floating point result to handle big numbers
    numcycle = numfloat;                             // Truncation process to get an interger value
  }
  else
  {
    if (freq > 50000)
    {
      numcycle = 64;
      SM_CLK_FREQ = (float)freq * bufdepth / numcycle;
    }
    else
    {
      numcycle = 16;
      SM_CLK_FREQ = (float)freq * bufdepth / numcycle;
    }
  }

  for (int i = 0; i < bufdepth; ++i)
  {
    factor = (float)numcycle * i / bufdepth; // convert integer division to floating point
    // put the AWG formula here:
    awg_buff[i] = 128 + (sin((factor) * 2 * PI) * 127); // Loads the AWG Buffer table with values of the sine wave
  }

  // ** Initialize PIO

  // Choose which PIO instance to use (there are two instances)
  PIO pio = pio0;
  uint sm = pio_claim_unused_sm(pio, true);
  uint offset = pio_add_program(pio, &pio_byte_out_program);
  pio_byte_out_program_init(pio, sm, offset, OUT_PIN_NUMBER, NPINS, SM_CLK_FREQ);

  // ** Initialize DMA
  // wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
  int wave_dma_chan_a = dma_claim_unused_channel(true);
  int wave_dma_chan_b = dma_claim_unused_channel(true);

  // set up the wave_dma_chan_a DMA channel
  dma_channel_config wave_dma_chan_a_config = dma_channel_get_default_config(wave_dma_chan_a);
  channel_config_set_chain_to(&wave_dma_chan_a_config, wave_dma_chan_b); // after block has been transferred, wave_dma_chan b
  channel_config_set_dreq(&wave_dma_chan_a_config, DREQ_PIO0_TX0);       // Transfer when PIO asks for a new value
  channel_config_set_ring(&wave_dma_chan_a_config, false, 12);           // wrap every 4096 bytes

  // Setup the wave_dma_chan_b DMA channel
  dma_channel_config wave_dma_chan_b_config = dma_channel_get_default_config(wave_dma_chan_b);
  channel_config_set_chain_to(&wave_dma_chan_b_config, wave_dma_chan_a); // after block has been transferred, wave_dma_chan a
  channel_config_set_dreq(&wave_dma_chan_b_config, DREQ_PIO0_TX0);       // Transfer when PIO asks for a new value
  channel_config_set_ring(&wave_dma_chan_b_config, false, 12);           // wrap every 4096 bytes (2**8)

  // Setup the first wave DMA channel for PIO output
  dma_channel_configure(
      wave_dma_chan_a,
      &wave_dma_chan_a_config,
      &pio0_hw->txf[sm], // Write address (sm1 transmit FIFO)
      awg_buff,          // Read values from waveform buffer
      bufdepth,          // 4096 values to copy
      false              // Don't start yet.
  );
  // Setup the second wave DMA channel for PIO output
  dma_channel_configure(
      wave_dma_chan_b,
      &wave_dma_chan_b_config,
      &pio0_hw->txf[sm], // Write address (sm1 transmit FIFO)
      awg_buff,          // Read values from waveform buffer
      bufdepth,          // 4096 values to copy
      false              //  Don't start yet.
  );

  // ** Everything is ready to go. Now start the first DMA
  dma_start_channel_mask(1u << wave_dma_chan_a);
}

void loop()
{
  // put your main code here, to run repeatedly:
}
