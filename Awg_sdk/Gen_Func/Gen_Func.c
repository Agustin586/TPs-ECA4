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
#include "blink.pio.h" // Our assembled PIO program

#define OUT_PIN_NUMBER 8
#define BUFFER_DEPTH 4096
#define NPINS 8
#define PI 3.14159
#define TWO_PI (2.0f * PI) // Definimos 2*PI para facilitar cálculos

// Definimos los tipos de señal
typedef enum
{
	SIGNAL_SINE,
	SIGNAL_SQUARE,
	SIGNAL_TRIANGLE,
	SIGNAL_SAWTOOTH
} SignalType;

// Definimos los parametros de la señal
typedef struct
{
	uint8_t buffer[BUFFER_DEPTH] __attribute__((aligned(BUFFER_DEPTH))); // Buffer para almacenar los valores de la señal
	uint16_t bufdepth;													 // Profundidad del buffer
	uint numcycle;														 // The number of cycles contained in the buffer, ie. 4 means four full cycles in the buffer
} SignalParameters;

// Estructura para los parámetros de la señal
typedef struct
{
	SignalType type;		 // Tipo de señal
	SignalParameters params; // Parametros de la señal
	float frequency;		 // Frecuencia en Hz
	float amplitude;		 // Amplitud de la señal
	float offset;			 // Offset de la señal
	float duty_cycle;		 // Duty cycle (solo para señales cuadradas)
	float rise_time;		 // Tiempo de subida (solo para señales cuadradas)
	float fall_time;		 // Tiempo de bajada (solo para señales cuadradas)
	int polarity;
} SignalGenerator;

SignalGenerator signal_ch1;

// Pio config
uint sm;
uint offset;
PIO pio = pio0;
uint SM_CLK_FREQ = 125000000; // Frecuencia de la maquina de estado.

// wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
int wave_dma_chan_a;
int wave_dma_chan_b;

dma_channel_config wave_dma_chan_a_config;
dma_channel_config wave_dma_chan_b_config;

/**
 * @brief Inicializacion de las configuraciones.
 */
static void init_config(void);
/**
 * @brief Inicializacion del PIO.
 */
static void init_pio(void);
/**
 * @brief Inicializacion de los canales del DMA.
 *
 * @param[in,out] sg Señal.
 */
static void init_dma(SignalGenerator *sg);
/**
 * @brief Configuracion de la frecuencia de la maquina de estados.
 *
 * @param[in,out] sg Señal.
 */
static void select_freq(SignalGenerator *sg);
/**
 * @brief Inicializacion del objeto signal.
 *
 * @param[in,out] signal Puntero al objeto.
 * @param[in] type Tipo de señal.
 * @param[in] frecuency Frecuencia de la señal.
 * @param[in] amplitude Amplitud.
 * @param[in] offset Offset de la señal.
 * @param[in] duty_cycle Ciclo de trabajo para la cuadrada.
 * @param[in] rise_time Tiempo de subida para la cuadrada.
 * @param[in] fall_time Tiempo de bajada.
 */
void init_signal(SignalGenerator *signal, SignalType type, float frequency, float amplitude, float offset, float duty_cycle, float rise_time, float fall_time);
/**
 * @brief Inicializacion de los parametros de la señal.
 *
 * @param[in,out] sg Señal.
 * @param[in] bufdepth Profundidad del buffer.
 */
static void init_signal_params(SignalGenerator *sg, uint16_t bufdepth);
/**
 * @brief Genera la forma de onda en el buffer.
 *
 * @param[in,out] sg Señal.
 */
void generate_signal(SignalGenerator *sg);
/**
 * @brief Rellena el buffer con una senoidal.
 *
 * @param[in,out] sg Señal.
 */
void fill_sine_wave(SignalGenerator *sg);
/**
 * @brief Rellena el buffer con una cuadrada.
 *
 * @param[in,out] sg Señal.
 */
void fill_square_wave(SignalGenerator *sg);

