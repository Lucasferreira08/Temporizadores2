#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// Definição dos GPIOs para os LEDs e o botão
#define LED_BLUE 11    // GPIO para o LED azul
#define LED_RED 12     // GPIO para o LED vermelho
#define LED_GREEN 13   // GPIO para o LED verde
#define BUTTON 5       // GPIO para o botão

// Enumeração para os estados dos LEDs
typedef enum {
    STATE_OFF,      // Todos os LEDs desligados
    STATE_ALL_ON,   // Todos os LEDs ligados
    STATE_TWO_ON,   // Dois LEDs ligados
    STATE_ONE_ON    // Um LED ligado
} LedState;

// Variáveis globais para controle de estado e temporização
volatile LedState currentState = STATE_OFF;  // Estado atual dos LEDs
volatile bool buttonPressed = false;         // Indica se o botão foi pressionado
volatile bool timerRunning = false;          // Indica se o temporizador está em execução

// Função para ligar todos os LEDs
void turn_on_all_leds() {
    gpio_put(LED_BLUE, 1);   // Liga o LED azul
    gpio_put(LED_RED, 1);    // Liga o LED vermelho
    gpio_put(LED_GREEN, 1);  // Liga o LED verde
}

// Função para desligar todos os LEDs
void turn_off_all_leds() {
    gpio_put(LED_BLUE, 0);   // Desliga o LED azul
    gpio_put(LED_RED, 0);    // Desliga o LED vermelho
    gpio_put(LED_GREEN, 0);  // Desliga o LED verde
}

// Função para ligar dois LEDs (vermelho e verde)
void turn_on_two_leds() {
    gpio_put(LED_BLUE, 0);   // Desliga o LED azul
    gpio_put(LED_RED, 1);    // Liga o LED vermelho
    gpio_put(LED_GREEN, 1);  // Liga o LED verde
}

// Função para ligar apenas um LED (verde)
void turn_on_one_led() {
    gpio_put(LED_BLUE, 0);   // Desliga o LED azul
    gpio_put(LED_RED, 0);    // Desliga o LED vermelho
    gpio_put(LED_GREEN, 1);  // Liga o LED verde
}

// Callback do temporizador: chamado a cada 3 segundos
int64_t timer_callback(alarm_id_t id, void *user_data) {
    switch (currentState) {
        case STATE_ALL_ON:
            currentState = STATE_TWO_ON;  // Muda para o estado de dois LEDs ligados
            turn_on_two_leds();           // Liga dois LEDs
            break;
        case STATE_TWO_ON:
            currentState = STATE_ONE_ON;  // Muda para o estado de um LED ligado
            turn_on_one_led();            // Liga um LED
            break;
        case STATE_ONE_ON:
            currentState = STATE_OFF;     // Muda para o estado de todos desligados
            turn_off_all_leds();          // Desliga todos os LEDs
            timerRunning = false;         // Indica que o temporizador parou
            break;
        default:
            break;
    }

    // Se o estado não for OFF, reconfigura o temporizador para chamar a função novamente após 3 segundos
    if (currentState != STATE_OFF) {
        add_alarm_in_ms(3000, timer_callback, NULL, false);
    }

    return 0;
}

// Callback do botão: chamado quando o botão é pressionado
void button_callback(uint gpio, uint32_t events) {
    if (!timerRunning) {  // Verifica se o temporizador não está em execução
        buttonPressed = true;  // Indica que o botão foi pressionado
    }
}

// Função principal
int main() {
    stdio_init_all();  // Inicializa a comunicação serial (para depuração)

    // Inicializa os GPIOs dos LEDs e do botão
    gpio_init(LED_BLUE);
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(BUTTON);

    // Configura os GPIOs dos LEDs como saída
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    // Configura o GPIO do botão como entrada com pull-up
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    // Configura a interrupção do botão para chamar button_callback quando o botão for pressionado
    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Loop principal
    while (1) {
        // Verifica se o botão foi pressionado e se o temporizador não está em execução
        if (buttonPressed && !timerRunning) {
            buttonPressed = false;  // Reseta a flag do botão
            timerRunning = true;    // Indica que o temporizador está em execução
            currentState = STATE_ALL_ON;  // Define o estado inicial como todos os LEDs ligados
            turn_on_all_leds();           // Liga todos os LEDs
            add_alarm_in_ms(3000, timer_callback, NULL, false);  // Configura o temporizador para 3 segundos
        }
        sleep_ms(100);  // Pequeno atraso para debounce do botão
    }
}