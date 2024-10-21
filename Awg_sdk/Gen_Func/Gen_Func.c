// #include <stdio.h>
// #include "pico/stdlib.h"
// #include "hardware/spi.h"
// #include "hardware/i2c.h"
// #include "hardware/dma.h"
// #include "hardware/pio.h"
// #include "hardware/interp.h"
// #include "hardware/timer.h"
// #include "hardware/watchdog.h"
// #include "hardware/clocks.h"
// #include "hardware/uart.h"

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
// #include "blink.pio.h" // Our assembled PIO program

#include "include/awg.h"

int main()
{
	stdio_init_all();

	/* Configura el awg. */
	awg_config();

	/* Configura el display. */

	/* Configura los perifericos. */

	while (1)
	{
	}

	return 0;
}

void vApplicationMallocFailedHook(void)
{
	printf("Failed to assigned memory\n\r");

	// Aquí puedes manejar el error, por ejemplo, entrar en un bucle infinito o registrar el error.
	for (;;)
		; // Bucle infinito
}

void vApplicationTickHook(void)
{
	// Aquí puedes manejar el tick, como hacer algo cada vez que se incrementa el tick.
	// Por ejemplo, puedes controlar algún contador, o realizar tareas de mantenimiento.
}

// int main()
// {
// 	// ** Initialize variables
// 	stdio_init_all();		 // set up to print out
// 	uint OUT_PIN_NUMBER = 8; // Start of output pin group
// 	uint NPINS = 8;			 // Number of output pins
// 	uint bufdepth = 4096;	 // the number of samples in the AWG buffer table, must be a power of 2 (i.ie, 2,4,8,16,32,64...)
// 	float factor;
// 	uint cyclecount;
// 	uint cyclestep;
// 	uint cyclelength;
// 	uint numcycle;	// The number of cycles contained in the buffer, ie. 4 means four full cycles in the buffer
// 	float numfloat; // this is a floating point variable to hold the results of the numcycle calc before truncation
// 	uint SM_CLK_FREQ = 125000000;
// 	uint wavefreq;
// 	char str[10];
// 	// define the waveform buffer to hold the waveform
// 	uint8_t awg_buff[4096] __attribute__((aligned(4096)));

// 	// ** Select wave parameters
// 	char type = 'P'; //'S' for sine, 'T' for triangle, 'P' for square (pulse) wave
// 	uint freq = 1000;
// 	float duty = .5;  // 0 to 1
// 	int polarity = 1; // -1 for inverted signal from pico, will invert again from amplifier

// 	// ** Select between high speed and variable speed SM_CLK_FREQ
// 	if (freq > 1800000)
// 	{
// 		// ** Determine number of cycles in wavetable and the State Machine Clock Frequency
// 		numfloat = (float)freq * bufdepth / SM_CLK_FREQ; // floating point result to handle big numbers
// 		numcycle = numfloat;							 // Truncation process to get an interger value
// 	}
// 	else
// 	{
// 		if (freq > 50000)
// 		{
// 			numcycle = 64;
// 			SM_CLK_FREQ = (float)freq * bufdepth / numcycle;
// 		}
// 		else
// 		{
// 			numcycle = 16;
// 			SM_CLK_FREQ = (float)freq * bufdepth / numcycle;
// 		}
// 	}

// 	// ** Fill wavetable with data
// 	switch (type)
// 	{
// 		// for sine wave
// 	case 'S':
// 		for (int i = 0; i < bufdepth; ++i)
// 		{
// 			factor = (float)numcycle * i / bufdepth; // convert integer division to floating point
// 			// put the AWG formula here:
// 			awg_buff[i] = 128 + (sin((factor) * 2 * PI) * 127); // Loads the AWG Buffer table with values of the sine wave
// 		}
// 		break;
// 		// For triangle and sawtooth wave
// 	case 'T':
// 		cyclelength = bufdepth / numcycle;
// 		for (cyclecount = 0; cyclecount < numcycle; ++cyclecount)
// 		{ // steps through all the cycles in the buffer
// 			for (cyclestep = 0; cyclestep < cyclelength; ++cyclestep)
// 			{ // steps through one of the several cycles
// 				factor = (float)cyclestep / cyclelength;
// 				if (polarity == -1)
// 				{
// 					if (factor < duty)
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 255 - ((255 / duty) * factor);
// 					else
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 255 - ((255 / duty) * (1 - factor));
// 				}
// 				else
// 				{
// 					if (factor < duty)
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = (255 / duty) * factor;
// 					else
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = (255 / duty) * (1 - factor);
// 				}
// 			}
// 		}
// 		break;
// 		// For Square wave
// 	case 'P':
// 		cyclelength = bufdepth / numcycle;
// 		for (cyclecount = 0; cyclecount < numcycle; ++cyclecount)
// 		{ // steps through all the cycles in the buffer
// 			for (cyclestep = 0; cyclestep < cyclelength; ++cyclestep)
// 			{ // steps through one of the several cycles
// 				factor = (float)cyclestep / cyclelength;
// 				if (polarity == -1)
// 				{
// 					if (factor < duty)
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 255;
// 					else
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 0;
// 				}
// 				else
// 				{
// 					if (factor < duty)
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 0;
// 					else
// 						awg_buff[(cyclecount * bufdepth / numcycle) + cyclestep] = 255;
// 				}
// 			}
// 		}
// 		break;
// 	}

