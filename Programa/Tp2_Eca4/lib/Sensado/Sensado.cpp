/**
 * @file Sensado.c
 * @brief Este archivo se encarga de tomar los datos de las
 * variables externas como luz, temperatura y humedad. Luego
 * a partir de esto generar una respuesta en los actuadores.
 * @author Zuliani, Agustin. Fasolato, Alejandro. Salomon, Uriel.
 * @date 22/09/24.
 */

#include <Sensado.h>

#include <stdio.h>

#include <Arduino.h>

#include "esp_system.h"
#include "esp_task_wdt.h"

// #include "DHT.h"

//............................................................................
//                              DEFINICIONES PRIVADAS
//............................................................................
/* ENTRADAS ANALOGICAS. */
#define TEMP_PIN 34
#define LUZ_PIN 35
#define PERIODIC_TIME_ADC_READ_TEMP 500
#define PERIODIC_TIME_ADC_READ_LUZ 100

/* SALIDAS Y ENTRADAS DIGITALES. */
#define RELE_PIN 14
#define PULSADOR_1_PIN 32
#define PULSADOR_2_PIN 33
#define RGB_RED_PIN 26
#define RGB_BLUE_PIN 16
#define RGB_GREEN_PIN 27

/* ATAJOS PARA ACTIVAR O DESACTIVAR SALIDAS. */
#define RELE_ESTADO(x) digitalWrite(RELE_PIN, x)
#define RGB_RED_ESTADO(x) digitalWrite(RGB_RED_PIN, x)
#define RGB_BLUE_ESTADO(x) digitalWrite(RGB_BLUE_PIN, x)
#define RGB_GREEN_ESTADO(x) digitalWrite(RGB_GREEN_PIN, x)

/* PWM. */
#define PWM_PIN 25         // Pin donde se generará el PWM
#define PWM_FREQUENCY 5000 // Frecuencia del PWM (en Hz)
#define PWM_RESOLUTION 8   // Resolución del PWM (8 bits)
#define PWM_DUTY_CYCLE 128 // Ciclo de trabajo (0-255)

#define _delay_ms(x) vTaskDelay(pdMS_TO_TICKS(x))
#define LED_PIN 2

// #define DHTPIN 3      // Pin donde está conectado el pin de datos del DHT11
// #define DHTTYPE DHT11 // Definir el tipo de sensor DHT (DHT11, DHT22 o DHT21)

//............................................................................
//                              VARIABLES PRIVADAS
//............................................................................
typedef struct
{
    uint16_t luz;         // Variable de luz medida
    uint16_t temperatura; // Variable de temperatura medida
    uint16_t humedad;     // Variable de humedad medida
} VariablesEntrada;

typedef struct
{
    bool rele; // Estado del relé (0: apagado, 1: encendido)
    bool rgb_red;
    bool rgb_green;
    bool rgb_blue;
    int pwm; // Ciclo de trabajo del PWM (0-100%)
} VariablesSalida;

typedef struct
{
    VariablesEntrada entradas; // Estructura que agrupa las entradas
    VariablesSalida salidas;   // Estructura que agrupa las salidas
} SistemaControl_t;

static SistemaControl_t sistema = {
    .entradas = {0, 0, 0}, // Inicializa entradas a 0
    .salidas = {0, 0}      // Inicializa salidas a 0
};

// DHT dht(DHTPIN, DHTTYPE); // Inicializa el sensor DHT

static int valorTemp = 0; // Lectura adc de temperatura
static int valorLuz = 0;  // Lectura adc de luz

TimerHandle_t timerAdcLuz;
TimerHandle_t timerAdcTemp;
//............................................................................
//                      PROTOTIPO DE FUNCIONES PRIVADAS
//............................................................................
/**
 * @brief Inicialización de las tareas de freertos.
 */
static void tareas_init(void);
/**
 * @brief Incialización de los perifericos como Adc, puerto serie, etc.
 */
static void perifericos_init(void);
/**
 * @brief Tarea de sensado de luz ambiente.
 */
static void taskRtos_Luz(void *pvParameters);
/**
 * @brief Tarea de sensado de temperatura.
 */
