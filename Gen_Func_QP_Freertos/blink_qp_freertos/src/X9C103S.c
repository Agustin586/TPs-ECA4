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

// Resistencia máxima del X9C103S (10kΩ)
#define MAX_RESISTANCE 10000.0
#define MIN_RESISTANCE 40.0

// Delay de freertos
#define DELAY_X9C103S   vTaskDelay(pdMS_TO_TICKS(5))

// Implementación de las funciones
extern void X9C103S_init(X9C103S *device)
{
    // Inicializa los pines
    gpio_init(INC_PIN);
    gpio_set_dir(INC_PIN, GPIO_OUT);
    gpio_init(U_D_PIN);
    gpio_set_dir(U_D_PIN, GPIO_OUT);
    gpio_init(CS_PIN);
    gpio_set_dir(CS_PIN, GPIO_OUT);

    // Lleva el potenciómetro a su posición mínima
    gpio_put(U_D_PIN, 0);
    // Configura U/D para disminuir
    gpio_put(CS_PIN, 0);
    // Activa CS
    for (int i = 0; i < 100; ++i)
    {
        gpio_put(INC_PIN, 1);
        DELAY_X9C103S;
        gpio_put(INC_PIN, 0);
        DELAY_X9C103S;
    }
    gpio_put(CS_PIN, 1);

    // Configura la posición inicial
    device->position = 0;
    device->resistance = 100.0;
}

extern void X9C103S_increase(X9C103S *device)
{
    if (device->position < 99)
    {
        gpio_put(U_D_PIN, 1); // Configura U/D para aumentar
        gpio_put(CS_PIN, 0);  // Activa CS
        gpio_put(INC_PIN, 1);
        DELAY_X9C103S;
        gpio_put(INC_PIN, 0);
        DELAY_X9C103S;
        gpio_put(CS_PIN, 1); // Desactiva CS
        device->position++;
        device->resistance = (device->position / 99.0) * MAX_RESISTANCE;
    }
}

extern void X9C103S_decrease(X9C103S *device)
{
    if (device->position > 0)
    {
        gpio_put(U_D_PIN, 0); // Configura U/D para disminuir
        gpio_put(CS_PIN, 0);  // Activa CS
        gpio_put(INC_PIN, 1);
        DELAY_X9C103S;
        gpio_put(INC_PIN, 0);
        DELAY_X9C103S;
        gpio_put(CS_PIN, 1); // Desactiva CS
        device->position--;
        device->resistance = (device->position / 99.0) * MAX_RESISTANCE;
    }
}

extern float X9C103S_get_resistance(X9C103S *device)
{
    return device->resistance;
}

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
    uint8_t target_position = (uint8_t)((resistance / MAX_RESISTANCE) * 99);
    while (device->position < target_position)
    {
        X9C103S_increase(device);
    }
    while (device->position > target_position)
    {
        X9C103S_decrease(device);
    }
}
