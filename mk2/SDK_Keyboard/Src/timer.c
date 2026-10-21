#include "kb.h"
#include "fonts.h"
#include "oled.h"
#include "timer.h"
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include "buzzer.h"

typedef enum { STATE_INPUT, STATE_RUNNING, STATE_DONE } TimerState;

#define DENOMINATION 1000000000

static TimerState state     = STATE_INPUT;
static uint32_t   counter   = 0;
static uint32_t   lastTick  = 0;
static char       lastKey   = 0;
static uint8_t    needRedraw = 1;
uint32_t megalovania_melody[] = {
		N_D3, N_D3, N_D4, N_A3
};
uint32_t megalovania_delays[] = {
		16, 16, 8, 6
};

static void showTime(uint32_t secs) {
    char buf[12];
    sprintf(buf, "%u", (unsigned)secs);
    oled_SetCursor(25, 27);
    oled_WriteString(buf, Font_7x10, White);
}

void timerRun(void) {
    char key = Scan_Keyboard();

    if (key != lastKey) {
        lastKey = key;
        if (key != 0) {
            switch (state) {
                case STATE_INPUT:
                    if (key >= '0' && key <= '9') {
                        counter = (counter * 10 + (key - '0')) % DENOMINATION;
                        needRedraw = 1;
                    } else if (key == '#' && counter > 0) {
                        state    = STATE_RUNNING;
                        lastTick = HAL_GetTick();
                        needRedraw = 1;
                    } else if (key == '*') {
                        counter  = 0;
                        needRedraw = 1;
                    }
                    break;

                case STATE_RUNNING:
                    if (key == '*') {
                        state    = STATE_INPUT;
                        counter  = 0;
                        needRedraw = 1;
                    }
                    break;

                case STATE_DONE:
                    if (key == '*' || key == '#') {
                        state    = STATE_INPUT;
                        counter  = 0;
                        needRedraw = 1;
                    }
                    break;
            }
        }
    }

    if (state == STATE_RUNNING) {
        uint32_t now = HAL_GetTick();
        if (now - lastTick >= 1000) {
            lastTick += 1000;
            counter--;
            needRedraw = 1;
            if (counter == 0)
                state = STATE_DONE;
        }
    }

    if (needRedraw) {
        needRedraw = 0;
        oled_Fill(Black);
        switch (state) {
            case STATE_INPUT:
                oled_SetCursor(0, 0);
                oled_WriteString("# start  * clear", Font_7x10, White);
                showTime(counter);
                break;
            case STATE_RUNNING:
                showTime(counter);
                break;
            case STATE_DONE:
                oled_SetCursor(30, 27);
                oled_WriteString("DONE!", Font_7x10, White);
                Buzzer_Play(megalovania_melody, megalovania_delays, sizeof(megalovania_melody) / sizeof(uint32_t));
                break;
        }
        oled_UpdateScreen();
        if(state == STATE_DONE){
            Buzzer_Play(megalovania_melody, megalovania_delays, sizeof(megalovania_melody) / sizeof(uint32_t));
        }
    }
}

void OLED_KB(uint8_t OLED_Keys[12]) {
    for (int i = 3; i >= 0; i--) {
        oled_SetCursor(56, 5 + (4 - i) * 10);
        for (int j = 0; j < 3; j++) {
            oled_WriteChar(OLED_Keys[j + 3 * i], Font_7x10, Black);
        }
    }
    oled_UpdateScreen();
}

void oled_Reset(void) {
    oled_Fill(Black);
    oled_SetCursor(0, 0);
    oled_UpdateScreen();
}
