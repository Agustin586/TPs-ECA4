#ifndef INCLUDE_SENSADO_H_
#define INCLUDE_SENSADO_H_

#include <stdint.h>
#include <stdbool.h>

// #ifdef __cplusplus
// extern "C" {
// #endif

/**
 * @brief Se encarga de inicializar las tareas y perifericos.
 */
extern void Sensado_init(void);
extern uint16_t Sensado_Temp(void);
extern uint16_t Sensado_Humd(void);
extern uint16_t Sensado_Luz(void);
extern bool Sensado_getReleState(void);
extern int Sensado_getPwmState(void);

// #ifdef __cplusplus
// }
// #endif

#endif