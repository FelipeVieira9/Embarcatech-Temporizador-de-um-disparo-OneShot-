#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define LED_RED 11
#define LED_GREEN 12
#define LED_BLUE 13

#define button 5

static void gpio_irq_handler(uint gpio, uint32_t events);
int64_t alarm_callback(alarm_id_t id, void *user_data);

// Variável para o controle de tempo, para debouncing
static volatile uint32_t last_time = 0;

// Variável para permitir somente uma entrada do botão até finalizar o alarm
static volatile int rodando = 0;

int main() {
    stdio_init_all();

    // Iniciando os leds como saída e o botão como entrada com pull up
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_put(LED_RED, 0);

    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_put(LED_BLUE, 0);

    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);

    // função de interrupção para o botão
    gpio_set_irq_enabled_with_callback(button, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        sleep_ms(250);
    }
}

static void gpio_irq_handler(uint gpio, uint32_t events) {
    // Capturar o tempo atual
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verificar se todos os leds estão desligados para liberar a ação do botão
    if (!gpio_get(LED_RED) && !gpio_get(LED_GREEN) && !gpio_get(LED_BLUE)) {
        rodando = 0;
    }

    if (((current_time - last_time) > 200000) && !rodando){
        rodando = 1;
        last_time = current_time;

        // Liga tods os leds
        gpio_put(LED_RED, 1);
        gpio_put(LED_GREEN, 1);
        gpio_put(LED_BLUE, 1);

        // Alarme para desligar cada led
        add_alarm_in_ms(3000, alarm_callback, NULL, true);
        add_alarm_in_ms(3000 * 2, alarm_callback, NULL, true);
        add_alarm_in_ms(3000 * 3, alarm_callback, NULL, true);

        printf("Alarme enviado\n");
        
    }
}

int64_t alarm_callback(alarm_id_t id, void *user_data) {
    // Verificar qual led está ligado e desligar somente 1 por chamada até o último led
    if (gpio_get(LED_RED)) {
        gpio_put(LED_RED, 0);
        printf("Primeiro led desligado\n");

    } else if (gpio_get(LED_GREEN)) {
        gpio_put(LED_GREEN, 0);
        printf("Segundo led desligado\n");

    } else {
        gpio_put(LED_BLUE, 0);
        printf("Ultimo led desligado, liberado botão\n\n");
    }

    return 0;
}