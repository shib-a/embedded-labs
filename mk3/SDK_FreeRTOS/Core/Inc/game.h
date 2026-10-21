#ifndef game_h
#define game_h

#include "input.h"
#include <stdint.h>

#define BLOCK_SIZE 4u
#define SCREEN_W 64u
#define SCREEN_H 128u
#define FIELD_W ((int8_t)10)
#define FIELD_H ((int8_t)(SCREEN_H/BLOCK_SIZE))
#define PANEL_X ((uint8_t)(FIELD_W*BLOCK_SIZE))

typedef enum {SHAPE_T, SHAPE_LIGHTNING, SHAPE_LONG, SHAPE_L, SHAPE_SQUARE, SHAPE_COUNT} shape_t;
typedef enum {STATE_PLAYING, STATE_PAUSED, STATE_OVER, STATE_STARTING} state_t;
typedef enum {UP, RIGHT, DOWN, LEFT} rotation_t;

typedef struct {
	int8_t x;
	int8_t y;
} position_t;

typedef struct {
	shape_t shape;
	rotation_t rotation;
	position_t pos;
	uint8_t is_falling;
	uint16_t fall_timer_ms;
} figure_t;

typedef struct {
	uint8_t field_map[FIELD_H][FIELD_W];
	figure_t current_figure;
	shape_t next_shape;
	uint32_t score;
	state_t state;
	uint8_t needs_render;
} game_t;

void game_init(game_t* g);

void game_act(game_t* g, action_t action);

void game_tick(game_t* game, uint32_t dt_ms);

void render(game_t* g);

#endif
