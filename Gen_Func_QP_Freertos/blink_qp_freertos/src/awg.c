#include <stdio.h>
#include <math.h>
#include "awg.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/gpio.h"
#include "prog_pio.pio.h" // Our assembled PIO program

#include "mef_awg.h"

// #include "FreeRTOS.h"
// #include "FreeRTOSConfig.h"
// #include "freertos/include/task.h"
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#define OUT_PIN_NUMBER 8
#define BUFFER_DEPTH 4096
#define NPINS 8
#define PI 3.14159
#define TWO_PI (2.0f * PI) // Definimos 2*PI para facilitar cálculos

// Define los pines de avance y retroceso
#define AVANCE_PIN 7U
#define ATRAS_PIN 6U
// Define los pines del encoder
#define ENCODER_PIN_A 4
#define ENCODER_PIN_B 5
// Define el pin del multiplicador
#define MULTIPLICADOR_PIN 3
// Sentido de giro
#define SENTIDO_HORARIO 1
#define SENTIDO_ANTIHORARIO -1
// Conversion de amplitud
#define AMP_MAX_CUENTAS 127
#define AMP_MIN_CUENTAS 0
#define VOLTAJE_MAX 1.65
#define VOLTAJE_MIN 0
#define AMPLITUD_EN_VOLTAJE(x) x *AMP_MAX_CUENTAS / VOLTAJE_MAX
// Conversion de offset
#define OFFSET_MAX_CUENTAS 127
#define OFFSET_MIN_CUENTAS 0
#define OFFSET_EN_VOLTAJE(X) X *OFFSET_MAX_CUENTAS / VOLTAJE_MAX

// Parametros maximos
#define FREQ_MAX 1500000
#define FREQ_MIN 1
#define AMPLITUD_SALIDA_MAX 12
#define AMPLITUD_SALIDA_MIN 1.67
#define OFFSET_SALIDA_MAX 6
#define OFFSET_SALIDA_MIN 0

// Definimos los multiplicadores de frecuencia
typedef enum
{
    MULT_HZ = 0,
    MULT_KHZ,
    MULT_MHZ,
} MultFreq_t;

// Definimos los multiplicadores de amplitud
typedef enum
{
    V = 0,
    mV,
    uV,
} MultAmp_t;

// Definimos los tipos de señal
typedef enum
{
    SIGNAL_SINE = 0,
    SIGNAL_SQUARE,
    SIGNAL_TRIANGLE,
    SIGNAL_SAWTOOTH,
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
    /**
     * @brief Tipo de señal
     */
    SignalType type;
    /**
     * @brief Parametros de la señal
     */
    SignalParameters params;
    /**
     * @brief Frecuencia de la señal
     */
    float frequency;
    /**
     * @brief Amplitud de la señal
     */
    float amplitude;
    /**
     * @brief Offset de la señal
     */
    float offset;
    float duty_cycle; // Duty cycle (solo para señales cuadradas)
    float rise_time;  // Tiempo de subida (solo para señales cuadradas)
    float fall_time;  // Tiempo de bajada (solo para señales cuadradas)
    int polarity;
    /**
     * @brief Habilitacion de la salida
     */
    bool state_out;
} SignalGenerator;

SignalGenerator signal_ch1;

