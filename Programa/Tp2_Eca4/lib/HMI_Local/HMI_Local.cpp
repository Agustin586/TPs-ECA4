/**
 * @file Hmi Local
 * @brief Este archivo se encarga de mostrar por pantalla las
 * variables externas y los actuadores. Tambien permitirá
 * modificar ciertos parámetros del control que se realice.
 * @date 22/09/24.
 */

#include <HMI_Local.h>

#include <stdio.h>

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <Sensado.h>

//............................................................................
//                          VARIABLES PRIVADAS
//............................................................................
// Dirección I2C del LCD (puede variar dependiendo del adaptador, por defecto 0x27 o 0x3F)
#define I2C_ADDR 0x27

// Crea un objeto de la clase LiquidCrystal_I2C, con la dirección I2C y el tamaño de la pantalla (20x4)
LiquidCrystal_I2C lcd(I2C_ADDR, 20, 4);
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
        taskRtos_Lcd,   // Función de la tarea
        "Tarea de Lcd", // Nombre de la tarea (solo para depuración)
        2048,           // Tamaño del stack en palabras
        NULL,           // Parámetro que se pasa a la tarea (pvParameters)
        1,              // Prioridad de la tarea
        NULL            // Manejador de la tarea (opcional, puedes dejar NULL)
    );

    configASSERT(status == pdPASS); // Detiene el programa si no puede crear la tarea.

    printf("\n\rTareas de HMI creadas con exito.\n\r");

    return;
}
//............................................................................
static void lcd_init(void)
{
    // Inicializa la comunicación I2C
    // Wire.begin(21, 22); // SDA = GPIO 21, SCL = GPIO 22

    lcd.init();
    lcd.backlight();

    lcd.clear();

    // Imprime un mensaje en la primera línea
    lcd.print("TP2 DISPOSITIVOS IV");

    // Imprime un mensaje en la segunda línea
    lcd.setCursor(0,1);
    lcd.print("SENSADO DE PARAMETRO");

    lcd.setCursor(8, 2);
    lcd.print("Z,F,S");

    return;
}
//............................................................................
static void taskRtos_Lcd(void *pvParameters)
{
    /* Inicializaciones. */
    lcd_init();

    vTaskDelay(pdMS_TO_TICKS(2000));

    /* Nivel de stack de la tarea */
    // UBaseType_t stackFree = uxTaskGetStackHighWaterMark(NULL);
    // Serial.println(stackFree); // Muestra el espacio libre en la pila
    // Sobran ~260 palabras

    lcd.clear();

    /* Loop. */
    for (;;)
    {
        char buffer[15];

        /* Imprimimos la temperatura. */
        float Temp = (Sensado_Temp() * (3.3 / 4095.0)) / 0.01;
        sprintf(buffer, "Temp:%.1f C",Temp);
        lcd.setCursor(0, 0);
        lcd.print(buffer);

        /* Imprimimos la humedad. */
        sprintf(buffer,"Humd:%d",Sensado_Humd());
        lcd.setCursor(12, 0);
        lcd.print(buffer);

        /* Imprimimos la luz. */
        sprintf(buffer,"Luz:%d",Sensado_Luz());
        lcd.setCursor(0, 1);
        lcd.print(buffer);

        /* Imprimimos el estado del rele. */
        sprintf(buffer, "Rele:%s", Sensado_getReleState() ? "ON" : "OFF");
        lcd.setCursor(0,3);
        lcd.print(buffer);

        /* Imprimimos el valor del pwm. */
        sprintf(buffer,"Pwm:%d%%",Sensado_getPwmState());
        lcd.setCursor(12,3);
        lcd.print(buffer);

        vTaskDelay(pdMS_TO_TICKS(500));

        lcd.clear();    // Limpiamos la pantalla
    }

    vTaskDelete(NULL); // No debería salir pero se borra por seguridad.

    return;
}