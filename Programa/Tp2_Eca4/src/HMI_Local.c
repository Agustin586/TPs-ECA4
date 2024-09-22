/**
 * @file Hmi Local
 * @brief Este archivo se encarga de mostrar por pantalla las
 * variables externas y los actuadores. Tambien permitirá
 * modificar ciertos parámetros del control que se realice.
 * @date 22/09/24.
 */

#include "HMI_Local.h"

#include <stdio.h>

#include "FreeRTOSConfig.h"
#include "freertos/task.h"

#include "Sensado.h"

//............................................................................
//                      PROTOTIPO DE FUNCIONES PRIVADAS
//............................................................................
/**
 * @brief Inicialización de las tareas de freertos.
 */
static void tareas_init(void);
/**
 * @brief Inicialización del lcd 20x4.
 */
static void lcd_init(void);
/**
 * @brief Tarea de lcd.
 */
static void taskRtos_Lcd(void *pvParameters);

//............................................................................
//                           CUERPO DE FUNCIONES
//............................................................................
extern void HmiLocal_init(void)
{
    tareas_init();

    return;
}

//............................................................................
static void tareas_init(void)
{
    BaseType_t status = xTaskCreate(
        taskRtos_Lcd,             // Función de la tarea
        "Tarea de Lcd",           // Nombre de la tarea (solo para depuración)
        configMINIMAL_STACK_SIZE, // Tamaño del stack en palabras
        NULL,                     // Parámetro que se pasa a la tarea (pvParameters)
        2,                        // Prioridad de la tarea
        NULL                      // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    if (status != pdPASS)
        printf("\n\rError: no se pudo crear la tarea.\n\r");

    configASSERT(status != NULL); // Detiene el programa si no puede crear la tarea.

    return;
}
//............................................................................
static void lcd_init(void)
{

    return;
}
//............................................................................
static void taskRtos_Lcd(void *pvParameters)
{
    /* Inicializaciones. */
    lcd_init();

    /* Loop. */
    for (;;)
    {
    }

    vTaskDelay(NULL); // No debería salir pero se borra por seguridad.

    return;
}