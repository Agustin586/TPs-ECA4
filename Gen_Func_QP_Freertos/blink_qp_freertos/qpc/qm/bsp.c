//============================================================================
// Product: Blink example RP2040 board, FreeRTOS kernel
// Last updated for version 7.3.2
// Last updated on  2023-12-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h" // QP/C real-time embedded framework
#include "dpp.h" // DPP Application interface
#include "bsp.h" // Board Support Package
#include "mef_awg.h"

// RP2040 Includes ===========================================================
#include "pico/stdlib.h"
#include <stdio.h>

Q_DEFINE_THIS_FILE // define the name of this file for assertions

// "RTOS-aware" interrupt priorities for FreeRTOS on ARM Cortex-M, NOTE1
#define RTOS_AWARE_ISR_CMSIS_PRI \
    (configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - __NVIC_PRIO_BITS))

    // Freertos declared ========================================================
    void
    init_task(void);
void vTaskBlink(void *pvParameters);

//============================================================================
// Error handler

Q_NORETURN Q_onError(char const *const module, int_t const id)
{
    // NOTE: this implementation of the error handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U); // report assertion to QS

    printf("Failed: Fallo general.\n");

#ifndef NDEBUG
    // light up LED
    // BSP_LED_On(LED1);
    // for debugging, hang on in an endless loop...
    for (;;)
    {
        printf("Failed: q_onError qp.\n");
    }
#else
    // NVIC_SystemReset();
    for (;;)
    { // explicitly "no-return"
    }
#endif
}
//............................................................................
void assert_failed(char const *const module, int_t const id); // prototype
void assert_failed(char const *const module, int_t const id)
{
    Q_onError(module, id);
}

// ISRs used in the application ==============================================

// Application hooks used in this project ====================================
// NOTE: only the "FromISR" API variants are allowed in vApplicationTickHook

void vApplicationTickHook(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // process time events at rate 0
    QTIMEEVT_TICK_FROM_ISR(0U, &xHigherPriorityTaskWoken, &l_TickHook);
    QF_TICK_FROM_ISR(&xHigherPriorityTaskWoken, (void *)0);

    // printf("Check: tick hook.\n");

    // notify FreeRTOS to perform context switch from ISR, if needed
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
//............................................................................
void vApplicationIdleHook(void)
{
    // toggle the User LED on and then off, see NOTE01
    // QF_INT_DISABLE();
    // QF_INT_ENABLE();

    // printf("Check: idle hook.\n");

#ifdef Q_SPY
    QS_rxParse(); // parse all the received bytes

    if ((l_uartHandle.Instance->ISR & UART_FLAG_TXE) != 0U)
    { // TXE empty?
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD)
        {                                             // not End-Of-Data?
            l_uartHandle.Instance->TDR = (b & 0xFFU); // put into TDR
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    //
    // !!!CAUTION!!!
    // The WFI instruction stops the CPU clock, which unfortunately disables
    // the JTAG port, so the ST-Link debugger can no longer connect to the
    // board. For that reason, the call to __WFI() has to be used with CAUTION.
    //
    // NOTE: If you find your board "frozen" like this, strap BOOT0 to VDD and
    // reset the board, then connect with ST-Link Utilities and erase the part.
    // The trick with BOOT(0) is it gets the part to run the System Loader
    // instead of your broken code. When done disconnect BOOT0, and start over.
    //
    //__WFI(); // Wait-For-Interrupt
#endif
}
//............................................................................
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    
    printf("ERROR: Stack overflow en tarea: %s\n", pcTaskName);

    Q_ERROR();
}
//............................................................................
// configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must
// provide an implementation of vApplicationGetIdleTaskMemory() to provide
// the memory that is used by the Idle task.
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    // If the buffers to be provided to the Idle task are declared inside
    // this function then they must be declared static - otherwise they will
    // be allocated on the stack and so not exists after this function exits.
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    // Pass out a pointer to the StaticTask_t structure in which the
    // Idle task's state will be stored.
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    // Pass out the array that will be used as the Idle task's stack.
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    // Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    // Note that, as the array is necessarily of type StackType_t,
    // configMINIMAL_STACK_SIZE is specified in words, not bytes.
    *pulIdleTaskStackSize = Q_DIM(uxIdleTaskStack);
}
//............................................................................
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    // Define variables estáticas para el bloque de control y la pila de la tarea del temporizador
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
//............................................................................
void vApplicationMallocFailedHook(void)
{
    // Aquí puedes manejar el error. Por ejemplo:
    taskDISABLE_INTERRUPTS(); // Deshabilitar interrupciones (opcional)
    for (;;)
    {
        printf("Failed: no memory heap.\n");
        // Bucle infinito para indicar fallo en la asignación de memoria
        // Puedes encender un LED o implementar alguna forma de depuración
    }
}
// BSP functions =============================================================

