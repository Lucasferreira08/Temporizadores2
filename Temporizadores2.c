#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define LED_BLUE 11
#define LED_RED 12
#define LED_GREEN 13
#define BUTTON 5

typedef enum {
    STATE_OFF,
    STATE_ALL_ON,
    STATE_TWO_ON,
    STATE_ONE_ON
} LedState;

volatile LedState currentState = STATE_OFF;
volatile bool buttonPressed = false;
volatile bool timerRunning = false;

void turn_on_all_leds() {
    gpio_put(LED_BLUE, 1);
    gpio_put(LED_RED, 1);
    gpio_put(LED_GREEN, 1);
}

void turn_off_all_leds() {
    gpio_put(LED_BLUE, 0);
    gpio_put(LED_RED, 0);
    gpio_put(LED_GREEN, 0);
}

void turn_on_two_leds() {
    gpio_put(LED_BLUE, 1);
    gpio_put(LED_RED, 1);
    gpio_put(LED_GREEN, 0);
}

void turn_on_one_led() {
    gpio_put(LED_BLUE, 1);
    gpio_put(LED_RED, 0);
    gpio_put(LED_GREEN, 0);
}

int64_t timer_callback(alarm_id_t id, void *user_data) {
    switch (currentState) {
        case STATE_ALL_ON:
            currentState = STATE_TWO_ON;
            turn_on_two_leds();
            break;
        case STATE_TWO_ON:
            currentState = STATE_ONE_ON;
            turn_on_one_led();
            break;
        case STATE_ONE_ON:
            currentState = STATE_OFF;
            turn_off_all_leds();
            timerRunning = false;
            break;
        default:
            break;
    }

    if (currentState != STATE_OFF) {
        add_alarm_in_ms(3000, timer_callback, NULL, false);
    }

    return 0;
}

void button_callback(uint gpio, uint32_t events) {
    if (!timerRunning) {
        buttonPressed = true;
    }
}

int main() {
    stdio_init_all();

    gpio_init(LED_BLUE);
    gpio_init(LED_RED);
    gpio_init(LED_GREEN);
    gpio_init(BUTTON);

    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_set_dir(BUTTON, GPIO_IN);

    gpio_pull_up(BUTTON);

    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    while (1) {
        if (buttonPressed && !timerRunning) {
            buttonPressed = false;
            timerRunning = true;
            currentState = STATE_ALL_ON;
            turn_on_all_leds();
            add_alarm_in_ms(3000, timer_callback, NULL, false);
        }
        sleep_ms(100); // Debounce delay
    }
}