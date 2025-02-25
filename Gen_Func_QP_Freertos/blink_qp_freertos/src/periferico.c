#include "periferico.h"

#include "hardware/gpio.h"
#include "mef_awg.h"
#include "awg.h"
#include "config.h"
#include "pico/stdlib.h"
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <queue.h>

// Pines de botones
#define AVANCE_PIN 7U
#define ATRAS_PIN 6U
#define MULTIPLICADOR_PIN 3
// Pines de encoder
#define ENCODER_PIN_A 4
#define ENCODER_PIN_B 5
// Pin buzzer
#define BUZZER_PIN 22

// Queue
#define QUEUE_LENGTH 10

// FREERTOS: VARIABLES
TimerHandle_t timer_Antirrebote;
TaskHandle_t handle_Encoder;

// PINS: VARIABLES
volatile bool channel_a_state;
volatile bool channel_b_state;
volatile uint8_t pin;

static QueueHandle_t perifericoQueue;

periferico_t DatosPeriferico;

/**
 * @brief Inicializa los pines de perifericos
 *
 */
static void init_pins(void);
/**
 * @brief Get the State Pulsador object
 *
 * @param pin
 * @return true
 * @return false
 */
static bool getState_Pulsador(uint16_t pin);
/**
 * @brief Inicializa parametros del encoder
 *
 */
static void init_encoder(void);
/**
 * @brief Inicializa las tareas del archivo
 *
 */
static void init_task(void);
/**
 * @brief Inicializa los timers
 *
 */
static void init_timers(void);
/**
 * @brief Inicializa la cola de datos
 *
 */
static void perifericoQueue_init(void);
/**
 * @brief Envia un dato
 *
 * @param dato objeto tipo periferico
 */
static bool perifericoQueue_send(periferico_t *dato);
/**
 * @brief Funcion de interrupcion de pines.
 */
static void encoder_isrA(uint gpio, uint32_t events);
/**
 * @brief ISR del pin B.
 */
// static void encoder_isrB(uint gpio, uint32_t events);
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
/**
 * @brief Activa el buzzer y espera un tiempo para desactivarlo
 *
 */
static void Buzzer(void);

// FUNCIONES EXTERNAS
//..........................................................................................
extern void EtapaPerf_init(void)
{
    /* Inicializa pines */
    init_pins();
    init_encoder();

    /* Inicializa tareas de freertos */
    init_task();

    /* Inicializa timers frertos */
    init_timers();

    /* Inicializa la cola de datos */
    perifericoQueue_init();

    return;
}
//..........................................................................................
extern bool periferico_QueueReceive(periferico_t *dato)
{
    return xQueueReceive(perifericoQueue, dato, pdMS_TO_TICKS(50)) == pdPASS;
}
//..........................................................................................
extern void periferico_ResetMultiplicadores(periferico_t *Perf_obj)
{
    Perf_obj->encoder.multAmpl = MULT_AMP_X_1;
    Perf_obj->encoder.multFreq = MULT_FREQ_X_1;
    Perf_obj->encoder.multOffset = MULT_OFFSET_X_1;

    perifericoQueue_send(Perf_obj);

    return;
}
//..........................................................................................

