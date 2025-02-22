#include "display.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "awg.h"
#include "signal.h"

// Definición de pines y UART
#define UART_ID uart0
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
    sendCommand("rest");

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
    switch (get_funcion())
    {
    case SIGNAL_SINE:
        sendCommand("bFunction.txt=\"SINE>\"");
        break;
    case SIGNAL_SQUARE:
        sendCommand("bFunction.txt=\"<SQUA.>\"");
        break;
    case SIGNAL_TRIANGLE:
        sendCommand("bFunction.txt=\"<TRIAN.>\"");
        break;
    case SIGNAL_SAWTOOTH:
        sendCommand("bFunction.txt=\"<SAWT.\"");
        break;
    default:
        sendCommand("bFunction.txt=\"Desconocido\"");
        break;
    }

    return;
}

// Actualiza la frecuencia en la pantalla
extern void display_freq(void)
{
    char cmd[50];
    sprintf(cmd, "tFreq.txt=\"%.0f\"", get_frequency());
    sendCommand(cmd);
    return;
}

// Actualiza la amplitud en la pantalla
extern void display_amp(void)
{
    char cmd[50];
    sprintf(cmd, "tAmp.txt=\"%.2f\"", get_amplitude());
    sendCommand(cmd);
    return;
}

// Actualiza el offset en la pantalla
extern void display_offset(void)
{
    char cmd[50];
    sprintf(cmd, "tOffset.txt=\"%.2f\"", get_offset());
    sendCommand(cmd);
    return;
}

// Actualiza la salida en la pantalla
extern void display_salida(void)
{
    sendCommand("tSalida.txt=\"Salida Actual\"");
    return;
}

extern void display_pulsarBoton(Boton_t TipoBoton, bool state)
{
    switch (TipoBoton)
    {
    case BOTON_FUNCTION:
        state ? sendCommand("click bFunction,1") : sendCommand("click bFunction,0");
        break;
    case BOTON_AMP:
        state ? sendCommand("click bAmp,1") : sendCommand("click bAmp,0");
        break;
    case BOTON_OFFSET:
        state ? sendCommand("click bOffset,1") : sendCommand("click bOffset,0");
        break;
    case BOTON_FREQ:
        state ? sendCommand("click bFreq,1") : sendCommand("click bFreq,0");
        break;
    case BOTON_DUTY:
        state ? sendCommand("click bDuty,1") : sendCommand("click bDuty,0");
        break;
    case BOTON_SALIDA:
        sendCommand("click bSalida,1");
    default:
        break;
    }
    return;
}

// Envía un punto al widget de forma de onda
static void sendWaveformPoint(uint8_t channel, uint8_t value)
{
    char cmd[30];
    sprintf(cmd, "add 6,%d,%d", channel, value);
    sendCommand(cmd);
}
// Escala la amplitud de 1-10V a 0-100
static uint8_t scaleAmplitude(float amplitude)
{
    if (amplitude < 1.0)
        amplitude = 1.0;
    if (amplitude > 10.0)
        amplitude = 10.0;
    return (uint8_t)((amplitude - 1.0) / 9.0 * 100.0);
}
// Dibuja una señal senoide en el widget de forma de onda
static void drawSineWave(float amplitude, float frequency, float offset)
{
    #define MAX_SAMPLES 200
    #define MIN_SAMPLES 10

    #define AMP_MAX 45
    #define AMP_MIN 10
    
    #define OFFSET_MAX 46

    // uint8_t scaledAmplitude = scaleAmplitude(amplitude);
    uint16_t samples = MAX_SAMPLES;

    if (samples > MAX_SAMPLES)
        samples = MAX_SAMPLES;
    else if (samples < MIN_SAMPLES)
        samples = MIN_SAMPLES;

    for (int i = 0; i < samples; ++i)
    {
        uint8_t value = (uint8_t)(46 + AMP_MAX * sin(2 * 3.14 * i / samples));
        sendWaveformPoint(0, value);
    }
}
// Dibuja una señal triangular en el widget de forma de onda
static void drawTriangleWave(float amplitude, float frequency, float offset)
{
    // uint8_t scaledAmplitude = scaleAmplitude(amplitude);
    uint8_t scaledAmplitude = 35;
    offset = 10;

    uint8_t samples = 150;
    for (int i = 0; i < samples; ++i)
    {
        uint8_t value = (i < samples / 2) ? (uint8_t)(offset + 2 * scaledAmplitude * i / (samples / 2)) : (uint8_t)(offset + 2 * scaledAmplitude * (samples - i) / (samples / 2));
        sendWaveformPoint(0, value);
    }
    for (int i = 0; i < samples/2; ++i)
    {
        uint8_t value = (i < samples / 2) ? (uint8_t)(offset + 2 * scaledAmplitude * i / (samples / 2)) : (uint8_t)(offset + 2 * scaledAmplitude * (samples - i) / (samples / 2));
        sendWaveformPoint(0, value);
    }
}
// Dibuja una señal cuadrada en el widget de forma de onda
static void drawSquareWave(float amplitude, float frequency, float offset)
{
    uint8_t scaledAmplitude = scaleAmplitude(amplitude);
    uint8_t samples = 200;
    for (int i = 0; i < samples; ++i)
    {
        uint8_t value = (i < samples / 2) ? (uint8_t)(offset + scaledAmplitude) : (uint8_t)(offset - scaledAmplitude);
        sendWaveformPoint(0, value);
    }
}
// Función para seleccionar y dibujar la señal
extern void drawWaveform(uint8_t waveformType, float amplitude, float frequency, float offset)
{
    // Limpia el widget de forma de onda antes de dibujar una nueva señal
    sendCommand("cle 6,0");
    switch (waveformType)
    {
    case 0:
        drawSineWave(amplitude, frequency, offset);
        break;
    case 1:
        drawSquareWave(amplitude, frequency, offset);
        break;
    case 2:
        drawTriangleWave(amplitude, frequency, offset);
        break;
    default:
        break;
    }
}

// Configura el texto del objeto tMult en la pantalla Nextion
extern void display_setMultiplicadorText(const char *text)
{
    char cmd[50];
    sprintf(cmd, "tMult.txt=\"%s\"", text);
    sendCommand(cmd);
}