#include "FreeRTOS.h"
#include "task.h"
// #include "qpc.h"
#include "pio.h"
#include "pico/stdlib.h"

void task_blink(void *params) {
    // Parpadeo simple del LED usando FreeRTOS
    while(1) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main() {
    // Inicialización del sistema
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // Crear la tarea en FreeRTOS
    xTaskCreate(task_blink, "Blink", 256, NULL, 1, NULL);

    // Iniciar el planificador de FreeRTOS
    vTaskStartScheduler();

    // El control no debería llegar aquí
    while (1) { }
}
