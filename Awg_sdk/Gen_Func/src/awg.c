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
#include "include/awg.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "blink.pio.h" // Our assembled PIO program

#include <FreeRTOS.h>
#include <task.h>

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
    uint16_t bufdepth;                                                   // Profundidad del buffer
    uint numcycle;                                                       // The number of cycles contained in the buffer, ie. 4 means four full cycles in the buffer
} SignalParameters;

// Estructura para los parámetros de la señal
typedef struct
{
    SignalType type;         // Tipo de señal
    SignalParameters params; // Parametros de la señal
    float frequency;         // Frecuencia en Hz
    float amplitude;         // Amplitud de la señal
    float offset;            // Offset de la señal
    float duty_cycle;        // Duty cycle (solo para señales cuadradas)
    float rise_time;         // Tiempo de subida (solo para señales cuadradas)
    float fall_time;         // Tiempo de bajada (solo para señales cuadradas)
    int polarity;
} SignalGenerator;

SignalGenerator signal_ch1;

typedef struct
{
    uint8_t multiplador; // Multiplicador para frecuencia, amplitud, etc
    uint16_t cont;       // Suma cada vez que el encoder gira a la derecha y resta a la izquierda
    bool avanc;          // Pulsador del encoder para avanzar a la siguiente configuracion
} Selector_t;

Selector_t selector;

// Pio config
uint sm;
uint offset;
PIO pio = pio0;
uint SM_CLK_FREQ = 125000000; // Frecuencia de la maquina de estado.

// Configuraciones de dma
// wave_dma_chan_a and wave_dma_chan_b loads AWG buffer table to PIO in ping pong method
int wave_dma_chan_a;
int wave_dma_chan_b;

dma_channel_config wave_dma_chan_a_config;
dma_channel_config wave_dma_chan_b_config;

/**
 * @brief Inicializa las tareas de freertos.
 */
static void init_task(void);
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
static void init_signal(SignalGenerator *signal, SignalType type, float frequency, float amplitude, float offset, float duty_cycle, float rise_time, float fall_time);
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
 * @brief Configuracion de frecuencia.
 */
static void config_freq(SignalGenerator *sg, float freq);
/**
 * @brief Configuracion de amplitud.
 */
static void config_amplitude(SignalGenerator *sg, float amplitude);
/**
 * @brief Configuracion de offset.
 */
static void config_offset(SignalGenerator *sg, float offset);
/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Tarea de awg de freertos.
 */
static void prvTaskRtos_awg(void *pvParameters);
/**
 * @brief Tarea de interrupcion de encoder.
 */
static void prvTaskRtos_encoder(void *pvParameters);

TaskHandle_t handle_Encoder;

/* EXTERN FUNCTIONS */
/*-------------------------------------------------------------------------------------------*/
extern void awg_config(void)
{
    /* Crea las tareas. */
    init_task();

    /* Configura la señal. */

    /* Configura el pwm. */

    /* Configura los pines del encoder. */

    /* Configura los pulsadores. */

    // Inicializamos una señal sinusoidal con 1000 Hz, amplitud 1.0 y offset 0.0
    // init_signal(&signal_ch1, SIGNAL_SINE, 1000, 127, 128, 0.0, 0.0, 0.0);
    // init_signal_params(&signal_ch1, BUFFER_DEPTH);

    // select_freq(&signal_ch1); // Configura la frecuencia del SM.

    // generate_signal(&signal_ch1);

    // init_config();

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Func(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Freq(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Amp(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Offset(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_enableOutput(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_start(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_blink(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_ledOff(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_resetEnc(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_reset(void)
{
    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_stop(void)
{

    return;
}
extern int awg_reconfig(void)
{

    return 1;
}

/* PRIVATE FUNCTIONS FREERTOS */
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_awg(void *pvParameters)
{

    for (;;)
    {
    }

    vTaskDelete(NULL);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_encoder(void *pvParameters)
{

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);

    return;
}

/* PRIVATE FUNCTIONS NO FREERTOS */
/*-------------------------------------------------------------------------------------------*/
static void init_task(void)
{
    BaseType_t status;

    status = xTaskCreate(prvTaskRtos_awg, "Task awg", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);

    configASSERT(status == NULL); // Detiene el programa si no se crea correctamente

    status = xTaskCreate(prvTaskRtos_encoder, "Int encoder", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, &handle_Encoder);

    configASSERT(status == NULL);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void init_signal(SignalGenerator *signal, SignalType type, float frequency, float amplitude, float offset, float duty_cycle, float rise_time, float fall_time)
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
/*-------------------------------------------------------------------------------------------*/
static void init_signal_params(SignalGenerator *sg, uint16_t bufdepth)
{
    sg->params.bufdepth = bufdepth;

    return;
}
/*-------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------------------------*/
static void init_config(void)
{
    init_pio();            // Configura el pio
    init_dma(&signal_ch1); // Configura los canales del dma.

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void init_pio(void)
{
    // Choose which PIO instance to use (there are two instances)
    sm = pio_claim_unused_sm(pio, true);
    offset = pio_add_program(pio, &pio_byte_out_program);
    pio_byte_out_program_init(pio, sm, offset, OUT_PIN_NUMBER, NPINS, SM_CLK_FREQ);

    return;
}
/*-------------------------------------------------------------------------------------------*/
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
        &pio0_hw->txf[sm],          // Write address (sm1 transmit FIFO)
        signal_ch1.params.buffer,   // Read values from waveform buffer
        signal_ch1.params.bufdepth, // 4096 values to copy
        false                       // Don't start yet.
    );

    // Setup the second wave DMA channel for PIO output
    dma_channel_configure(
        wave_dma_chan_b,
        &wave_dma_chan_b_config,
        &pio0_hw->txf[sm],          // Write address (sm1 transmit FIFO)
        signal_ch1.params.buffer,   // Read values from waveform buffer
        signal_ch1.params.bufdepth, // 4096 values to copy
        false                       //  Don't start yet.
    );

    /* Inicia el dma. */
    dma_start_channel_mask(1u << wave_dma_chan_a);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void generate_signal(SignalGenerator *sg)
{
    switch (sg->type)
    {
    case SIGNAL_SINE:
        // printf("se configuro la senoidal");
        fill_sine_wave(sg);
        break;
    case SIGNAL_SQUARE:
        // fill_square_wave(sg);
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
/*-------------------------------------------------------------------------------------------*/
static void fill_sine_wave(SignalGenerator *sg)
{
    for (int i = 0; i < sg->params.bufdepth; ++i)
    {
        float factor = (float)(sg->params.numcycle * i) / sg->params.bufdepth;                 // Calcular el factor basado en la frecuencia y el índice
        sg->params.buffer[i] = (uint8_t)(sg->offset + (sin(factor * TWO_PI) * sg->amplitude)); // Cálculo de la onda senoidal
    }

    return;
}
/*-------------------------------------------------------------------------------------------*/
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
/*-------------------------------------------------------------------------------------------*/
static void config_freq(SignalGenerator *sg, float freq)
{
    sg->frequency = freq;

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void config_amplitude(SignalGenerator *sg, float amplitude)
{
    sg->amplitude = amplitude;

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void config_offset(SignalGenerator *sg, float offset)
{
    sg->offset = offset;

    return;
}
/*-------------------------------------------------------------------------------------------*/