static void taskRtos_Temperatura(void *pvParameters);
/**
 * @brief Tarea de sensado de humedad.
 */
static void taskRtos_Humedad(void *pvParameters);
/**
 * @brief Tarea de blink led placa.
 */
static void taskRtos_Blink(void *pvParameters);
/**
 * @brief Se encarga de configurar los timers.
 */
static void timer_init(void);
/**
 * @brief Funcion de callback de lectura de luz.
 */
static void timerRtos_Luz(TimerHandle_t xTimer);
/**
 * @brief Funcion de callback de lectura de temperatura.
 */
static void timerRtos_Temp(TimerHandle_t xTimer);

//............................................................................
//                           CUERPO DE FUNCIONES
//............................................................................
extern void Sensado_init(void)
{
    tareas_init();
    timer_init();
    perifericos_init();

    // Activa el scheduler manualmente genera errores. No debe
    // hacerse en el framevowrk de arduino.
    // vTaskStartScheduler(); // Incializa el scheduler.

    return;
}
//............................................................................
extern uint16_t Sensado_Temp(void)
{
    return sistema.entradas.temperatura;
}
//............................................................................
extern uint16_t Sensado_Humd(void)
{
    return sistema.entradas.humedad;
}
//............................................................................
extern uint16_t Sensado_Luz(void)
{
    return sistema.entradas.luz;
}
//............................................................................
extern bool Sensado_getReleState(void)
{
    return sistema.salidas.rele;
}
//............................................................................
extern int Sensado_getPwmState(void)
{
    return sistema.salidas.pwm;
}

