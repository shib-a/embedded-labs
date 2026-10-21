#ifndef input_h
#define input_h

#include "main.h"

typedef enum {ACTION_NONE, ACTION_LEFT, ACTION_RIGHT, ACTION_ROTATE, ACTION_DROP, ACTION_PAUSE, ACTION_RESTART} action_t;

void input_init(void);
action_t input_poll(void);

#endif
