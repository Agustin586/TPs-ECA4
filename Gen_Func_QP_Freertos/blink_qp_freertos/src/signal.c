#include "signal.h"

#include "config.h"

#include "hardware/pio.h"
#include "hardware/dma.h"
#include "prog_pio.pio.h" // Our assembled PIO program
#include "math.h"

// DEFINICIONES: PIO
#define OUT_PIN_NUMBER 8
#define NPINS 8
#define PI 3.14159
#define TWO_PI (2.0f * PI) // Definimos 2*PI para facilitar cálculos

// PIO: VARIABLES
uint sm;
uint offset;
PIO pio = pio0;
uint SM_CLK_FREQ = 125000000; // Frecuencia de la maquina de estado.

// DMA: VARIABLES
int wave_dma_chan_a;
int wave_dma_chan_b;
dma_channel_config wave_dma_chan_a_config;
dma_channel_config wave_dma_chan_b_config;

// DECLARACION DE FUNCIONES
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
 * @brief Inicializacion de los parametros de la señal.
 *
 * @param[in,out] sg Señal.
 * @param[in] bufdepth Profundidad del buffer.
 */
static void init_signal_params(SignalGenerator *sg, uint16_t bufdepth);
/**
 * @brief Configuracion de la frecuencia de la maquina de estados.
 *
 * @param[in,out] sg Señal.
 */
static void select_freq(SignalGenerator *sg);
/**
 * @brief Genera la forma de onda en el buffer.
 *
 * @param[in,out] sg Señal.
 */
static void generate_signal(SignalGenerator *sg);
/**
 * @brief Rellena el buffer con una senoidal.
 *
 * @param[in,out] sg Señal.
 */
static void fill_sine_wave(SignalGenerator *sg);
/**
 * @brief Rellena el buffer con una cuadrada.
 *
 * @param[in,out] sg Señal.
 */
static void fill_square_wave(SignalGenerator *sg);
/**
 * @brief Genera la señal triangular.
 */
static void fill_triangle_wave(SignalGenerator *sg);
/**
 * @brief Genera el vector para la función diente de sierra.
 *
 * @param sg
 */
// static void fill_sawtooth_wave(SignalGenerator *sg);

// CUERPO DE FUNCIONES EXTERNAS
//...................................................................................
extern void signal_init(SignalGenerator *sg)
{
    // Signal params
    init_signal_params(sg, BUFFER_DEPTH);
    
    // Config. Perifericos micro
    init_pio();
    init_dma(sg);

    // Config. Valor iniciales de la señal
    sg->type = SIGNAL_INIT_TYPE;
    sg->frequency = SIGNAL_INIT_FREQ;
    sg->amplitude = SIGNAL_INIT_AMP;
    sg->offset = SIGNAL_INIT_OFFSET;
    sg->duty_cycle = SIGNAL_INIT_DUTY;
    sg->polarity = SIGNAL_INIT_POLARITY;

    // PIO
    pio_sm_set_enabled(pio, sm, false);

    return;
}
//...................................................................................
extern void signal_config_freq(SignalGenerator *sg, float freq)
{
    sg->frequency = freq;

    return;
}
//...................................................................................
extern void signal_config_amplitude(SignalGenerator *sg, float amplitude)
{
    sg->amplitude = amplitude;

    return;
}
//...................................................................................
extern void signal_config_offset(SignalGenerator *sg, float offset)
{
    sg->offset = offset;

    return;
}
//...................................................................................
extern void signal_start(SignalGenerator *sg)
{
    sg->state_out = true;

    /* Configuramos la frecuencia en el PIO */
    select_freq(sg);

    /* Generamos el vector de la funcion */
    generate_signal(sg);

    /* Habilitamos la señal al PIO */
    pio_sm_set_enabled(pio, sm, true);

    return;
}
//...................................................................................
extern void signal_stop(SignalGenerator *sg)
{
    sg->state_out = false;

    pio_sm_set_enabled(pio, sm, false);

    /**
     * @brief Nota: Dependiendo donde se pare el PIO puede quedar
     * en un valor de estado alto, por lo que debe asegurarse que
     * se baje hasta cero. Todavía esta correcion no esta hecha.
     */

    return;
}
//...................................................................................
//...................................................................................

