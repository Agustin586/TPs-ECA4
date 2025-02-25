#ifndef INCLUDE_AMPLFICACION_H_
#define INCLUDE_AMPLFICACION_H_

#include "X9C103S.h"
#include "pico/stdlib.h"
#include <stdint.h>

typedef struct
{
    // Privado:
    X9C103S potenciometro;

    // Publico:
    float amp_vp;
    bool state_out;
} amplificacion_t;

extern void EtapaAmp_init(amplificacion_t *Amp_obj);
extern void EtapaAmp_enable(amplificacion_t *Amp_obj);
extern void EtapaAmp_disable(amplificacion_t *Amp_obj);

#endif