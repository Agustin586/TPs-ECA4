#include <stdio.h>
#include <math.h>
#include "awg.h"
#include "pico/stdlib.h"
#include "pico/stdlib.h"

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include "mef_awg.h"
#include "display.h"
#include "dacR2R.h"
#include "periferico.h"
#include "amplificacion.h"
#include "offset.h"
#include "config.h"

// Objeto tipo generador de funciones
typedef struct
{
    dacR2R_t EtapaDac;
    amplificacion_t EtapaAmp;
    offset_t EtapaOffset;
    periferico_t DatosPerifericos;
} awg;

// Generamos el objeto señal
awg Gen_Funcion;
// SignalGenerator signal_ch1;

/**
 * @brief Inicializa las tareas de freertos.
 */
static void init_task(void);
/**
 * @brief Tarea de awg de freertos.
 */
static void prvTaskRtos_awg(void *pvParameters);

// FUNCIONES EXTERNAS
/*-------------------------------------------------------------------------------------------*/
extern void awg_config(void)
{
    /* Crea las tareas. */
    init_task();

    /* Inicializa la etapa de Perifericos */
    EtapaPerf_init();

    /* Inicializa la etapa de DAC */
    EtapaDacR2R_init(&Gen_Funcion.EtapaDac);

    /* Inicialzia la etapa de amplificacion */
    EtapaAmp_init(&Gen_Funcion.EtapaAmp);

    /* Inicializa la etapa de offset */
    Gen_Funcion.EtapaOffset.Desacople.gpio_pin = DESACOPLE_GPIO;
    EtapaOffset_init(&Gen_Funcion.EtapaOffset);

    /* Configuracion display. */
    display_init();

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Func(void)
{
    // Selecciona funcion
    if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_HORARIO)
    {
        if (Gen_Funcion.EtapaDac.signal.type != SIGNAL_SAWTOOTH)
            Gen_Funcion.EtapaDac.signal.type += 1;
    }
    else if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_ANTIHORARIO)
    {
        if (Gen_Funcion.EtapaDac.signal.type != SIGNAL_SINE)
            Gen_Funcion.EtapaDac.signal.type -= 1;
    }

    // Display
    switch (Gen_Funcion.EtapaDac.signal.type)
    {
    case SIGNAL_SINE:
        drawWaveform(SIGNAL_SINE, 10, 1000.0, 0);
        break;
    case SIGNAL_SQUARE:
        drawWaveform(SIGNAL_SQUARE, 10, 1000.0, 0);
        break;
    case SIGNAL_TRIANGLE:
        drawWaveform(SIGNAL_TRIANGLE, 10, 1000.0, 0);
        break;
    case SIGNAL_SAWTOOTH:
        // printf("Tipo de señal: diente de sierra.\n");
        break;
    default:
        // printf("Tipo de señal: desconocida.\n");
        break;
    }

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Freq(void)
{
    float freq_new = Gen_Funcion.EtapaDac.signal.frequency;

    // Cambia la frecuencia
    if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_HORARIO)
    {
        if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1)
            freq_new += 1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_10)
            freq_new += 10;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_100)
            freq_new += 100;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1000)
            freq_new += 1000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_10000)
            freq_new += 10000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_100000)
            freq_new += 100000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1000000)
            freq_new += 1000000;
    }
    else if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_ANTIHORARIO)
    {
        if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1)
            freq_new -= 1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_10)
            freq_new -= 10;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_100)
            freq_new -= 100;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1000)
            freq_new -= 1000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_10000)
            freq_new -= 10000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_100000)
            freq_new -= 100000;
        else if (Gen_Funcion.DatosPerifericos.encoder.multFreq == MULT_FREQ_X_1000000)
            freq_new -= 1000000;
    }

    // Limites
    if (freq_new > FREQ_MAX)
        freq_new = Gen_Funcion.EtapaDac.signal.frequency; // No carga nuevo dato
    else if (freq_new < FREQ_MIN)
        freq_new = Gen_Funcion.EtapaDac.signal.frequency; // No carga nuevo dato

    // Carga la informacion en la señal
    dacR2R_setFreq(&Gen_Funcion.EtapaDac, freq_new);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Amp(void)
{
    float amp_new = Gen_Funcion.EtapaAmp.amp_vp;

    // Detecta el evento de amplitud
    if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_HORARIO)
    {
        if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_1)
            amp_new += 1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_0_1)
            amp_new += 0.1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_0_01)
            amp_new += 0.01;
    }
    else if (Gen_Funcion.DatosPerifericos.encoder.direccion == SENTIDO_ANTIHORARIO)
    {
        if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_1)
            amp_new -= 1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_0_1)
            amp_new -= 0.1;
        else if (Gen_Funcion.DatosPerifericos.encoder.multAmpl == MULT_AMP_X_0_01)
            amp_new -= 0.01;
    }

    // Limites
    if (amp_new > AMPLITUD_SALIDA_MAX)
        amp_new = Gen_Funcion.EtapaAmp.amp_vp;
    else if (amp_new < AMPLITUD_SALIDA_MIN)
        amp_new = Gen_Funcion.EtapaAmp.amp_vp;

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_Offset(void)
{
    // float offset_new = signal_ch1.offset;

    // Detecta el evento del offset
    // if (selector.direccion == SENTIDO_HORARIO)
    // {
    //     // if (selector.multOffset == MULT_OFFSET_X_1)
    //     //     offset_new += 1;
    //     // else if (selector.multOffset == MULT_OFFSET_X_0_1)
    //     //     offset_new += 0.1;
    //     // else if (selector.multOffset == MULT_OFFSET_X_0_01)
    //     //     offset_new += 0.01;
    // }
    // else if (selector.direccion == SENTIDO_ANTIHORARIO)
    // {
    //     // if (selector.multOffset == MULT_OFFSET_X_1)
    //     //     offset_new -= 1;
    //     // else if (selector.multOffset == MULT_OFFSET_X_0_1)
    //     //     offset_new -= 0.1;
    //     // else if (selector.multOffset == MULT_OFFSET_X_0_01)
    //     //     offset_new -= 0.01;
    // }

    // Limites
    // if (offset_new > OFFSET_SALIDA_MAX)
    //     offset_new = OFFSET_SALIDA_MAX;
    // else if (offset_new < OFFSET_SALIDA_MIN)
    //     offset_new = OFFSET_SALIDA_MIN;

    // Carga la informacion
    // signal_config_offset(&signal_ch1, offset_new);

    // Cargamos la informacion en la pantalla
    // display_offset();

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_enableOutput(void)
{
    Gen_Funcion.EtapaDac.signal.state_out = !Gen_Funcion.EtapaDac.signal.state_out;

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_start(void)
{
    /* Habilito la etapa de señal */
    EtapaDacR2R_enable(&Gen_Funcion.EtapaDac);

    /* Habilito la etapa de amplificacion */
    EtapaAmp_enable(&Gen_Funcion.EtapaAmp);

    /* Habilito la etapa de offset */
    EtapaOffset_setDesacople(&Gen_Funcion.EtapaOffset);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_blink(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_ledOff(void)
{

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_resetEnc(void)
{
    /* Configuracion del selector. */
    // Gen_Funcion.cont = 0;
    // selector.direccion = SENTIDO_HORARIO;

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_reset(void)
{
    // signal_ch1.state_out = false;

    setEvt_init();

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_stop(void)
{
    // Reseteo el multiplicador del selector
    periferico_ResetMultiplicadores(&Gen_Funcion.DatosPerifericos);

    /* Deshabilito la etapa de dac */
    EtapaDacR2R_disable(&Gen_Funcion.EtapaDac);

    /* Deshabilito la etapa de amplificacion */
    EtapaAmp_disable(&Gen_Funcion.EtapaAmp);

    /* Deshabilito la etapa de offset */
    EtapaOffset_clearDesacople(&Gen_Funcion.EtapaOffset);

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern int awg_reconfig(void)
{
    return Gen_Funcion.EtapaDac.signal.state_out ? true : false;
}
/*-------------------------------------------------------------------------------------------*/
extern void awg_multiplicador(uint8_t tipo)
{
    switch (tipo)
    {
    case MULTIPLICADOR_FREQ:
        Gen_Funcion.DatosPerifericos.encoder.multFreq += 1;
        if (Gen_Funcion.DatosPerifericos.encoder.multFreq > MULT_FREQ_X_1000000)
            Gen_Funcion.DatosPerifericos.encoder.multFreq = MULT_FREQ_X_1;
        switch (Gen_Funcion.DatosPerifericos.encoder.multFreq)
        {
        case MULT_FREQ_X_1:
            display_setMultiplicadorText("x1 Hz");
            break;
        case MULT_FREQ_X_10:
            display_setMultiplicadorText("x10 Hz");
            break;
        case MULT_FREQ_X_100:
            display_setMultiplicadorText("x100 Hz");
            break;
        case MULT_FREQ_X_1000:
            display_setMultiplicadorText("x1 KHz");
            break;
        case MULT_FREQ_X_10000:
            display_setMultiplicadorText("x10 KHz");
            break;
        case MULT_FREQ_X_100000:
            display_setMultiplicadorText("x100 KHz");
            break;
        case MULT_FREQ_X_1000000:
            display_setMultiplicadorText("x1 MHz");
            break;
        default:
            break;
        }

        break;
    case MULTIPLICADOR_AMP:
        Gen_Funcion.DatosPerifericos.encoder.multAmpl += 1;
        if (Gen_Funcion.DatosPerifericos.encoder.multAmpl > MULT_AMP_X_0_01)
            Gen_Funcion.DatosPerifericos.encoder.multAmpl = MULT_AMP_X_1;

        switch (Gen_Funcion.DatosPerifericos.encoder.multAmpl)
        {
        case MULT_AMP_X_1:
            display_setMultiplicadorText("x1 Vp");
            break;
        case MULT_AMP_X_0_1:
            display_setMultiplicadorText("x0.1 Vp");
            break;
        case MULT_AMP_X_0_01:
            display_setMultiplicadorText("x0.01 Vp");
            break;
        default:
            break;
        }

        break;
    case MULTIPLICADOR_OFFSET:
        Gen_Funcion.DatosPerifericos.encoder.multOffset += 1;
        if (Gen_Funcion.DatosPerifericos.encoder.multOffset > MULT_AMP_X_0_01)
            Gen_Funcion.DatosPerifericos.encoder.multOffset = MULT_AMP_X_1;

        switch (Gen_Funcion.DatosPerifericos.encoder.multOffset)
        {
        case MULT_AMP_X_1:
            display_setMultiplicadorText("x1 Vp");
            break;
        case MULT_AMP_X_0_1:
            display_setMultiplicadorText("x0.1 Vp");
            break;
        case MULT_AMP_X_0_01:
            display_setMultiplicadorText("x0.01 Vp");
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }

    return;
}
/*-------------------------------------------------------------------------------------------*/
extern uint8_t awg_getCurrentState(void)
{
    return getCurrentState();
}
/*-------------------------------------------------------------------------------------------*/
extern float get_frequency(void)
{
    return Gen_Funcion.EtapaDac.signal.frequency;
}
/*-------------------------------------------------------------------------------------------*/
extern float get_amplitude(void)
{
    return Gen_Funcion.EtapaDac.signal.amplitude;
}
/*-------------------------------------------------------------------------------------------*/
extern float get_offset(void)
{
    return Gen_Funcion.EtapaDac.signal.offset;
}
/*-------------------------------------------------------------------------------------------*/
extern int get_funcion(void)
{
    return Gen_Funcion.EtapaDac.signal.type;
}
/*-------------------------------------------------------------------------------------------*/

// FUNCIONES PRIVADAS
/*-------------------------------------------------------------------------------------------*/
static void prvTaskRtos_awg(void *pvParameters)
{
    for (;;)
    {
        if (periferico_QueueReceive(&Gen_Funcion.DatosPerifericos))
        {
            if (Gen_Funcion.DatosPerifericos.evt_atras)
                setEvt_Atras();
            if (Gen_Funcion.DatosPerifericos.evt_avance)
                setEvt_Avanc();
            if (Gen_Funcion.DatosPerifericos.evt_confr)
                setEvt_Confirm();
            if (Gen_Funcion.DatosPerifericos.evt_mult)
                setEvt_Multiplicador();
            if (Gen_Funcion.DatosPerifericos.evt_stop)
                setEvtStop();
            if (Gen_Funcion.DatosPerifericos.evt_encoder)
                setEvt_Econder();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);

    return;
}
/*-------------------------------------------------------------------------------------------*/
#define TASKRTOS_AWG_PRIORITY tskIDLE_PRIORITY + 1
static void init_task(void)
{
    BaseType_t status = 0;

    status = xTaskCreate(prvTaskRtos_awg, "Task awg", configMINIMAL_STACK_SIZE, NULL, TASKRTOS_AWG_PRIORITY, NULL);

    configASSERT(status == NULL); // Detiene el programa si no se crea correctamente

    return;
}
/*-------------------------------------------------------------------------------------------*/
// X9C103S_set_resistance(&potenciometro, resistencia);
// if (!getState_Pulsador(MULTIPLICADOR_PIN))
// {
//     X9C103S_incrementar();
//     while (!getState_Pulsador(MULTIPLICADOR_PIN))
//     {
//         vTaskDelay(pdMS_TO_TICKS(30));
//     }
// }
// if (!getState_Pulsador(ATRAS_PIN))
// {
//     X9C103S_decrementar();
//     while (!getState_Pulsador(ATRAS_PIN))
//     {
//         vTaskDelay(pdMS_TO_TICKS(30));
//     }
// }