// FUNCIONES INTERNAS
//..........................................................................................
static void prvTaskRtos_encoder(void *pvParameters)
{
    uint32_t ulNotificationValue;

    for (;;)
    {
        ulNotificationValue = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(100));

        if (ulNotificationValue == 1)
        {
            // Determina la dirección del giro según los estados de A y B
            // Flanco descendente de A
            Buzzer();

            if (!channel_b_state && !channel_a_state)
            {
                if (DatosPeriferico.encoder.cont < 65535)
                    DatosPeriferico.encoder.cont++;

                DatosPeriferico.encoder.direccion = SENTIDO_HORARIO;
                DatosPeriferico.evt_encoder = true;

                if (perifericoQueue_send(&DatosPeriferico))
                    DatosPeriferico.evt_encoder = false;
            }
            else if (channel_b_state && !channel_a_state)
            {
                if (DatosPeriferico.encoder.cont > 0)
                    DatosPeriferico.encoder.cont--;

                DatosPeriferico.encoder.direccion = SENTIDO_ANTIHORARIO;
                DatosPeriferico.evt_encoder = true;

                if (perifericoQueue_send(&DatosPeriferico))
                    DatosPeriferico.evt_encoder = false;
            }

            if (timer_Antirrebote != NULL)
            {
                xTimerStart(timer_Antirrebote, 0); // Inicia el temporizador
            }
        }
    }

    vTaskDelete(NULL);

    return;
}
//..........................................................................................
static void prvTaskRtos_pulsadores(void *pvParameters)
{
    for (;;)
    {
        if (!getState_Pulsador(AVANCE_PIN))
        {
            DatosPeriferico.evt_avance = true;

            if (perifericoQueue_send(&DatosPeriferico))
                DatosPeriferico.evt_avance = false;

            Buzzer();

            while (!getState_Pulsador(AVANCE_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        else if (!getState_Pulsador(ATRAS_PIN))
        {
            DatosPeriferico.evt_atras = true;

            if (perifericoQueue_send(&DatosPeriferico))
                DatosPeriferico.evt_atras = false;

            Buzzer();

            while (!getState_Pulsador(ATRAS_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        else if (!getState_Pulsador(MULTIPLICADOR_PIN))
        {
            if (awg_getCurrentState() != CONFIG_CONFIRM && getCurrentState() != SALIDA_EN)
                DatosPeriferico.evt_mult = true;
            else if (awg_getCurrentState() == CONFIG_CONFIRM)
                DatosPeriferico.evt_confr = true;
            else if (awg_getCurrentState() == SALIDA_EN)
                DatosPeriferico.evt_stop = true;

            // Envia a la Queue
            if (perifericoQueue_send(&DatosPeriferico))
            {
                DatosPeriferico.evt_mult = false;
                DatosPeriferico.evt_confr = false;
                DatosPeriferico.evt_stop = false;
            }

            Buzzer();

            while (!getState_Pulsador(MULTIPLICADOR_PIN))
            {
                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}
//..........................................................................................
static void encoder_isrA(uint gpio, uint32_t events)
{
    // Notifica a la tarea de encoder
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    gpio_set_irq_enabled(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, false);

    channel_a_state = gpio_get(ENCODER_PIN_A);
    channel_b_state = gpio_get(ENCODER_PIN_B);
    // pin = ENCODER_PIN_A;

    vTaskNotifyGiveFromISR(handle_Encoder, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
//..........................................................................................
// static void encoder_isrB(uint gpio, uint32_t events)
// {
//     // Notifica a la tarea de encoder
//     // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//
//     // gpio_set_irq_enabled(ENCODER_PIN_B, GPIO_IRQ_EDGE_FALL, false);
//
//     // channel_a_state = gpio_get(ENCODER_PIN_A);
//     // channel_b_state = gpio_get(ENCODER_PIN_B);
//     // pin = ENCODER_PIN_B;
//
//     // vTaskNotifyGiveFromISR(handle_Encoder, &xHigherPriorityTaskWoken);
//
//     // portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// }
//..........................................................................................
static void prvTimerRtos_Antirrebote(TimerHandle_t xTimer)
{
    /* Vuelve a habilitar la interrupcion. */
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, true, &encoder_isrA);
    // gpio_set_irq_enabled_with_callback(ENCODER_PIN_B, GPIO_IRQ_EDGE_FALL, true, &encoder_isrB);

    return;
}
//..........................................................................................
static void perifericoQueue_init(void)
{
    perifericoQueue = xQueueCreate(QUEUE_LENGTH, sizeof(periferico_t)); // Cola con 10 elementos
    configASSERT(perifericoQueue != NULL);
    return;
}
//..........................................................................................
static bool perifericoQueue_send(periferico_t *dato)
{
    return xQueueSend(perifericoQueue, dato, pdMS_TO_TICKS(10)) == pdPASS;
}
//..........................................................................................
static bool getState_Pulsador(uint16_t pin)
{
    return gpio_get(pin);
}
//..........................................................................................
#define TASKRTOS_PULSADORES_PRIORITY tskIDLE_PRIORITY + 1
static void init_task(void)
{
    BaseType_t status;

    status = xTaskCreate(prvTaskRtos_encoder, "Int encoder", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES, &handle_Encoder);

    // configASSERT(status == NULL);
    if (status != pdPASS)
    {
        printf("Error: No se pudo crear la tarea del encoder\n");
        while (1)
            ; // Detener el sistema si es crítico
    }

    status = xTaskCreate(prvTaskRtos_pulsadores, "Task pulsadores", configMINIMAL_STACK_SIZE, NULL, TASKRTOS_PULSADORES_PRIORITY, NULL);

    // configASSERT(status == NULL);
    if (status != pdPASS)
    {
        printf("Error: No se pudo crear la tarea de pulsadores\n");
        while (1)
            ; // Detener el sistema si es crítico
    }

    return;
}
//..........................................................................................
static void init_timers(void)
{
    timer_Antirrebote = xTimerCreate(
        "Timer Antirrebote",
        pdMS_TO_TICKS(20),
        pdFALSE,
        (void *)0,
        prvTimerRtos_Antirrebote);

    configASSERT(timer_Antirrebote == NULL);

    return;
}
//..........................................................................................
static void init_pins(void)
{
    // Pulsadores
    gpio_init(AVANCE_PIN);
    gpio_set_dir(AVANCE_PIN, GPIO_IN);

    gpio_init(ATRAS_PIN);
    gpio_set_dir(ATRAS_PIN, GPIO_IN);

    gpio_init(MULTIPLICADOR_PIN);
    gpio_set_dir(MULTIPLICADOR_PIN, GPIO_IN);

    // Buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Encoder
    gpio_init(ENCODER_PIN_A);
    gpio_set_dir(ENCODER_PIN_A, GPIO_IN);
    // gpio_set_slew_rate(ENCODER_PIN_A, GPIO_SLEW_RATE_FAST);
    gpio_init(ENCODER_PIN_B);
    gpio_set_dir(ENCODER_PIN_B, GPIO_IN);
    // gpio_set_slew_rate(ENCODER_PIN_B, GPIO_SLEW_RATE_FAST);
    gpio_set_irq_enabled_with_callback(ENCODER_PIN_A, GPIO_IRQ_EDGE_FALL, true, &encoder_isrA);
    // gpio_set_irq_enabled_with_callback(ENCODER_PIN_B, GPIO_IRQ_EDGE_FALL, true, &encoder_isrB);

    return;
}
//..........................................................................................
static void init_encoder(void)
{
    /* Configuracion del selector. */
    DatosPeriferico.encoder.cont = 0;
    DatosPeriferico.encoder.direccion = SENTIDO_HORARIO;
    DatosPeriferico.encoder.multAmpl = MULT_AMP_X_1;
    DatosPeriferico.encoder.multFreq = MULT_FREQ_X_1;
    DatosPeriferico.encoder.multOffset = MULT_AMP_X_1;

    return;
}
//..........................................................................................
static void Buzzer(void)
{
#define DELAY_BUZZER pdMS_TO_TICKS(50)

    gpio_put(BUZZER_PIN, 1);
    vTaskDelay(DELAY_BUZZER);
    gpio_put(BUZZER_PIN, 0);

    return;
}
//..........................................................................................