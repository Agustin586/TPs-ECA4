#ifndef INCLUDE_X9C103S_H_
#define INCLUDE_X9C103S_H_

#include <stdint.h>

typedef struct
{
    uint8_t position; // Posición actual del potenciómetro (0-99)
    float resistance; // Resistencia actual en ohmios
} X9C103S;

// Funciones para manejar el X9C103S
extern void X9C103S_init(X9C103S *device);
extern void X9C103S_increase(X9C103S *device, uint8_t target_pos);
extern void X9C103S_decrease(X9C103S *device, uint8_t target_pos);
extern float X9C103S_get_resistance(X9C103S *device);
extern void X9C103S_set_resistance(X9C103S *device, float resistance);
extern void X9C103S_setToZero(X9C103S *device);
extern void X9C103S_incrementar(void);
extern void X9C103S_decrementar(void);

#endif