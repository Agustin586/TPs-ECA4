#include "filtro.h"

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define FILTRO_OUT_ENABLE 1
#define FILTRO_OUT_DISABLE 0

#define FILTRO_VAL_MEDIO_50_PORCIENTO 1.65

//......................................................................................................
extern void filtroRC_init(filtroRC *filtroRC)
{
    // Obtener el slice de PWM del GPIO
    filtroRC->slice_num = pwm_gpio_to_slice_num(filtroRC->gpio_pin);
    filtroRC->chan = pwm_gpio_to_channel(filtroRC->gpio_pin);

    filtroRC->val_medio = FILTRO_VAL_MEDIO_50_PORCIENTO;
    filtroRC->state = FILTRO_OUT_DISABLE;

    // Configurar el GPIO para PWM
    gpio_set_function(filtroRC->gpio_pin, GPIO_FUNC_PWM);

    // Calcular divisor y top
    uint32_t clock_freq = clock_get_hz(clk_sys); // 125 MHz por defecto

    // Inicializar divisor
    float divisor = 1.0;
    filtroRC->top = (clock_freq / (divisor * filtroRC->freq)) - 1;

    // Ajustar el divisor si `top` es mayor que 65535
    while (filtroRC->top > 65535)
    {
        divisor += 1.0;
        filtroRC->top = (clock_freq / (divisor * filtroRC->freq)) - 1;
    }

    // Configurar el divisor y el TOP en el PWM
    pwm_set_clkdiv(filtroRC->slice_num, divisor);
    pwm_set_wrap(filtroRC->slice_num, filtroRC->top);

    // Ajustar ciclo de trabajo (Duty Cycle 50%)
    pwm_set_chan_level(filtroRC->slice_num, filtroRC->chan, (uint32_t)(filtroRC->top / 2.0));
    pwm_set_enabled(filtroRC->slice_num, filtroRC->state);

    return;
}
//......................................................................................................
extern void filtroRC_setValMedio(filtroRC *filtroRC, float val_medio)
{
    float duty = 0.5 * val_medio / 1.65;

    filtroRC->val_medio = val_medio;

    pwm_set_chan_level(filtroRC->slice_num, filtroRC->chan, (uint32_t)(filtroRC->top * duty));

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

    pwm_set_chan_level(filtroRC->slice_num, filtroRC->chan, (uint32_t)(filtroRC->top * 0));
    // pwm_set_enabled(filtroRC->slice_num, filtroRC->state);

    return;
}
//......................................................................................................
