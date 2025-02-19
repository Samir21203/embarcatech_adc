#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/pwm.h"

// Definições do joystick
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define JOYSTICK_BTN 22
#define BTN_A 5

// Definições do display I2C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// Definições do LED
#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11
#define LED_STEP 200
uint16_t led_level_red = 0;
uint16_t led_level_blue = 0;

// Diretivas para o desenho do quadrado
#define SIZE 8
#define GAP 500 // Zona morta do joystick
#define BASE_STEP 8
#define MAX_STEP 16
uint8_t x = 60;
uint8_t y = 28;

ssd1306_t ssd;

void display_init();
void joystick_init();
void init_button();
void pwm_led_init();

// Callback global para tratar todas as interrupções de GPIO
void gpio_callback(uint gpio, uint32_t events);

// Variáveis globais para controle de estados e debounce
volatile bool pwm_enabled = true;
volatile bool draw_border = false;
volatile bool led_green_state = false;
uint32_t btn_last_pressed = 0;
uint32_t sw_last_pressed = 0;

int main() {
    stdio_init_all();

    display_init();    // Inicializa o display
    joystick_init();   // Inicializa o ADC do joystick
    init_button();     // Inicializa os botões (incluindo o do joystick)
    pwm_led_init();    // Inicializa os LEDs vermelho e azul via PWM

    // Inicializa o LED verde
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_put(LED_GREEN, 0);

    // Registra o callback global para interrupções.
    // Após essa chamada, todas as interrupções usarão o mesmo callback.
    gpio_set_irq_enabled_with_callback(JOYSTICK_BTN, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    // Habilita a interrupção para o BTN_A (usando o callback já registrado)
    gpio_set_irq_enabled(BTN_A, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        // Leitura dos valores do joystick
        adc_select_input(1);
        uint16_t x_value = adc_read();
        adc_select_input(0);
        uint16_t y_value = adc_read();

        // Cálculo do step proporcional ao deslocamento do joystick
        uint16_t step_x = BASE_STEP;
        uint16_t step_y = BASE_STEP;
        if (abs(x_value - 2048) > GAP) {
            step_x = BASE_STEP + ((abs(x_value - 2048) - GAP) * (MAX_STEP - BASE_STEP)) / (2048 - GAP);
        }
        if (abs(y_value - 2048) > GAP) {
            step_y = BASE_STEP + ((abs(y_value - 2048) - GAP) * (MAX_STEP - BASE_STEP)) / (2048 - GAP);
        }
    
        // Se o joystick estiver no centro, reposiciona o quadrado no meio da tela
        if ((x_value >= 2048 - GAP && x_value <= 2048 + GAP) &&
            (y_value >= 2048 - GAP && y_value <= 2048 + GAP)) {
            x = (WIDTH - SIZE) / 2;
            y = (HEIGHT - SIZE) / 2;
        } else {
            // Movimentação em X, sem ultrapassar os limites
            if (x_value < 2048 - GAP && x >= step_x) {
                x -= step_x;  // Move para a esquerda
            } else if (x_value > 2048 + GAP && x <= WIDTH - SIZE - step_x) {
                x += step_x;  // Move para a direita
            }
    
            // Movimentação em Y, sem ultrapassar os limites
            if (y_value < 2048 - GAP && y <= HEIGHT - SIZE - step_y) {
                y += step_y;  // Move para baixo
            } else if (y_value > 2048 + GAP && y >= step_y) {
                y -= step_y;  // Move para cima
            }
        }

        if (pwm_enabled) {
            // Cálculo do brilho dos LEDs baseado na distância do centro
            if (abs(x_value - 2048) > GAP) {
                led_level_red = ((abs(x_value - 2048) - GAP) * 4000) / (2048 - GAP);
            } else {
                led_level_red = 0;
            }
            if (abs(y_value - 2048) > GAP) {
                led_level_blue = ((abs(y_value - 2048) - GAP) * 4000) / (2048 - GAP);
            } else {
                led_level_blue = 0;
            }
        }

        // Limpa o display apenas uma vez
        ssd1306_fill(&ssd, false);

        // Se a variável draw_border estiver ativa, desenha a borda
        if (draw_border) {
            ssd1306_border(&ssd, true);
        }

        // Desenha o quadrado
        ssd1306_draw_square(&ssd, x, y, SIZE, true, true);
        ssd1306_send_data(&ssd);

        // Atualiza os níveis dos LEDs PWM
        pwm_set_gpio_level(LED_RED, led_level_red);
        pwm_set_gpio_level(LED_BLUE, led_level_blue);
    
        sleep_ms(50);
    }
    
    return 0;
}

void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    // Trata o botão do joystick
    if (gpio == JOYSTICK_BTN) {
        // Debounce para o botão do joystick
        if (current_time - sw_last_pressed > 50) {
            // Se a borda for de descida (botão pressionado)
            if (events & GPIO_IRQ_EDGE_FALL) {
                led_green_state = !led_green_state;      // Alterna o estado do LED verde
                gpio_put(LED_GREEN, led_green_state);      // Atualiza o LED verde
                draw_border = !draw_border;                // Alterna o desenho da borda no display
            }
            sw_last_pressed = current_time;
        }
    }
    // Trata o BTN_A (se necessário)
    else if (gpio == BTN_A) {
        // Debounce para o BTN_A
        if (current_time - btn_last_pressed > 80) {
            if (events & GPIO_IRQ_EDGE_FALL) {
                pwm_enabled = !pwm_enabled; // Alterna o estado do PWM
            }
            btn_last_pressed = current_time;
        }
    }
}

void display_init () {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, 0x3C, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_fill(&ssd, false);
}

void joystick_init() {
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
}

void init_button() {
    // Configura o botão BTN_A
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    // Configura o botão do joystick (JOYSTICK_BTN)
    gpio_init(JOYSTICK_BTN);
    gpio_set_dir(JOYSTICK_BTN, GPIO_IN);
    gpio_pull_up(JOYSTICK_BTN);
}

void pwm_led_init() {
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);

    uint slice_red = pwm_gpio_to_slice_num(LED_RED);
    uint slice_blue = pwm_gpio_to_slice_num(LED_BLUE);

    pwm_set_clkdiv(slice_red, 4.0f);
    pwm_set_clkdiv(slice_blue, 4.0f);

    pwm_set_wrap(slice_red, 4000);
    pwm_set_wrap(slice_blue, 4000);

    pwm_set_gpio_level(LED_RED, 0);
    pwm_set_gpio_level(LED_BLUE, 0);

    pwm_set_enabled(slice_red, true);
    pwm_set_enabled(slice_blue, true);
}
