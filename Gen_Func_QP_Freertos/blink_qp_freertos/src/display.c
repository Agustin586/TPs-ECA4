#include "display.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "awg.h"

// Definición de pines y UART
#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// Prototipos de funciones
static void sendCommand(const char *cmd);

// Inicializa la comunicación con la pantalla Nextion
extern void display_init(void)
{
    // Inicializa UART
    uart_init(UART_ID, BAUD_RATE);

    // Setea los pines para UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Enviar un comando de inicialización a la pantalla
    sendCommand("page 0");

    return;
}

// Envía un comando a la pantalla Nextion
static void sendCommand(const char *cmd)
{
    uart_puts(UART_ID, cmd);
    uart_putc(UART_ID, 0xFF);
    uart_putc(UART_ID, 0xFF);
    uart_putc(UART_ID, 0xFF);
}

// Actualiza la función de la pantalla
extern void display_func(void)
{
    sendCommand("tFunc.txt=\"Función Actual\"");
    return;
}

// Actualiza la frecuencia en la pantalla
extern void display_freq(void)
{
    char cmd[50];
    sprintf(cmd, "tFreq.txt=\"Frecuencia: %.2f\"", get_frequency());
    sendCommand(cmd);
    return;
}

// Actualiza la amplitud en la pantalla
extern void display_amp(void)
{
    char cmd[50];
    sprintf(cmd, "tAmp.txt=\"Amplitud: %.2f\"", get_amplitude());
    sendCommand(cmd);
    return;
}

// Actualiza el offset en la pantalla
extern void display_offset(void)
{
    char cmd[50];
    sprintf(cmd, "tOffset.txt=\"Offset: %.2f\"", get_offset());
    sendCommand(cmd);
    return;
}

// Actualiza la salida en la pantalla
extern void display_salida(void)
{
    sendCommand("tSalida.txt=\"Salida Actual\"");
    return;
}
