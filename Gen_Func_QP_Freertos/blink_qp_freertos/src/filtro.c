#include "filtro.h"

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define FILTRO_OUT_ENABLE 1
#define FILTRO_OUT_DISABLE 0

#define FILTRO_VAL_MEDIO_50_PORCIENTO 1.65

//......................................................................................................
extern void filtroRC_init(filtroRC *filtroRC, uint8_t gpio_pin, uint16_t frecuencia)
{
    // Obtener el slice de PWM del GPIO
    uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
    uint chan = pwm_gpio_to_channel(gpio_pin);

    filtroRC->gpio_pin = gpio_pin;
    filtroRC->freq = frecuencia;
    filtroRC->val_medio = FILTRO_VAL_MEDIO_50_PORCIENTO;
    filtroRC->slice_num = slice_num;
    filtroRC->chan = chan;
    filtroRC->state = FILTRO_OUT_DISABLE;

    // Configurar el GPIO para PWM
    gpio_set_function(gpio_pin, GPIO_FUNC_PWM);

    // Calcular divisor y top
    uint32_t clock_freq = clock_get_hz(clk_sys);           // 125 MHz por defecto
    uint32_t divider16 = clock_freq / (frecuencia * 4096); // Divisor con precisión de 4 bits
    uint32_t top = (clock_freq / (divider16 * frecuencia)) - 1;

    filtroRC->top = top;

    // Configurar el divisor y el TOP
    pwm_set_clkdiv(slice_num, divider16 / 16.0);
    pwm_set_wrap(slice_num, top);

    // Ajustar ciclo de trabajo (Duty Cycle 50%)
    pwm_set_chan_level(slice_num, chan, top / 2);

    return;
}
//......................................................................................................
extern void filtroRC_setValMedio(filtroRC *filtroRC, float val_medio)
{
    float duty = 0.5 * val_medio / 1.65;

    filtroRC->val_medio = val_medio;

    pwm_set_chan_level(filtroRC->slice_num, filtroRC->chan, (uint32_t)filtroRC->top * duty);

    return;
}
//......................................................................................................
extern void filtroRC_enable(filtroRC *filtroRC)
{
    filtroRC->state = FILTRO_OUT_ENABLE;

    pwm_set_enabled(filtroRC->slice_num, filtroRC->state);

    return;
}
//......................................................................................................
extern void filtroRC_disable(filtroRC *filtroRC)
{
    filtroRC->state = FILTRO_OUT_DISABLE;

    pwm_set_enabled(filtroRC->slice_num, filtroRC->state);

    return;
}
//......................................................................................................
