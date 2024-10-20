// -------------------------------------------------- //
// This file is autogenerated by pioasm; do not edit! //
// -------------------------------------------------- //

#pragma once

#if !PICO_NO_HARDWARE
#include "hardware/pio.h"
#endif

// ------------ //
// pio_byte_out //
// ------------ //

#define pio_byte_out_wrap_target 0
#define pio_byte_out_wrap 0

static const uint16_t pio_byte_out_program_instructions[] = {
            //     .wrap_target
    0x6008, //  0: out    pins, 8                    
            //     .wrap
};

#if !PICO_NO_HARDWARE
static const struct pio_program pio_byte_out_program = {
    .instructions = pio_byte_out_program_instructions,
    .length = 1,
    .origin = -1,
};

static inline pio_sm_config pio_byte_out_program_get_default_config(uint offset) {
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_wrap(&c, offset + pio_byte_out_wrap_target, offset + pio_byte_out_wrap);
    return c;
}

#include "hardware/clocks.h"
static inline void pio_byte_out_program_init(PIO pio, uint sm, uint offset, uint pin, uint npins, float freq) {
    // Set the pin group GPIO function (connect PIO to the pad)
	// "pin" is start of output pin group, "npins" is the number of pins in the output pin group
    for(uint j=pin; j<(pin+npins); j++) {
		pio_gpio_init(pio, j);
	}
    // Set the pin group's direction to output 
    pio_sm_set_consecutive_pindirs(pio, sm, pin, npins, true);
    pio_sm_config c = pio_byte_out_program_get_default_config(offset);
    // Map the DAC outputs to 'npins' starting at 'pin'
	sm_config_set_out_pins(&c, pin, npins);
	// set the state machine clock rate
	float div = clock_get_hz(clk_sys) / freq ;  // calculates the clock divider
	sm_config_set_clkdiv(&c, div);
	//set up autopull
	sm_config_set_out_shift(&c, true, true, 32);
    // Load our configuration, and jump to the start of the program
    pio_sm_init(pio, sm, offset, &c);
    // Set the state machine running
    pio_sm_set_enabled(pio, sm, true);
}

#endif
