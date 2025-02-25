#ifndef INCLUDE_FILTRO_H_
#define INCLUDE_FILTRO_H_

#include "pico/stdlib.h"
#include <stdint.h>
#include "config.h"

// Objeto Filtro RC
typedef struct
{
    // Publico:
    uint8_t gpio_pin;
    uint32_t freq;
    bool state;
    float val_medio; // Valor Medio en V

    // Privado:
    uint slice_num;
    uint chan;
    uint32_t top; // Constante (No modificar)
} filtroRC;

/**
 * @brief Configura el filtro RC para que pueda trabajar en el micro
 *
 * @param filtroRC Objeto filtro
 */
extern void filtroRC_init(filtroRC *filtroRC);
/**
 * @brief Setea el valor medio del filtro que se elija
 *
 * @param filtroRC Objeto filtro
 * @param val_medio Valor medio que se quiere a la salida del filtro RC
 */
extern void filtroRC_setValMedio(filtroRC *filtroRC, float val_medio);
/**
 * @brief Habilita el filtro
 *
 * @param filtroRC Objeto filtro
 */
extern void filtroRC_enable(filtroRC *filtroRC);
/**
 * @brief Apaga el filtro RC desde el pwm
 *
 * @param filtroRC Objeto filtro
 */
extern void filtroRC_disable(filtroRC *filtroRC);

#endif // INCLUDE_FILTRO_H