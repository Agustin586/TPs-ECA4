#ifndef AWG_H_
#define AWG_H_

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

#endif