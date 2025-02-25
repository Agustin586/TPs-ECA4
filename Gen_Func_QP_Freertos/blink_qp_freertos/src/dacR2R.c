#include "dacR2R.h"

#include "hardware/gpio.h"

/**
 * @brief Inicializa los pines del dac
 *
 */
static void init_pins(void);

// FUNCIONES INTERNAS
//.....................................................................
static void init_pins(void)
{
    gpio_init(8);
    gpio_init(9);
    gpio_init(10);
    gpio_init(11);
    gpio_init(12);
    gpio_init(13);
    gpio_init(14);
    gpio_init(15);
    
    gpio_set_dir(8, GPIO_OUT);
    gpio_set_dir(9, GPIO_OUT);
    gpio_set_dir(10, GPIO_OUT);
    gpio_set_dir(11, GPIO_OUT);
    gpio_set_dir(12, GPIO_OUT);
    gpio_set_dir(13, GPIO_OUT);
    gpio_set_dir(14, GPIO_OUT);
    gpio_set_dir(15, GPIO_OUT);
    
    gpio_set_slew_rate(8, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(9, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(10, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(11, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(12, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(13, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(14, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(15, GPIO_SLEW_RATE_FAST);

    return;
}
//.....................................................................

// FUNCIONES EXTERNAS
//.....................................................................
extern void EtapaDacR2R_init(dacR2R_t *dac)
{
    /* Pines */
    init_pins();

    /* Señal */
    signal_init(&(dac->signal));

    return;
}
//.....................................................................
extern void EtapaDacR2R_enable(dacR2R_t *dac)
{
    signal_start(&(dac->signal));

    return;
}
//.....................................................................
extern void EtapaDacR2R_disable(dacR2R_t *dac)
{
    signal_stop(&(dac->signal));

    return;
}
//.....................................................................
extern void dacR2R_setAmplitud(dacR2R_t *dac, float amp)
{
    signal_config_amplitude(&(dac->signal), amp);
}
//.....................................................................
extern void dacR2R_setFreq(dacR2R_t *dac, float freq)
{
    signal_config_freq(&(dac->signal), freq);
}
//.....................................................................
extern void dacR2R_setOffset(dacR2R_t *dac, float offset)
{
    signal_config_offset(&(dac->signal), offset);
}
//.....................................................................
