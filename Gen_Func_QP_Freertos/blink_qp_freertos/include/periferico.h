#ifndef INCLUDE_PERIFERICOS_H_
#define INCLUDE_PERIFERICOS_H_

#include "config.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    /**
     * @brief Multiplicador de frecuencia
     */
    MultFreq_t multFreq;
    /**
     * @brief Multiplicador de amplitud
     */
    MultAmp_t multAmpl;
    /**
     * @brief Multiplicador de offset
     */
    MultOffset_t multOffset;
    /**
     * @brief Contador del encoder
     */
    int16_t cont;
    /**
     * @brief Sentido de giro
     *
     *  1: sentido horario
     * -1: sentido antihorario
     */
    int8_t direccion;
} Selector_t;

typedef struct
{
    // Publico:
    Selector_t encoder;
    bool evt_encoder;
    bool evt_avance;
    bool evt_atras;
    bool evt_mult;
    bool evt_confr;
    bool evt_stop;
} periferico_t;

/**
 * @brief Inicializacion de perifericos
 *
 */
extern void EtapaPerf_init(void);
/**
 * @brief Recibe un dato de la cola de datos
 *
 * @param dato tipo de dato periferico
 * @return true habia datos disponible
 * @return false no habi dato disponible
 */
extern bool periferico_QueueReceive(periferico_t *dato);
/**
 * @brief Resetea los multiplicadores a los primeros
 *
 * @param Perf_obj
 */
extern void periferico_ResetMultiplicadores(periferico_t *Perf_obj);

#endif