// CUERPO DE FUNCIONES INTERNAS
//...................................................................................
static void init_signal_params(SignalGenerator *sg, uint16_t bufdepth)
{
    sg->params.bufdepth = bufdepth;

    return;
}
//...................................................................................
static void init_pio(void)
{
    // Choose which PIO instance to use (there are two instances)
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &pio_byte_out_program);
    pio_byte_out_program_init(pio, sm, offset, OUT_PIN_NUMBER, NPINS, SM_CLK_FREQ);

    return;
}
//...................................................................................
static void init_dma(SignalGenerator *sg)
{
    // wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
    wave_dma_chan_a = dma_claim_unused_channel(true);
    wave_dma_chan_b = dma_claim_unused_channel(true);

    /* Configura el canal a del DMA. */
    wave_dma_chan_a_config = dma_channel_get_default_config(wave_dma_chan_a);

    channel_config_set_chain_to(&wave_dma_chan_a_config, wave_dma_chan_b); // Activa el dma b, luego de finalizar el a.
    channel_config_set_dreq(&wave_dma_chan_a_config, DREQ_PIO0_TX0);       // Solicitud de transferencia.
    channel_config_set_ring(&wave_dma_chan_a_config, false, 12);           // Deshabilita el buffer circular y configura el tamaño en 12 bits.

    /* Configura le canal b dle DMA. */
    wave_dma_chan_b_config = dma_channel_get_default_config(wave_dma_chan_b);

    channel_config_set_chain_to(&wave_dma_chan_b_config, wave_dma_chan_a); // Activa el dma a, luego de finalizar el b.
    channel_config_set_dreq(&wave_dma_chan_b_config, DREQ_PIO0_TX0);
    channel_config_set_ring(&wave_dma_chan_b_config, false, 12);

    // Setup the first wave DMA channel for PIO output
    dma_channel_configure(
        wave_dma_chan_a,
        &wave_dma_chan_a_config,
        &pio0_hw->txf[sm],   // Write address (sm1 transmit FIFO)
        sg->params.buffer,   // Read values from waveform buffer
        sg->params.bufdepth, // 4096 values to copy
        false                // Don't start yet.
    );

    // Setup the second wave DMA channel for PIO output
    dma_channel_configure(
        wave_dma_chan_b,
        &wave_dma_chan_b_config,
        &pio0_hw->txf[sm],   // Write address (sm1 transmit FIFO)
        sg->params.buffer,   // Read values from waveform buffer
        sg->params.bufdepth, // 4096 values to copy
        false                //  Don't start yet.
    );

    /* Inicia el dma. */
    dma_start_channel_mask(1u << wave_dma_chan_a);

    return;
}
//...................................................................................
static void generate_signal(SignalGenerator *sg)
{
    switch (sg->type)
    {
    case SIGNAL_SINE:
        fill_sine_wave(sg);
        break;
    case SIGNAL_SQUARE:
        fill_square_wave(sg);
        break;
    case SIGNAL_TRIANGLE:
        fill_triangle_wave(sg);
        break;
    case SIGNAL_SAWTOOTH:
        // fill_sawtooth_wave(sg);
        break;
    default:
        break;
    }

    return;
}
//...................................................................................
static void fill_sine_wave(SignalGenerator *sg)
{
    for (int i = 0; i < sg->params.bufdepth; ++i)
    {
        float factor = (float)(sg->params.numcycle * i) / sg->params.bufdepth; // Calcular el factor basado en la frecuencia y el índice
        sg->params.buffer[i] = (uint8_t)(127 + (sin(factor * TWO_PI) * 127));  // Cálculo de la onda senoidal
    }

    return;
}
//...................................................................................
static void fill_square_wave(SignalGenerator *sg)
{
    int cyclelength = sg->params.bufdepth / sg->params.numcycle;

    for (int cyclecount = 0; cyclecount < sg->params.numcycle; ++cyclecount)
    {
        for (int cyclestep = 0; cyclestep < cyclelength; ++cyclestep)
        {
            float factor = (float)cyclestep / cyclelength;
            if (sg->polarity == -1)
            {
                sg->params.buffer[(cyclecount * cyclelength) + cyclestep] = (factor < sg->duty_cycle) ? 255 : 0;
            }
            else
            {
                sg->params.buffer[(cyclecount * cyclelength) + cyclestep] = (factor < sg->duty_cycle) ? 0 : 255;
            }
        }
    }

    return;
}
//...................................................................................
static void fill_triangle_wave(SignalGenerator *sg)
{
    int cyclelength = sg->params.bufdepth / sg->params.numcycle;
    for (int cyclecount = 0; cyclecount < sg->params.numcycle; ++cyclecount)
    { // steps through all the cycles in the buffer
        for (int cyclestep = 0; cyclestep < cyclelength; ++cyclestep)
        { // steps through one of the several cycles
            float factor = (float)cyclestep / cyclelength;
            if (sg->polarity == -1)
            {
                if (factor < sg->duty_cycle)
                    sg->params.buffer[(cyclecount * sg->params.bufdepth / sg->params.numcycle) + cyclestep] = 255 - ((255 / sg->duty_cycle) * factor);
                else
                    sg->params.buffer[(cyclecount * sg->params.bufdepth / sg->params.numcycle) + cyclestep] = 255 - ((255 / sg->duty_cycle) * (1 - factor));
            }
            else
            {
                if (factor < sg->duty_cycle)
                    sg->params.buffer[(cyclecount * sg->params.bufdepth / sg->params.numcycle) + cyclestep] = (255 / sg->duty_cycle) * factor;
                else
                    sg->params.buffer[(cyclecount * sg->params.bufdepth / sg->params.numcycle) + cyclestep] = (255 / sg->duty_cycle) * (1 - factor);
            }
        }
    }
    return;
}
//...................................................................................
// static void fill_sawtooth_wave(SignalGenerator *sg)
// {

//     return;
// }
//...................................................................................
static void select_freq(SignalGenerator *sg)
{
    if (sg->frequency > 1800000)
    {
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
//...................................................................................