typedef struct
{
    /**
     * @brief Multiplicador de frecuencia
     */
    MultFreq_t multFreq;
    /**
     * @brief Multiplicador de amplitud
     */
    MultAmp_t multAmpl;
    /**
     * @brief Multiplicador de offset
     */
    MultAmp_t multOffset;
    /**
     * @brief Contador del encoder
     */
    int16_t cont;
    /**
     * @brief Sentido de giro
     *
     *  1: sentido horario
     * -1: sentido antihorario
     */
    int8_t direccion;
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

TimerHandle_t timer_Antirrebote;

// PRIVATE FUNCTIONS DECLARED ==================================================================
/**
 * @brief Inicializa las tareas de freertos.
 */
static void init_task(void);
/**
 * @brief Inicializa los timers por software de freertos.
 */
static void init_timers(void);
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
/**
 * @brief Inicializa los pines que se usan.
 */
static void init_pins(void);
/**
 * @brief Obtiene el estado del pulsador
 *
 * @param[in] pin Pin del pulsador.
 */
static bool getState_Pulsador(uint16_t pin);
/**
 * @brief Funcion de interrupcion de pines.
 */
static void encoder_isrA(uint gpio, uint32_t events);
static void encoder_isrB(uint gpio, uint32_t events);
/*-------------------------------------------------------------------------------------------*/
// FREERTOS FUNCTIONS DECLARED ================================================================
/**
 * @brief Tarea de awg de freertos.
 */
static void prvTaskRtos_awg(void *pvParameters);
/**
 * @brief Tarea de interrupcion de encoder.
 */
static void prvTaskRtos_encoder(void *pvParameters);
/**
 * @brief Tarea de detección de pulsadores.
 */
static void prvTaskRtos_pulsadores(void *pvParameters);
/**
 * @brief Timer por software para antirrebote de encoder.
 */
static void prvTimerRtos_Antirrebote(TimerHandle_t xTimer);

TaskHandle_t handle_Encoder;

// EXTERN FUNCTIONS ===========================================================================
/*-------------------------------------------------------------------------------------------*/
extern void awg_config(void)
{
    /* Crea las tareas. */
    init_task();

    /* Crea los timers. */
    init_timers();

    /* Configura la señal. */
    signal_ch1.type = SIGNAL_SINE;                           // Tipo de señal
    signal_ch1.frequency = 1000;                             // Frecuencia
    signal_ch1.amplitude = AMPLITUD_EN_VOLTAJE(VOLTAJE_MAX); // Amplitud
    signal_ch1.offset = OFFSET_EN_VOLTAJE(VOLTAJE_MAX);      // Offset

    /* Configura el pwm. */

    /* Configura los pines. */
    init_pins();

    /* Configuracion del selector. */
    selector.cont = 0;
    selector.direccion = SENTIDO_HORARIO;
    selector.multAmpl = V;
    selector.multFreq = MULT_HZ;
    selector.multOffset = V;

    // Inicializamos una señal sinusoidal con 1000 Hz, amplitud 1.0 y offset 0.0
    // init_signal(&signal_ch1, SIGNAL_SINE, 1000, 127, 128, 0.0, 0.0, 0.0);
    // init_signal_params(&signal_ch1, BUFFER_DEPTH);

    // select_freq(&signal_ch1); // Configura la frecuencia del SM.

    // generate_signal(&signal_ch1);

    // init_config();

    printf("Check: Configuracion del awg.\n");

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Func(void)
{
    // printf("Estado: tipo de funcion.\n");

    // Deteccion del encoder ...
    // Seleccionamos el tipo de funcion
    if (selector.direccion == SENTIDO_HORARIO)
    {
        if (signal_ch1.type != SIGNAL_SAWTOOTH)
            signal_ch1.type += 1;
    }
    else if (selector.direccion == SENTIDO_ANTIHORARIO)
    {
        if (signal_ch1.type != SIGNAL_SINE)
            signal_ch1.type -= 1;
    }

    switch (signal_ch1.type)
    {
    case SIGNAL_SINE:
        printf("Tipo de señal: senoidal.\n");
        break;
    case SIGNAL_SQUARE:
        printf("Tipo de señal: cuadrada.\n");
        break;
    case SIGNAL_TRIANGLE:
        printf("Tipo de señal: triangular.\n");
        break;
    case SIGNAL_SAWTOOTH:
        printf("Tipo de señal: diente de sierra.\n");
        break;
    default:
        printf("Tipo de señal: desconocida.\n");
        break;
    }

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Freq(void)
{
    // printf("Estado: config freq.\n");
    float freq_new = signal_ch1.frequency;

    /* Deteccion del evento encoder. */
    if (selector.direccion == SENTIDO_HORARIO)
    {
        if (selector.multFreq == MULT_HZ)
            freq_new += 1;
        else if (selector.multFreq == MULT_KHZ)
            freq_new += 1000;
        else if (selector.multFreq == MULT_MHZ)
            freq_new += 1000000;
    }
    else if (selector.direccion == SENTIDO_ANTIHORARIO)
    {
        if (selector.multFreq == MULT_HZ)
            freq_new -= 1;
        else if (selector.multFreq == MULT_KHZ)
            freq_new -= 1000;
        else if (selector.multFreq == MULT_MHZ)
            freq_new -= 1000000;
    }

    // Limites
    if (freq_new > FREQ_MAX)
        freq_new = FREQ_MAX;
    else if (freq_new < FREQ_MIN)
        freq_new = FREQ_MIN;

    // Carga la informacion en la señal
    config_freq(&signal_ch1, freq_new);

    printf("Frecuencia: %.2f.\n", signal_ch1.frequency);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Amp(void)
{
    // printf("Estado: config Amp.\n");

    float amp_new = signal_ch1.amplitude;

    // Detecta el evento de amplitud
    if (selector.direccion == SENTIDO_HORARIO)
    {
        if (selector.multAmpl == V)
            amp_new += 1;
        else if (selector.multAmpl == mV)
            amp_new += 0.1;
    }
    else if (selector.direccion == SENTIDO_ANTIHORARIO)
    {
        if (selector.multAmpl == V)
            amp_new -= 1;
        else if (selector.multAmpl == mV)
            amp_new -= 0.1;
    }

    // Limites
    if (amp_new > AMPLITUD_SALIDA_MAX)
        amp_new = AMPLITUD_SALIDA_MAX;
    else if (amp_new < AMPLITUD_SALIDA_MIN)
        amp_new = AMPLITUD_SALIDA_MIN;

    // Carga la informacion
    config_amplitude(&signal_ch1, amp_new);

    printf("Amplitud: %.2f.\n", amp_new);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Offset(void)
{
    // printf("Estado: config offset.\n");
    float offset_new = signal_ch1.offset;

    // Detecta el evento del offset
    if (selector.direccion == SENTIDO_HORARIO)
    {
        if (selector.multOffset == V)
            offset_new += 1;
        else if (selector.multOffset == mV)
            offset_new += 0.1;
    }
    else if (selector.direccion == SENTIDO_ANTIHORARIO)
    {
        if (selector.multOffset == V)
            offset_new -= 1;
        else if (selector.multOffset == mV)
            offset_new -= 0.1;
    }

    // Limites
    if (offset_new > OFFSET_SALIDA_MAX)
        offset_new = OFFSET_SALIDA_MAX;
    else if (offset_new < OFFSET_SALIDA_MIN)
        offset_new = OFFSET_SALIDA_MIN;

    // Carga la informacion
    config_offset(&signal_ch1, offset_new);

    printf("Offset: %.2f.\n", offset_new);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_enableOutput(void)
{
    // printf("Estado: confirmacion config.\n");

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_start(void)
{
    printf("Estado: salida.\n");

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
    printf("Check: reset encoder.\n");

    selector.cont = 0;

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_reset(void)
{
    setEvt_init();

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_stop(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern int awg_reconfig(void)
{
    return 0;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_multiplicador(uint8_t tipo)
{
    switch (tipo)
    {
    case MULTIPLICADOR_FREQ:
        selector.multFreq += 1;
        if (selector.multFreq > MULT_MHZ)
            selector.multFreq = MULT_HZ;
        break;
    case MULTIPLICADOR_AMP:
        selector.multAmpl += 1;
        if (selector.multAmpl > mV)
            selector.multAmpl = V;
        break;
    case MULTIPLICADOR_OFFSET:
        selector.multOffset += 1;
        if (selector.multOffset > mV)
            selector.multOffset = V;
        break;
    default:
        break;
    }

    return;
}

// PRIVATE FUNCTIONS FREERTOS =================================================================
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_awg(void *pvParameters)
{

    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_encoder(void *pvParameters)
{
    uint32_t ulNotificationValue;

    for (;;)
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (ulNotificationValue == 1)
        {
            int channel_a_state = gpio_get(ENCODER_PIN_A);
            int channel_b_state = gpio_get(ENCODER_PIN_B);

            printf("A: %d, B: %d\n", channel_a_state, channel_b_state);

            // Determina la dirección del giro según los estados de A y B
            // Flanco descendente de A
            if (channel_b_state)
            {
                if (selector.cont < 65535)
                    selector.cont++;
                // encoder_position++;
                // encoder_direction = 1; // Sentido horario
            }
            else if (!channel_b_state)
            {
                if (selector.cont > 0)
                    selector.cont--;
                // encoder_position--;
                // encoder_direction = -1; // Sentido antihorario
            }

            printf("Valor Enconder: %d\n", selector.cont);

            if (timer_Antirrebote != NULL)
            {
                xTimerStart(timer_Antirrebote, 0); // Inicia el temporizador

                setEvt_Econder();
            }
        }
    }

    vTaskDelete(NULL);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_pulsadores(void *pvParameters)
{
    for (;;)
    {
        if (!getState_Pulsador(AVANCE_PIN))
        {
            setEvt_Avanc();

            while (!getState_Pulsador(AVANCE_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
        else if (!getState_Pulsador(ATRAS_PIN))
        {
            setEvt_Atras();

            while (!getState_Pulsador(ATRAS_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }
        else if (!getState_Pulsador(MULTIPLICADOR_PIN))
        {
            setEvt_Multiplicador();

            while (!getState_Pulsador(MULTIPLICADOR_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(5));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}

// PRIVATE FUNCTIONS WAVE =====================================================================
/*-------------------------------------------------------------------------------------------*/
#define TASKRTOS_AWG_PRIORITY tskIDLE_PRIORITY + 1
#define TASKRTOS_PULSADORES_PRIORITY tskIDLE_PRIORITY + 1

static void init_task(void)
{
    BaseType_t status;

    status = xTaskCreate(prvTaskRtos_awg, "Task awg", configMINIMAL_STACK_SIZE, NULL, TASKRTOS_AWG_PRIORITY, NULL);

    configASSERT(status == NULL); // Detiene el programa si no se crea correctamente

    status = xTaskCreate(prvTaskRtos_encoder, "Int encoder", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, &handle_Encoder);

    configASSERT(status == NULL);

    status = xTaskCreate(prvTaskRtos_pulsadores, "Task pulsadores", configMINIMAL_STACK_SIZE, NULL, TASKRTOS_PULSADORES_PRIORITY, NULL);

    configASSERT(status == NULL);

    printf("Check: Tareas creadas correctamente.\n");

    return;
}
/*-------------------------------------------------------------------------------------------*/
static void init_timers(void)
{
    timer_Antirrebote = xTimerCreate(
        "Timer Antirrebote",
        pdMS_TO_TICKS(30),
        pdFALSE,
        (void *)0,
        prvTimerRtos_Antirrebote);

    configASSERT(timer_Antirrebote == NULL);

    printf("Check: Timers creados correctamente.\n");

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

// PRIVATE FUNCTIONS PINS =====================================================================
/*-------------------------------------------------------------------------------------------*/
static void init_pins(void)
{
    // Configuracion de pines de avance y retroceso
    gpio_init(AVANCE_PIN);
    gpio_set_dir(AVANCE_PIN, GPIO_IN);

    gpio_init(ATRAS_PIN);
    gpio_set_dir(ATRAS_PIN, GPIO_IN);

    gpio_init(MULTIPLICADOR_PIN);
    gpio_set_dir(MULTIPLICADOR_PIN, GPIO_IN);

    // gpio_put(LED_PIN, 1);

    // Confguracion de pines del encoder
    // Configura los pines A y B del encoder
    gpio_init(ENCODER_PIN_A);
    gpio_set_dir(ENCODER_PIN_A, GPIO_IN);
    gpio_pull_up(ENCODER_PIN_A);

    gpio_init(ENCODER_PIN_B);
    gpio_set_dir(ENCODER_PIN_B, GPIO_IN);
    gpio_pull_up(ENCODER_PIN_B);

    // Configura las interrupciones en ambos pines
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, true, &encoder_isrA);
    // gpio_set_irq_enabled_with_callback(ENCODER_PIN_B, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &encoder_isrB);

    return;
}
/*-------------------------------------------------------------------------------------------*/
static bool getState_Pulsador(uint16_t pin)
{
    return gpio_get(pin);
}
/*-------------------------------------------------------------------------------------------*/
static void prvTimerRtos_Antirrebote(TimerHandle_t xTimer)
{
    /* Vuelve a habilitar la interrupcion. */
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, true, &encoder_isrA);

    return;
}
/*-------------------------------------------------------------------------------------------*/

// PRIVATE FUNCTIONS ISR ======================================================================
/*-------------------------------------------------------------------------------------------*/
// Manejador de interrupciones para el encoder
static void encoder_isrA(uint gpio, uint32_t events)
{
    // Notifica a la tarea de encoder
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, false, &encoder_isrA);

    vTaskNotifyGiveFromISR(handle_Encoder, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*-------------------------------------------------------------------------------------------*/
static void encoder_isrB(uint gpio, uint32_t events)
{
    // Notifica a la tarea de encoder
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // vTaskNotifyGiveFromISR(handle_Encoder, &xHigherPriorityTaskWoken);
    // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
/*-------------------------------------------------------------------------------------------*/
