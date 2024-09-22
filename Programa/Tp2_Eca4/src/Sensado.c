/**
 * @file Sensado.c
 * @brief Este archivo se encarga de tomar los datos de las
 * variables externas como luz, temperatura y humedad. Luego
 * a partir de esto generar una respuesta en los actuadores.
 * @author Zuliani, Agustin. Fasolato, Alejandro. Salomon, Uriel.
 * @date 22/09/24.
 */

#include "Sensado.h"

#include "FreeRTOSConfig.h"
#include "freertos/task.h"

//............................................................................
//                          PROTOTIPO DE FUNCIONES
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

//............................................................................
//                           CUERPO DE FUNCIONES
//............................................................................
extern void Sensado_init(void)
{
    tareas_init();
    perifericos_init();

    vTaskStartScheduler(); // Incializa el scheduler.

    return;
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

    configASSERT(status != NULL); // Detiene el programa si no puede crear la tarea.

    status = xTaskCreate(
        taskRtos_Temperatura,     // Función de la tarea
        "Tarea de Temperatura",   // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        1,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status != NULL); // Detiene el programa si no puede crear la tarea.

    status = xTaskCreate(
        taskRtos_Humedad,         // Función de la tarea
        "Tarea de Humedad",       // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        1,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status != NULL); // Detiene el programa si no puede crear la tarea.

    return;
}
//............................................................................
static void perifericos_init(void)
{

    return;
}
//............................................................................
static void taskRtos_Luz(void *pvParameters)
{
    /* Inicializaciones. */

    /* Loop. */
    for (;;)
    {
    }

    vTaskDelay(NULL); // No debería salir pero se borra por seguridad.

    return;
}
//............................................................................
static void taskRtos_Temperatura(void *pvParameters)
{
    /* Inicializaciones. */

    /* Loop. */
    for (;;)
    {
    }

    vTaskDelay(NULL); // No debería salir pero se borra por seguridad.

    return;
}
//............................................................................
static void taskRtos_Humedad(void *pvParameters)
{
    /* Inicializaciones. */

    /* Loop. */
    for (;;)
    {
    }

    vTaskDelay(NULL); // No debería salir pero se borra por seguridad.

    return;
}