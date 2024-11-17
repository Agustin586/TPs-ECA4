#ifndef INCLUDE_DISPLAY_H_
#define INCLUDE_DISPLAY_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    BOTON_FUNCTION = 0,
    BOTON_FREQ,
    BOTON_AMP,
    BOTON_OFFSET,
    BOTON_DUTY,
    BOTON_SALIDA,
} Boton_t;

extern void display_init(void);
extern void display_func(void);
extern void display_freq(void);
extern void display_amp(void);
extern void display_offset(void);
extern void display_salida(void);
extern void display_pulsarBoton(Boton_t TipoBoton, bool state);
extern void drawWaveform(uint8_t waveformType, float amplitude, float frequency, float offset);
extern void display_setMultiplicadorText(const char *text);

#endif