//............................................................................
static void tareas_init(void)
{
    BaseType_t status = xTaskCreate(
        taskRtos_Luz,             // Función de la tarea
        "Tarea de Luz",           // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        2,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status == pdPASS); // Detiene el programa si no puede crear la tarea.

    status = xTaskCreate(
        taskRtos_Temperatura,     // Función de la tarea
        "Tarea de Temperatura",   // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        1,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status == pdPASS);

    status = xTaskCreate(
        taskRtos_Humedad,         // Función de la tarea
        "Tarea de Humedad",       // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        1,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status == pdPASS);

    status = xTaskCreate(
        taskRtos_Blink,           // Función de la tarea
        "Tarea de BLink",         // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        1,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status == pdPASS);

    printf("\n\rTareas de sensado creadas con exito.\n\r");

    return;
}
//............................................................................
static void perifericos_init(void)
{
    /* Configuramos las entradas y salidas. */
    pinMode(RELE_PIN, OUTPUT);
    pinMode(PULSADOR_1_PIN, INPUT);
    pinMode(PULSADOR_2_PIN, INPUT);
    pinMode(RGB_RED_PIN, OUTPUT);
    pinMode(RGB_BLUE_PIN, OUTPUT);
    pinMode(RGB_GREEN_PIN, OUTPUT);

    RELE_ESTADO(LOW);
    RGB_RED_ESTADO(LOW);
    RGB_BLUE_ESTADO(LOW);
    RGB_GREEN_ESTADO(LOW);

    /* Configuración de PWM. */
    pinMode(PWM_PIN, OUTPUT);

    ledcSetup(0, PWM_FREQUENCY, PWM_RESOLUTION); // Canal 0
    ledcAttachPin(PWM_PIN, 0);                   // Asocia el pin al canal 0

    ledcWrite(0, PWM_DUTY_CYCLE); // Establece el ciclo de trabajo (0-255)

    return;
}
//............................................................................
static void taskRtos_Luz(void *pvParameters)
{
#define UMBRAL_ON 700  // Valor umbral para encender (ajusta según tus lecturas)
#define UMBRAL_OFF 600 // Valor umbral para apagar (debe ser menor que UMBRAL_ON)

    /* Inicializaciones. */
    sistema.salidas.rele = 0;
    sistema.salidas.rgb_blue = 0;
    sistema.salidas.rgb_green = 0;
    sistema.salidas.rgb_red = 0;

    /* Loop. */
    for (;;)
    {
        /* Obtiene el valor de la luz. */
        sistema.entradas.luz = valorLuz;

        /* Control ON-OFF con HISTERESIS. */
        if (sistema.entradas.luz > UMBRAL_ON && sistema.salidas.rele == false)
        {
            sistema.salidas.rele = true;
            RELE_ESTADO(HIGH);
        }
        else if (sistema.entradas.luz < UMBRAL_OFF && sistema.salidas.rele == true)
        {
            sistema.salidas.rele = false;
            RELE_ESTADO(LOW);
        }

        _delay_ms(100); // Esperamos un tiempo corto
    }

    vTaskDelete(NULL); // No debería salir pero se borra por seguridad.

    return;
}
//............................................................................
static void taskRtos_Temperatura(void *pvParameters)
{
    /* Inicializaciones. */
    sistema.salidas.pwm = PWM_DUTY_CYCLE;

    static uint8_t constP = 1;

    /* Loop. */
    for (;;)
    {
        /* Obtenemos el valor de la temperatura. */
        float Temp = (valorTemp * 3.3/4095.0) / 0.01;
        sistema.entradas.temperatura = valorTemp;

        /* Control P. */
        sistema.salidas.pwm = (int)((constP / Temp) * 100.0);
        
        if (sistema.salidas.pwm > 100)
            sistema.salidas.pwm = 100;
        

        _delay_ms(100);
    }

    vTaskDelete(NULL); // No debería salir pero se borra por seguridad.

    return;
}
//............................................................................
static void taskRtos_Humedad(void *pvParameters)
{
    /* Inicializaciones. */
    // dht.begin(); // Inicia el sensor DHT

    /* Loop. */
    for (;;)
    {
        // Leer la humedad
        // float humedad = dht.readHumidity();

        // Comprueba si la lectura es válida
        // if (isnan(humedad))
        // {
        //     Serial.println("Error al leer el sensor DHT11");
        //     return;
        // }

        _delay_ms(2000);
    }

    vTaskDelete(NULL); // No debería salir pero se borra por seguridad.

    return;
}
//............................................................................
static void taskRtos_Blink(void *pvParameters)
{
    pinMode(LED_PIN, OUTPUT);

    for (;;)
    {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    vTaskDelete(NULL);

    return;
}
//............................................................................
static void timer_init(void)
{
    // Crear el temporizador
    timerAdcLuz = xTimerCreate(
        "Timer Luz",                               // Nombre del temporizador
        pdMS_TO_TICKS(PERIODIC_TIME_ADC_READ_LUZ), // Período del temporizador (en milisegundos, aquí 1000 ms = 1 s)
        pdTRUE,                                    // pdTRUE para que se reinicie automáticamente (repetición periódica)
        (void *)0,                                 // ID opcional que puedes usar para pasar datos al callback (normalmente NULL)
        timerRtos_Luz                              // Función que se llamará cuando el temporizador expire
    );

    configASSERT(timerAdcLuz != NULL);

    // Crear el temporizador
    timerAdcTemp = xTimerCreate(
        "Timer Temperatura",                        // Nombre del temporizador
        pdMS_TO_TICKS(PERIODIC_TIME_ADC_READ_TEMP), // Período del temporizador (en milisegundos, aquí 1000 ms = 1 s)
        pdTRUE,                                     // pdTRUE para que se reinicie automáticamente (repetición periódica)
        (void *)0,                                  // ID opcional que puedes usar para pasar datos al callback (normalmente NULL)
        timerRtos_Temp                              // Función que se llamará cuando el temporizador expire
    );

    BaseType_t status = xTimerStart(timerAdcLuz, 0);
    configASSERT(status == pdPASS);

    status = xTimerStart(timerAdcTemp, 0);
    configASSERT(status == pdPASS);

    printf("\n\rTemporizadores creados e iniciados correctamente.\n\r");

    return;
}
//............................................................................
static void timerRtos_Luz(TimerHandle_t xTimer)
{
    valorLuz = analogRead(LUZ_PIN);

    return;
}
//............................................................................
static void timerRtos_Temp(TimerHandle_t xTimer)
{
    valorTemp = analogRead(TEMP_PIN);

    return;
}