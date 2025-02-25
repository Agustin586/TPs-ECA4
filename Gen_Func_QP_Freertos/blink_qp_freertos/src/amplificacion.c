#include "amplificacion.h"

#include "config.h"
#include "X9C103S.h"

#include "pico/stdlib.h"
#include <stdio.h>

#define POTE_DIGITAL_RESISTENCIA_INICIAL 1000
#define R_DAC 1500.0
#define V_DAC 3.3

// FUNCIONES EXTERNAS
//...............................................................
extern void EtapaAmp_init(amplificacion_t *Amp_obj)
{
    Amp_obj->amp_vp = AMPLITUD_SALIDA_MAX / 2.0;
    Amp_obj->potenciometro.resistance = POTE_DIGITAL_RESISTENCIA_INICIAL;

    /* Configura el pote digital. */
    X9C103S_init(&(Amp_obj->potenciometro));
    X9C103S_set_resistance(&(Amp_obj->potenciometro), Amp_obj->potenciometro.resistance);

    return;
}
//...............................................................
extern void EtapaAmp_enable(amplificacion_t *Amp_obj)
{
    float R_pote;

    R_pote = (Amp_obj->amp_vp * R_DAC) / V_DAC;

    X9C103S_set_resistance(&(Amp_obj->potenciometro), R_pote);

    return;
}
//...............................................................
extern void EtapaAmp_disable(amplificacion_t *Amp_obj)
{
    Amp_obj->potenciometro.resistance = POTE_DIGITAL_RESISTENCIA_INICIAL;

    X9C103S_set_resistance(&(Amp_obj->potenciometro), Amp_obj->potenciometro.resistance);

    return;
}
//...............................................................