//............................................................................
void BSP_init(void)
{
    // Acciones de inicializacion de los objetos activos y perifericos que usa
    stdio_init_all();

    // enable clock for to the peripherals used by this application...

    // Configure the LEDs
    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);
}
//............................................................................
void BSP_start(void)
{
    // no need to initialize event pools
    static QF_MPOOL_EL(QEvt) smlPoolSto[10];
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));

    // no need to initialize publish-subscribe
    // QActive_psInit(subscrSto, Q_DIM(subscrSto));

    init_task();

    // Inicializa objeto activo del blink
    static QEvt const *blinkyQueueSto[10];
    static StackType_t blinkyStackSto[256]; // Pila para la tarea (ajusta el tamaño según sea necesario)

    Blinky_ctor();

    QACTIVE_START(AO_Blinky,
                  Q_PRIO(1U, 0U),         // QP prio. of the AO
                  blinkyQueueSto,         // event queue storage
                  Q_DIM(blinkyQueueSto),  // queue length [events]
                  blinkyStackSto,         // stack storage
                  sizeof(blinkyStackSto), // stack size [bytes]
                  (void *)0);             // no initialization param

    // Inicializa objeto activo del awg
    static QEvt const *awgQueueSto[10];
    static StackType_t awgStackSto[2048]; // Pila para la tarea (ajusta el tamaño según sea necesario)

    Awg_ctor();

    QACTIVE_START(AO_Awg,
                  Q_PRIO(2U, 0U),      // QP prio. of the AO
                  awgQueueSto,         // event queue storage
                  Q_DIM(awgQueueSto),  // queue length [events]
                  awgStackSto,         // stack storage
                  sizeof(awgStackSto), // stack size [bytes]
                  (void *)0);          // no initialization param
}
//............................................................................
void BSP_ledOn(void)
{
    // printf("Led ON\n");
    // gpio_put(25,!gpio_get(25));
    // gpio_put(25, 1);
}
//............................................................................
void BSP_ledOff(void)
{
    // printf("Led OFF\n");
    // gpio_put(25,!gpio_get(25));
    // gpio_put(25, 0);
}
//============================================================================

// Freertos callbacks --------------------------------------------------------------
void vTaskBlink(void *pvParameters)
{
    for (;;)
    {
        gpio_put(25, !gpio_get(25));
        vTaskDelay(500 / portTICK_PERIOD_MS); // Espera 500 ms
    }

    vTaskDelete(NULL);
}
//............................................................................
void init_task(void)
{
    xTaskCreate(vTaskBlink,               // Función de la tarea
                "Blink",                  // Nombre de la tarea (para depuración)
                configMINIMAL_STACK_SIZE, // Tamaño de la pila
                NULL,                     // Parámetro para la tarea (no utilizado aquí)
                1,                        // Prioridad de la tarea
                NULL);                    // Handle de la tarea (no utilizado
    return;
}

//============================================================================

// QF callbacks --------------------------------------------------------------

void QF_onStartup(void)
{
    // Acciones antes de comenzar ... --> Habilita interrupciones ...
}
//............................................................................
void QF_onCleanup(void)
{
}
//============================================================================
// NOTE1:
// The configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY constant from the
// FreeRTOS configuration file specifies the highest ISR priority that
// is disabled by the QF framework. The value is suitable for the
// NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY level (i.e.,
// with the numerical values of priorities equal or higher than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are allowed to call any
// QP/FreeRTOS services. These ISRs are "kernel-aware".
//
// Conversely, any ISRs prioritized above the
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY priority level (i.e., with
// the numerical values of priorities less than
// configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) are never disabled and are
// not aware of the kernel. Such "kernel-unaware" ISRs cannot call any
// QP/FreeRTOS services. The only mechanism by which a "kernel-unaware" ISR
// can communicate with the QF framework is by triggering a "kernel-aware"
// ISR, which can post/publish events.
//
// For more information, see article "Running the RTOS on a ARM Cortex-M Core"
// http://www.freertos.org/RTOS-Cortex-M3-M4.html
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invocations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