int main()
{
	stdio_init_all();

	// Inicializamos una señal sinusoidal con 1000 Hz, amplitud 1.0 y offset 0.0
	init_signal(&signal_ch1, SIGNAL_SQUARE, 610900, 127, 128, 0.0, 0.0, 0.0);
	init_signal_params(&signal_ch1, BUFFER_DEPTH);

	select_freq(&signal_ch1); // Configura la frecuencia del SM.

	generate_signal(&signal_ch1);

	init_config();

	sleep_ms(2000);

	uint8_t i = 1;

	while (1)
	{
		// printf("Mensaje por el puerto serie\n");
		// i = i + 1;
		// if(i == 129)
		// 	i = 1;

		// init_signal(&signal_ch1, SIGNAL_SINE, 500000, i, 128, 0.0, 0.0, 0.0);
		// init_signal_params(&signal_ch1, BUFFER_DEPTH);
		// select_freq(&signal_ch1); // Configura la frecuencia del SM.
		// generate_signal(&signal_ch1);

		// Esperar un tiempo antes de continuar
		sleep_ms(50);
	}

	return 0;
}

void init_signal(SignalGenerator *signal, SignalType type, float frequency, float amplitude, float offset, float duty_cycle, float rise_time, float fall_time)
{
	signal->type = type;
	signal->frequency = frequency;
	signal->amplitude = amplitude;
	signal->offset = offset;
	signal->duty_cycle = duty_cycle;
	signal->rise_time = rise_time;
	signal->fall_time = fall_time;

	return;
}

static void init_signal_params(SignalGenerator *sg, uint16_t bufdepth)
{
	sg->params.bufdepth = bufdepth;

	return;
}

static void select_freq(SignalGenerator *sg)
{
	if (sg->frequency > 1800000)
	{
		// Con un aumento en la frecuencia cada vez meto menos puntos en cada ciclo.
		// ** Determine number of cycles in wavetable and the State Machine Clock Frequency
		// numfloat = (float)freq * bufdepth / SM_CLK_FREQ; // floating point result to handle big numbers
		// numcycle = numfloat;							 // Truncation process to get an interger value
		sg->params.numcycle = (uint)(sg->frequency * sg->params.bufdepth / SM_CLK_FREQ);
	}
	else
	{
		if (sg->frequency > 50000)
		{
			// Meto 64 puntos --> 4096 / 64
			sg->params.numcycle = 64; // 64 ciclos en un buffer
			SM_CLK_FREQ = sg->frequency * sg->params.bufdepth / sg->params.numcycle;
		}
		else
		{
			// Meto 256 puntos --> 4096 / 16
			sg->params.numcycle = 16; // 16 ciclos en un buffer
			SM_CLK_FREQ = sg->frequency * sg->params.bufdepth / sg->params.numcycle;
		}
	}

	// Calcular el nuevo divisor
	float div = (float)clock_get_hz(clk_sys) / (float)SM_CLK_FREQ;

	// Aplicar el divisor a la máquina de estado
	pio_sm_set_clkdiv(pio0, sm, div);

	return;
}

static void init_config(void)
{
	init_pio();			   // Configura el pio
	init_dma(&signal_ch1); // Configura los canales del dma.

	return;
}

static void init_pio(void)
{
	// Choose which PIO instance to use (there are two instances)
	sm = pio_claim_unused_sm(pio, true);
	offset = pio_add_program(pio, &pio_byte_out_program);
	pio_byte_out_program_init(pio, sm, offset, OUT_PIN_NUMBER, NPINS, SM_CLK_FREQ);

	return;
}

