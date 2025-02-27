#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

// Conversion de amplitud
#define AMP_MAX_CUENTAS 127
#define AMP_MIN_CUENTAS 0
#define VOLTAJE_MAX 1.65
#define VOLTAJE_MIN 0
#define AMPLITUD_EN_VOLTAJE(x) (x * AMP_MAX_CUENTAS / VOLTAJE_MAX)
// Parametros maximos
#define FREQ_MAX 1500000
#define FREQ_MIN 1
#define AMPLITUD_SALIDA_MAX 5
#define AMPLITUD_SALIDA_MIN 1
#define OFFSET_SALIDA_MAX 5
#define OFFSET_SALIDA_MIN -5
// Conversion de offset
#define OFFSET_MAX_CUENTAS 127
#define OFFSET_MIN_CUENTAS 0
#define OFFSET_EN_VOLTAJE(X) (X * OFFSET_MAX_CUENTAS / VOLTAJE_MAX)
// Configuraciones iniciales de la señal
#define SIGNAL_INIT_TYPE SIGNAL_SINE
#define SIGNAL_INIT_AMP AMPLITUD_SALIDA_MIN
#define SIGNAL_INIT_OFFSET 0
#define SIGNAL_INIT_FREQ 1000
#define SIGNAL_INIT_DUTY 0.5
#define SIGNAL_INIT_POLARITY -1
// Sentido encoder
#define SENTIDO_HORARIO 1
#define SENTIDO_ANTIHORARIO -1
// Pines de PWM
#define DESACOPLE_GPIO 18 // GPIO para el PWM
#define OFFSET_POST 16
#define OFFSET_NEGT 17
#define PWM_FREQ 80000    // Frecuencia en Hz

// Definimos los multiplicadores de frecuencia
typedef enum
{
    MULT_FREQ_X_1 = 0,
    MULT_FREQ_X_10,
    MULT_FREQ_X_100,
    MULT_FREQ_X_1000,
    MULT_FREQ_X_10000,
    MULT_FREQ_X_100000,
    MULT_FREQ_X_1000000,
} MultFreq_t;

// Definimos los multiplicadores de amplitud
typedef enum
{
    MULT_AMP_X_1 = 0,
    MULT_AMP_X_0_1,
    MULT_AMP_X_0_01,
} MultAmp_t;

typedef enum
{
    MULT_OFFSET_X_1 = 0,
    MULT_OFFSET_X_0_1,
    MULT_OFFSET_X_0_01,
} MultOffset_t;

#endif