#ifndef INCLUDE_SIGNAL_H_
#define INCLUDE_SIGNAL_H_

#include "pico/stdlib.h"
#include <stdint.h>
#include <stdbool.h>

#define BUFFER_DEPTH 4096

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

// DECLARACION DE FUNCIONES
/**
 * @brief Inicializa la señal del objeto
 * 
 * @param sg Objeto señal
 */
extern void signal_init(SignalGenerator *sg);
/**
 * @brief Configuracion de frecuencia.
 */
extern void signal_config_freq(SignalGenerator *sg, float freq);
/**
 * @brief Configuracion de amplitud.
 */
extern void signal_config_amplitude(SignalGenerator *sg, float amplitude);
/**
 * @brief Configuracion de offset.
 */
extern void signal_config_offset(SignalGenerator *sg, float offset);
/**
 * @brief Inicia la señal de salida
 * 
 * @param sg 
 */
extern void signal_start(SignalGenerator *sg);
/**
 * @brief Detiene la generacion de señal.
 * 
 * @param sg 
 */
extern void signal_stop(SignalGenerator *sg);

#endif  // INCLUDE_SIGNAL_H_