static void init_dma(SignalGenerator *sg)
{
	// wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
	wave_dma_chan_a = dma_claim_unused_channel(true);
	wave_dma_chan_b = dma_claim_unused_channel(true);

	/* Configura el canal a del DMA. */
	wave_dma_chan_a_config = dma_channel_get_default_config(wave_dma_chan_a);

	channel_config_set_chain_to(&wave_dma_chan_a_config, wave_dma_chan_b); // Activa el dma b, luego de finalizar el a.
	channel_config_set_dreq(&wave_dma_chan_a_config, DREQ_PIO0_TX0);	   // Solicitud de transferencia.
	channel_config_set_ring(&wave_dma_chan_a_config, false, 12);		   // Deshabilita el buffer circular y configura el tamaño en 12 bits.

	/* Configura le canal b dle DMA. */
	wave_dma_chan_b_config = dma_channel_get_default_config(wave_dma_chan_b);

	channel_config_set_chain_to(&wave_dma_chan_b_config, wave_dma_chan_a); // Activa el dma a, luego de finalizar el b.
	channel_config_set_dreq(&wave_dma_chan_b_config, DREQ_PIO0_TX0);
	channel_config_set_ring(&wave_dma_chan_b_config, false, 12);

	// Setup the first wave DMA channel for PIO output
	dma_channel_configure(
		wave_dma_chan_a,
		&wave_dma_chan_a_config,
		&pio0_hw->txf[sm],			// Write address (sm1 transmit FIFO)
		signal_ch1.params.buffer,	// Read values from waveform buffer
		signal_ch1.params.bufdepth, // 4096 values to copy
		false						// Don't start yet.
	);

	// Setup the second wave DMA channel for PIO output
	dma_channel_configure(
		wave_dma_chan_b,
		&wave_dma_chan_b_config,
		&pio0_hw->txf[sm],			// Write address (sm1 transmit FIFO)
		signal_ch1.params.buffer,	// Read values from waveform buffer
		signal_ch1.params.bufdepth, // 4096 values to copy
		false						//  Don't start yet.
	);

	/* Inicia el dma. */
	dma_start_channel_mask(1u << wave_dma_chan_a);

	return;
}

void generate_signal(SignalGenerator *sg)
{
	switch (sg->type)
	{
	case SIGNAL_SINE:
		printf("se configuro la senoidal");
		fill_sine_wave(sg);
		break;
	case SIGNAL_SQUARE:
		fill_square_wave(sg);
		break;
	case SIGNAL_TRIANGLE:
		// fill_triangle_wave(sg);
		break;
	case SIGNAL_SAWTOOTH:
		// fill_sawtooth_wave(sg);
		break;
	default:
		break;
	}

	return;
}

void fill_sine_wave(SignalGenerator *sg)
{
	for (int i = 0; i < sg->params.bufdepth; ++i)
	{
		float factor = (float)(sg->params.numcycle * i) / sg->params.bufdepth;				   // Calcular el factor basado en la frecuencia y el índice
		sg->params.buffer[i] = (uint8_t)(sg->offset + (sin(factor * TWO_PI) * sg->amplitude)); // Cálculo de la onda senoidal
	}

	return;
}

void fill_square_wave(SignalGenerator *sg)
{
	// int rise_samples = (int)(sg->rise_time / (1.0 / sg->frequency) * sg->params.bufdepth);
	// int fall_samples = (int)(sg->fall_time / (1.0 / sg->frequency) * sg->params.bufdepth);
	// int high_samples = (int)(sg->params.bufdepth * sg->duty_cycle);
	// int low_samples = sg->params.bufdepth - high_samples;

	// for (int i = 0; i < sg->params.bufdepth; ++i)
	// {
	// 	if (i < rise_samples)
	// 	{
	// 		// Durante el tiempo de subida
	// 		sg->params.buffer[i] = (uint16_t)(sg->offset + (sg->amplitude * i / rise_samples));
	// 	}
	// 	else if (i < rise_samples + high_samples)
	// 	{
	// 		// Nivel alto
	// 		sg->params.buffer[i] = (uint16_t)(sg->offset + sg->amplitude);
	// 	}
	// 	else if (i < rise_samples + high_samples + fall_samples)
	// 	{
	// 		// Durante el tiempo de bajada
	// 		sg->params.buffer[i] = (uint16_t)(sg->offset + sg->amplitude * (1.0 - (i - rise_samples - high_samples) / (float)fall_samples));
	// 	}
	// 	else
	// 	{
	// 		// Nivel bajo
	// 		sg->params.buffer[i] = (uint16_t)(sg->offset);
	// 	}
	// }
	int cyclelength = sg->params.bufdepth / sg->params.numcycle;

	for (int cyclecount = 0; cyclecount < sg->params.numcycle; ++cyclecount)
	{
		for (int cyclestep = 0; cyclestep < cyclelength; ++cyclestep)
		{
			float factor = (float)cyclestep / cyclelength;
			if (sg->polarity == -1)
			{
				sg->params.buffer[(cyclecount * cyclelength) + cyclestep] = (factor < duty) ? 255 : 0;
			}
			else
			{
				sg->params.buffer[(cyclecount * cyclelength) + cyclestep] = (factor < sg->duty_cycle) ? 0 : 255;
			}
		}
	}

	return;
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