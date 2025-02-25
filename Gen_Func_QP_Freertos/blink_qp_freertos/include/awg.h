#ifndef AWG_H_
#define AWG_H_

#include <stdint.h>

/**
 * @brief Configura el generador de señales.
 */
extern void awg_config(void);
/**
 * @brief Config. tipo de funcion
 * 
 */
extern void awg_Func(void);
/**
 * @brief Config. frecuencia
 * 
 */
extern void awg_Freq(void);
/**
 * @brief Config. amplitud
 * 
 */
extern void awg_Amp(void);
/**
 * @brief Config. offset
 * 
 */
extern void awg_Offset(void);
/**
 * @brief Habilita la señal de salida
 * 
 */
extern void awg_enableOutput(void);
/**
 * @brief Habilita la señal de salida
 * 
 */
extern void awg_start(void);
/**
 * @brief Realiza un blink
 * 
 */
extern void awg_blink(void);
/**
 * @brief 
 * 
 */
extern void awg_ledOff(void);
/**
 * @brief Reinicializa valores del encoder
 * 
 */
extern void awg_resetEnc(void);
/**
 * @brief Reseteo de informacion
 * 
 */
extern void awg_reset(void);
/**
 * @brief Detiene la señal de salida
 * 
 */
extern void awg_stop(void);
extern int awg_reconfig(void);
extern void awg_multiplicador(uint8_t tipo);
extern uint8_t awg_getCurrentState(void);
extern float get_frequency(void); 
extern float get_amplitude(void); 
extern float get_offset(void);
extern int get_funcion(void);

#endif