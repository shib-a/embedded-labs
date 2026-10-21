#include "input.h"
#include "kb.h"
#include <stdint.h>

void input_init(){
    Set_Keyboard();
}

action_t input_get(char in){
    action_t action;
    switch (in)
    {
    case '8':
        action = ACTION_LEFT;
        break;
    case '2':
        action = ACTION_RIGHT;
        break;
    case '5':
        action = ACTION_ROTATE;
        break;
    case '6':
        action = ACTION_DROP;
        break;
    case '*':
        action = ACTION_PAUSE;
        break;
    case '#':
        action = ACTION_RESTART;
        break;
    default:
        action = ACTION_NONE;
        break;
    }
    return action;
}

action_t input_poll() {
    static char prev = 0;
    char key = Scan_Keyboard();
    action_t a = (key != prev) ? input_get(key) : ACTION_NONE;
    prev = key;
    return a;
}
