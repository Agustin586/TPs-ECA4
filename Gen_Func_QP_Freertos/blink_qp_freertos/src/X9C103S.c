#include "X9C103S.h"

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include <FreeRTOS.h>
#include <task.h>

// Definición de pines para el control del X9C103S (ajusta según tu hardware)
#define INC_PIN 27
#define U_D_PIN 28
#define CS_PIN 2

#define LOW 0
#define HIGH 1

// Resistencia máxima del X9C103S (10kΩ)
#define MAX_RESISTANCE 10000.0
#define MIN_RESISTANCE 40.0

// Delay de freertos
#define DELAY_X9C103S vTaskDelay(pdMS_TO_TICKS(2))

/**
 * @brief Genera un flanco descente en el pin INC, con un delay dado por
 * DELAY_X9C103S.
 */
static void INC_fallingEdge(void);

//...........................................................................................
extern void X9C103S_init(X9C103S *device)
{
    // Inicializa los pines
    gpio_init(INC_PIN);
    gpio_set_dir(INC_PIN, GPIO_OUT);
    gpio_init(U_D_PIN);
    gpio_set_dir(U_D_PIN, GPIO_OUT);
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);

    X9C103S_setToZero(device);

    // Deberia estar en aprox 40 ohm entonces guardamos en memoria
    // Como es no volatil, deberia cargarse el programa una sola vez
    // y luego comentar estas lineas.
    // gpio_put(CS_PIN, LOW);
    // gpio_put(INC_PIN, HIGH);
    // vTaskDelay(pdMS_TO_TICKS(2));
    // gpio_put(CS_PIN, HIGH);

    return;
}
//...........................................................................................
extern void X9C103S_increase(X9C103S *device, uint8_t target_pos)
{
#define MAX_COUNTER 99
#define MAX_COUNTER_FLOAT 99.0
    if (device->position < MAX_COUNTER)
    {
        /* Configuracion inicial */
        gpio_put(U_D_PIN, LOW);  // Configura U/D para aumentar
        gpio_put(CS_PIN, LOW);   // CS
        gpio_put(INC_PIN, HIGH); // Estado inicial de INC
        DELAY_X9C103S;

        while (device->position < target_pos)
        {
            INC_fallingEdge();

            device->position++;
            device->resistance = ((device->position / MAX_COUNTER_FLOAT) * MAX_RESISTANCE);
        }
    }

    return;
}
//...........................................................................................
extern void X9C103S_decrease(X9C103S *device, uint8_t target_pos)
{
#define MAX_COUNTER_FLOAT 99.0
#define MIN_COUNTER 0
    if (device->position > MIN_COUNTER)
    {
        /* Configuracion inicial */
        gpio_put(U_D_PIN, HIGH); // Configura U/D para disminuir
        gpio_put(CS_PIN, LOW);
        gpio_put(INC_PIN, HIGH);
        DELAY_X9C103S;

        while (device->position > target_pos)
        {
            INC_fallingEdge();

            device->position--;
            device->resistance = (device->position / MAX_COUNTER_FLOAT) * MAX_RESISTANCE;
        }
    }

    return;
}
//...........................................................................................
extern float X9C103S_get_resistance(X9C103S *device)
{
    return device->resistance;
}
//...........................................................................................
extern void X9C103S_set_resistance(X9C103S *device, float resistance)
{
    if (resistance > MAX_RESISTANCE)
    {
        resistance = MAX_RESISTANCE;
    }
    else if (resistance < MIN_RESISTANCE)
    {
        resistance = MIN_RESISTANCE;
    }

    uint8_t target_position = (uint8_t)((resistance / MAX_RESISTANCE) * 99.0);

    X9C103S_setToZero(device);

    if (device->position < target_position)
    {
        X9C103S_increase(device, target_position);
        return;
    }
    else if (device->position > target_position)
    {
        X9C103S_decrease(device, target_position);
        return;
    }

    return;
}
//...........................................................................................
extern void X9C103S_incrementar(void)
{
    /* Configuracion inicial */
    gpio_put(U_D_PIN, LOW);  // Configura U/D para aumentar
    gpio_put(CS_PIN, LOW);   // CS
    gpio_put(INC_PIN, HIGH); // Estado inicial de INC
    DELAY_X9C103S;

    INC_fallingEdge();

    return;
}
//...........................................................................................
extern void X9C103S_decrementar(void)
{
    /* Configuracion inicial */
    gpio_put(U_D_PIN, HIGH); // Configura U/D para disminuir
    gpio_put(CS_PIN, LOW);
    gpio_put(INC_PIN, HIGH);
    DELAY_X9C103S;

    INC_fallingEdge();

    return;
}
//...........................................................................................
extern void X9C103S_setToZero(X9C103S *device)
{
    gpio_put(U_D_PIN, HIGH);
    gpio_put(CS_PIN, LOW);
    gpio_put(INC_PIN, HIGH);
    DELAY_X9C103S;

    for (int i = 0; i < 100; ++i)
    {
        X9C103S_decrementar();
    }

    device->position = 0;
    device->resistance = MIN_RESISTANCE;

    return;
}
//...........................................................................................
static void INC_fallingEdge(void)
{
    gpio_put(INC_PIN, LOW);
    DELAY_X9C103S;
    gpio_put(INC_PIN, HIGH);
    DELAY_X9C103S;

    return;
}
//...........................................................................................