#include "main.h"
#include "tm1637.h"
#include "keyboard.h"

#define CODE_LEN        4
#define MAX_ATTEMPTS    3
#define OPEN_TIME_MS    3000
#define DENIED_TIME_MS  1500
#define LOCK_TIME_MS    10000

#define LED_KEY_PIN     5
#define LED_GREEN_PIN   9
#define LED_RED_PIN     15

#define SEG_BLANK 0x00
#define SEG_DASH  0x40
#define SEG_O     0x3F
#define SEG_P     0x73
#define SEG_E     0x79
#define SEG_n     0x54
#define SEG_r     0x50
#define SEG_L     0x38

volatile uint32_t tickCount;
uint32_t last_display_update;
uint16_t counter;
char lastKey;
uint32_t lastScanTime;

typedef enum {
  ST_INPUT,
  ST_OPEN,
  ST_DENIED,
  ST_LOCKED
} AccessState;

char SECRET_CODE[CODE_LEN] = {'1', '2', '3', '4'};

AccessState state;
char input[CODE_LEN];
uint8_t inputLen;
uint8_t failedAttempts;
uint32_t stateStart;
uint32_t lastBlink;
uint32_t lastShownSeconds;

void osSystickHandler(void) {
  tickCount++;
}


void initGPIO() {
  // Включаем тактирование GPIOA и GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

  GPIOA->MODER = (GPIOA->MODER & ~(3U << (LED_KEY_PIN * 2))) | (1U << (LED_KEY_PIN * 2));
  GPIOA->MODER = (GPIOA->MODER & ~(3U << (LED_GREEN_PIN * 2))) | (1U << (LED_GREEN_PIN * 2));
  GPIOA->MODER = (GPIOA->MODER & ~(3U << (LED_RED_PIN * 2))) | (1U << (LED_RED_PIN * 2));
  GPIOA->OTYPER &= ~((1U << LED_GREEN_PIN) | (1U << LED_RED_PIN));

  GPIOA->BSRR = (1U << (LED_GREEN_PIN + 16)) | (1U << (LED_RED_PIN + 16));
}

void initUSART2() {
  // Включаем тактирование USART2
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // Настраиваем PA2 и PA3 в альтернативный режим
  GPIOA->MODER = (GPIOA->MODER & ~(0xF << 4)) | (0xA << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF << 8)) | (1 << 8) | (1 << 12);

  // Настраиваем USART2
  USART2->BRR = 417; // 48MHz/115200
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void initSysTick() {
  SysTick->LOAD = 47999; // 1ms при 48MHz
  SysTick->VAL = 0;
  SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}

int _write(int file, uint8_t *ptr, int len) {
  for (int i = 0; i < len; i++) {
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ptr[i];
  }
  return len;
}

void ledOn(uint8_t pin)  { 
  GPIOA->BSRR = (1U << pin); 
}

void ledOff(uint8_t pin) { 
  GPIOA->BSRR = (1U << (pin + 16)); 
}
void ledToggle(uint8_t pin) {
  GPIOA->ODR ^= (1U << pin); 
}

void showInput() {
  uint8_t seg[4];
  for (uint8_t i = 0; i < CODE_LEN; i++) {
    seg[i] = (i < inputLen) ? digit_codes[input[i] - '0'] : SEG_DASH;
  }
  tm1637_display_segments(seg);
}

void showOpen() {
  uint8_t seg[4] = {SEG_O, SEG_P, SEG_E, SEG_n};
  tm1637_display_segments(seg);
}

void showError(uint8_t attemptsLeft) {
  uint8_t seg[4] = {SEG_E, SEG_r, SEG_r, digit_codes[attemptsLeft]};
  tm1637_display_segments(seg);
}

void showLocked(uint32_t seconds) {
  uint8_t seg[4] = {SEG_L, SEG_BLANK, SEG_BLANK, digit_codes[seconds % 10]};
  if (seconds >= 10) seg[2] = digit_codes[(seconds / 10) % 10];
  tm1637_display_segments(seg);
}

void enterState(AccessState newState) {
  state = newState;
  stateStart = tickCount;
}

void resetInput() {
  inputLen = 0;
  enterState(ST_INPUT);
  showInput();
}

void checkCode() {
  if (memcmp(input, SECRET_CODE, CODE_LEN) == 0) {
    printf("Access GRANTED\n");
    failedAttempts = 0;
    ledOn(LED_GREEN_PIN);
    showOpen();
    enterState(ST_OPEN);
    return;
  }

  failedAttempts++;
  printf("Access DENIED (%d/%d)\n", failedAttempts, MAX_ATTEMPTS);
  ledOn(LED_RED_PIN);

  if (failedAttempts >= MAX_ATTEMPTS) {
    printf("System LOCKED for %d s\n", LOCK_TIME_MS / 1000);
    enterState(ST_LOCKED);
    lastBlink = tickCount;
    lastShownSeconds = LOCK_TIME_MS / 1000;
    showLocked(lastShownSeconds);
  } else {
    showError(MAX_ATTEMPTS - failedAttempts);
    enterState(ST_DENIED);
  }
}

static void handleKey(char key) {
  if (state != ST_INPUT) return;

  if (key == '*') {
    if (inputLen > 0) inputLen--;
    showInput();
  } else if (key == '#') {
    inputLen = 0;
    showInput();
  } else if (key >= '0' && key <= '9') {
    input[inputLen++] = key;
    showInput();
    if (inputLen == CODE_LEN) checkCode();
  }
}

static void updateAccess() {
  uint32_t elapsed = tickCount - stateStart;

  switch (state) {
    case ST_OPEN:
      if (elapsed >= OPEN_TIME_MS) {
        ledOff(LED_GREEN_PIN);
        resetInput();
      }
      break;
    case ST_DENIED:
      if (elapsed >= DENIED_TIME_MS) {
        ledOff(LED_RED_PIN);
        resetInput();
      }
      break;
    case ST_LOCKED:
      if (elapsed >= LOCK_TIME_MS) {
        printf("System UNLOCKED\n");
        ledOff(LED_RED_PIN);
        failedAttempts = 0;
        resetInput();
        break;
      }
      if (tickCount - lastBlink >= BLINK_MS) {
        lastBlink = tickCount;
        ledToggle(LED_RED_PIN);
      }
      uint32_t seconds = (LOCK_TIME_MS - elapsed + 999) / 1000;
      if (seconds != lastShownSeconds) {
        lastShownSeconds = seconds;
        showLocked(seconds);
      }
      break;
    case ST_INPUT:
      break;
    default:
      break;
  }
}

int main(void) {
  initGPIO();
  initUSART2();
  initSysTick();
  initKeyboard();
  tm1637_init();

  failedAttempts = 0;
  resetInput();

  while (1) {
    char key = scanKeyboard();
    if (key != '\0') handleKey(key);
    updateAccess();
  }

  return 0;
}