// 	// ** Initialize PIO

// 	// Choose which PIO instance to use (there are two instances)
// 	PIO pio = pio0;
// 	uint sm = pio_claim_unused_sm(pio, true);
// 	uint offset = pio_add_program(pio, &pio_byte_out_program);
// 	pio_byte_out_program_init(pio, sm, offset, OUT_PIN_NUMBER, NPINS, SM_CLK_FREQ);

// 	// ** Initialize DMA
// 	// wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
// 	int wave_dma_chan_a = dma_claim_unused_channel(true);
// 	int wave_dma_chan_b = dma_claim_unused_channel(true);

// 	// set up the wave_dma_chan_a DMA channel
// 	dma_channel_config wave_dma_chan_a_config = dma_channel_get_default_config(wave_dma_chan_a);
// 	channel_config_set_chain_to(&wave_dma_chan_a_config, wave_dma_chan_b); // after block has been transferred, wave_dma_chan b
// 	channel_config_set_dreq(&wave_dma_chan_a_config, DREQ_PIO0_TX0);	   // Transfer when PIO asks for a new value
// 	channel_config_set_ring(&wave_dma_chan_a_config, false, 12);		   // wrap every 4096 bytes

// 	// Setup the wave_dma_chan_b DMA channel
// 	dma_channel_config wave_dma_chan_b_config = dma_channel_get_default_config(wave_dma_chan_b);
// 	channel_config_set_chain_to(&wave_dma_chan_b_config, wave_dma_chan_a); // after block has been transferred, wave_dma_chan a
// 	channel_config_set_dreq(&wave_dma_chan_b_config, DREQ_PIO0_TX0);	   // Transfer when PIO asks for a new value
// 	channel_config_set_ring(&wave_dma_chan_b_config, false, 12);		   // wrap every 4096 bytes (2**8)

// 	// Setup the first wave DMA channel for PIO output
// 	dma_channel_configure(
// 		wave_dma_chan_a,
// 		&wave_dma_chan_a_config,
// 		&pio0_hw->txf[sm], // Write address (sm1 transmit FIFO)
// 		awg_buff,		   // Read values from waveform buffer
// 		bufdepth,		   // 4096 values to copy
// 		false			   // Don't start yet.
// 	);
// 	// Setup the second wave DMA channel for PIO output
// 	dma_channel_configure(
// 		wave_dma_chan_b,
// 		&wave_dma_chan_b_config,
// 		&pio0_hw->txf[sm], // Write address (sm1 transmit FIFO)
// 		awg_buff,		   // Read values from waveform buffer
// 		bufdepth,		   // 4096 values to copy
// 		false			   //  Don't start yet.
// 	);

// 	// ** Everything is ready to go. Now start the first DMA
// 	dma_start_channel_mask(1u << wave_dma_chan_a);

// 	// ** Communications for debugging and future enhancements
// 	sleep_ms(40000);
// 	while (true)
// 	{
// 		for (int k = 0; k < bufdepth; ++k)
// 		{
// 			//		printf("Enter the desired wave frequency: ");
// 			//		scanf("%s",&str);
// 			//		wavefreq = atoi(str);
// 			//		printf("You entered:  %i Counter: %i \n",wavefreq, k);
// 			//		wavefreq = 0;
// 			printf("awg_buff[%i]: %i sm: %i CLK: %i numcycle: %i freq: %i \n", k, awg_buff[k], sm, SM_CLK_FREQ, numcycle, freq);
// 			//		printf(" wave_dma_chan_a: %i wave_dma_chan_b: %i \n", wave_dma_chan_a, wave_dma_chan_b);
// 			// printf("ch#: %i &dma_hw->ch[wave_dma_chan_b].al2_write_addr_trig: %i \n", dma_hw->ch[wave_dma_chan_b].al2_write_addr_trig, &dma_hw->ch[wave_dma_chan_b].al2_write_addr_trig);
// 			sleep_ms(200);
// 		}
// 	}
// }