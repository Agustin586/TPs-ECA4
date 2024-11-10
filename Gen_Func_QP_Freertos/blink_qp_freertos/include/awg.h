#ifndef AWG_H_
#define AWG_H_

#include <stdint.h>

/**
 * @brief Configura el generador de señales.
 */
extern void awg_config(void);
extern void awg_Func(void);
extern void awg_Freq(void);
extern void awg_Amp(void);
extern void awg_Offset(void);
extern void awg_enableOutput(void);
extern void awg_start(void);
extern void awg_blink(void);
extern void awg_ledOff(void);
extern void awg_resetEnc(void);
extern void awg_reset(void);
extern void awg_stop(void);
extern int awg_reconfig(void);
extern void awg_multiplicador(uint8_t tipo);
extern float get_frequency(void); 
extern float get_amplitude(void); 
extern float get_offset(void);

#endif