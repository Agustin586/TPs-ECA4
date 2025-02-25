#ifndef INCLUDE_DAC_R2R_H_
#define INCLUDE_DAC_R2R_H_

#include "signal.h"

typedef struct
{
    // Publico:
    SignalGenerator signal;
} dacR2R_t;

/**
 * @brief Configura la señal del dac
 *
 * @param sg Objeto tipo señal
 */
extern void EtapaDacR2R_init(dacR2R_t *dac);
/**
 * @brief Habilita la señal del generador de funciones
 *
 * @param sg Objeto tipo señal
 */
extern void EtapaDacR2R_enable(dacR2R_t *dac);
/**
 * @brief Deshabilita la señal
 *
 * @param sg Objeto de tipo señal
 */
extern void EtapaDacR2R_disable(dacR2R_t *dac);
/**
 * @brief Setea la amplitud
 *
 * @param sg
 * @param amp
 */
extern void dacR2R_setAmplitud(dacR2R_t *dac, float amp);
/**
 * @brief Setea la frecuencia
 *
 * @param sg
 * @param freq
 */
extern void dacR2R_setFreq(dacR2R_t *dac, float freq);
/**
 * @brief Setea el offset
 *
 * @param sg
 * @param offset
 */
extern void dacR2R_setOffset(dacR2R_t *dac, float offset);

#endif