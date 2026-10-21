#include "game.h"
#include "oled.h"
#include "fonts.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>
#include <string.h>

#define GAME_TICK_THRESHOLD 500
#define PANEL_TEXT_X   (PANEL_X + 2)
#define PANEL_NEXT_Y   2
#define PANEL_PREVIEW_X (PANEL_X + (SCREEN_W - PANEL_X - 4 * BLOCK_SIZE) / 2)
#define PANEL_PREVIEW_Y 15
#define PANEL_SCORE_Y  38
#define PANEL_DIGITS_Y 51
#define PANEL_LINE_H   12
#define DIGITS_PER_LINE 3
#define PHYS_X(gy) (gy)
#define PHYS_Y(gx) (SCREEN_W - 1 - (gx))

void spawn(game_t* game);

static const uint16_t LINE_POINTS[5] = {0, 100, 300, 500, 800};

uint16_t SHAPES[SHAPE_COUNT][4] = {
    [SHAPE_T]         = {0x4E00, 0x4640, 0x0E40, 0x4C40},
    [SHAPE_LIGHTNING] = {0x6C00, 0x4620, 0x06C0, 0x8C40},
    [SHAPE_LONG]      = {0x0F00, 0x2222, 0x00F0, 0x4444},
    [SHAPE_L]         = {0x2E00, 0x4460, 0x0E80, 0xC440},
    [SHAPE_SQUARE]    = {0x6600, 0x6600, 0x6600, 0x6600},
  };

uint8_t shape_cell(shape_t s, rotation_t r, uint8_t col, uint8_t row) {
    return (SHAPES[s][r] >> (15 - (row * 4 + col))) & 1u;
}

uint8_t collides(game_t* game, figure_t f) {
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if (!shape_cell(f.shape, f.rotation, col, row)) continue;
            int8_t fx = f.pos.x + col;
            int8_t fy = f.pos.y + row;
            if (fx < 0 || fx >= FIELD_W || fy >= FIELD_H) return 1;
            if (fy >= 0 && game->field_map[fy][fx]) return 1;
        }
    }
    return 0;
}

void game_init(game_t* game) {
    srand(xTaskGetTickCount());
    memset(game, 0, sizeof(*game));
    game->next_shape = rand() % SHAPE_COUNT;
    spawn(game);
}

void spawn(game_t* game){
    figure_t new_figure;
    new_figure.shape = game->next_shape;
    game->next_shape = rand() % SHAPE_COUNT;
    new_figure.rotation = UP;
    new_figure.pos.x = FIELD_W / 2 - 2;
    new_figure.pos.y = 0;
    new_figure.fall_timer_ms = 0;
    game->current_figure = new_figure;
    game->needs_render = 1;
    if (collides(game, game->current_figure)) {
        for(int row = 0; row < FIELD_H; row++) {
		    for (int col = 0; col < FIELD_W; col++) {
			    game->field_map[row][col] = 0;
		    }
	    }
        game->score = 0;
    }
}

void fix_figure(game_t* game){
    figure_t f = game->current_figure;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if (!shape_cell(f.shape, f.rotation, col, row)) continue;
            int8_t fx = f.pos.x + col;
            int8_t fy = f.pos.y + row;
            if (fy < 0) continue;
            game->field_map[fy][fx] = 1;
        }
    }
}

uint8_t clear_lines(game_t* game){
    uint8_t cleared = 0;
    int8_t dst = FIELD_H - 1;
    for (int8_t src = FIELD_H - 1; src >= 0; src--) {
        uint8_t full = 1;
        for (int8_t col = 0; col < FIELD_W; col++) {
            if (!game->field_map[src][col]) {
                full = 0;
                break;
            }
        }
        if (full) {
            cleared++;
            continue;
        }
        if (dst != src) {
            memcpy(game->field_map[dst], game->field_map[src], sizeof(game->field_map[dst]));
        }
        dst--;
    }
    for (; dst >= 0; dst--) {
        memset(game->field_map[dst], 0, sizeof(game->field_map[dst]));
    }
    return cleared;
}

void land(game_t* game){
    fix_figure(game);
    game->score += LINE_POINTS[clear_lines(game)];
    spawn(game);
}

void game_tick(game_t* game, uint32_t dt_ms){
    if (game->state != STATE_PLAYING) return;
    game->current_figure.fall_timer_ms += dt_ms;
    if (game->current_figure.fall_timer_ms > GAME_TICK_THRESHOLD) {
        figure_t next = game->current_figure;
        next.pos.y += 1;
        next.fall_timer_ms = 0;
        game->current_figure.fall_timer_ms = 0;
        if (!collides(game, next)) {
            game->current_figure = next;
        } else {
            land(game);
        }
        game->needs_render = 1;
    }
}

void handleActionNone(game_t* game){
    return;
}

void handleActionLeft(game_t* game){
    figure_t next = game->current_figure;
    next.pos.x -= 1;
    if (!collides(game, next)) {
      game->current_figure = next;
      game->needs_render = 1;
    }
}

void handleActionRight(game_t* game){
    figure_t next = game->current_figure;
    next.pos.x += 1;
    if (!collides(game, next)) {
      game->current_figure = next;
      game->needs_render = 1;
    }
}

void handleActionRotate(game_t* game){
    figure_t next_f = game->current_figure;
    rotation_t next = (rotation_t)((game->current_figure.rotation + 1) & 3);
    next_f.rotation = next;
    if (!collides(game, next_f)) {
        game->current_figure = next_f;
        game->needs_render = 1;
    }
}

void handleActionDrop(game_t* game){
    figure_t next = game->current_figure;
    next.pos.y += 1;
    while (!collides(game, next)) {
        game->current_figure = next;
        next.pos.y += 1;
    }
    land(game);
    game->needs_render = 1;
}

void handleActionPause(game_t* game){
    if (game->state == STATE_PLAYING) {
        game->state = STATE_PAUSED;
    } else if (game->state == STATE_PAUSED) {
        game->state = STATE_PLAYING;
    }
    game->needs_render = 1;
}

void game_act(game_t* game, action_t action){
    if (action == ACTION_RESTART) {
        game_init(game);
        return;
    }
    if (action == ACTION_PAUSE) {
        handleActionPause(game);
        return;
    }
    if (game->state != STATE_PLAYING) return;
    switch (action)
    {
    case ACTION_ROTATE:
        handleActionRotate(game);
        break;
    case ACTION_LEFT:
        handleActionLeft(game);
        break;
    case ACTION_RIGHT:
        handleActionRight(game);
        break;
    case ACTION_DROP:
        handleActionDrop(game);
        break;
    default:
        break;
    }
};

void draw_block_px(uint8_t gx, uint8_t gy) {
    oled_DrawSquare(PHYS_X(gy), PHYS_X(gy + BLOCK_SIZE - 1),
                    PHYS_Y(gx + BLOCK_SIZE - 1), PHYS_Y(gx), White);
}

void draw_block(int8_t col, int8_t row) {
    draw_block_px(BLOCK_SIZE * col, BLOCK_SIZE * row);
}

void fill_rect_px(uint8_t gx, uint8_t gy, uint8_t w, uint8_t h, OLED_COLOR color) {
    for (uint8_t y = gy; y < gy + h; y++) {
        for (uint8_t x = gx; x < gx + w; x++) {
            oled_DrawPixel(PHYS_X(y), PHYS_Y(x), color);
        }
    }
}

void draw_text(uint8_t gx, uint8_t gy, const char* str) {
    for (; *str; str++, gx += Font_7x10.FontWidth) {
        for (uint8_t i = 0; i < Font_7x10.FontHeight; i++) {
            uint16_t b = Font_7x10.data[(*str - 32) * Font_7x10.FontHeight + i];
            for (uint8_t j = 0; j < Font_7x10.FontWidth; j++) {
                if ((b << j) & 0x8000) oled_DrawPixel(PHYS_X(gy + i), PHYS_Y(gx + j), White);
            }
        }
    }
}

void draw_field(game_t* game){
	for(int row = 0; row < FIELD_H; row++) {
		for (int col = 0; col < FIELD_W; col++) {
			if(game->field_map[row][col]) {
				draw_block(col, row);
			}
		}
	}
}

void draw_current(game_t* game){
    figure_t f = game->current_figure;
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if(shape_cell(f.shape, f.rotation, col, row)) {
                draw_block(f.pos.x + col, f.pos.y + row);
            }
        }
    }
}

void draw_score(uint32_t score){
    char digits[11];
    uint8_t len = 0;
    do {
        digits[len++] = '0' + score % 10;
        score /= 10;
    } while (score);
    uint8_t lines = (len + DIGITS_PER_LINE - 1) / DIGITS_PER_LINE;
    uint8_t pos = len;
    for (uint8_t line = 0; line < lines; line++) {
        uint8_t n = (line == 0) ? len - (lines - 1) * DIGITS_PER_LINE : DIGITS_PER_LINE;
        char buf[DIGITS_PER_LINE + 1];
        for (uint8_t k = 0; k < n; k++) buf[k] = digits[--pos];
        buf[n] = '\0';
        draw_text(PANEL_TEXT_X + (DIGITS_PER_LINE - n) * Font_7x10.FontWidth,
                  PANEL_DIGITS_Y + line * PANEL_LINE_H, buf);
    }
}

void draw_panel(game_t* game){
    oled_DrawHLine(PHYS_X(0), PHYS_X(SCREEN_H - 1), PHYS_Y(PANEL_X), White);
    draw_text(PANEL_TEXT_X, PANEL_NEXT_Y, "NXT");
    for (uint8_t row = 0; row < 4; row++) {
        for (uint8_t col = 0; col < 4; col++) {
            if (shape_cell(game->next_shape, UP, col, row)) {
                draw_block_px(PANEL_PREVIEW_X + BLOCK_SIZE * col, PANEL_PREVIEW_Y + BLOCK_SIZE * row);
            }
        }
    }
    draw_text(PANEL_TEXT_X, PANEL_SCORE_Y, "SCR");
    draw_score(game->score);
}

void draw_pause(void){
    int8_t cx = FIELD_W / 2;
    int8_t cy = FIELD_H / 2;
    fill_rect_px(BLOCK_SIZE * (cx - 3), BLOCK_SIZE * (cy - 3), BLOCK_SIZE * 6, BLOCK_SIZE * 6, Black);
    for (int8_t row = cy - 2; row < cy + 2; row++) {
        draw_block(cx - 2, row);
        draw_block(cx + 1, row);
    }
}

void render(game_t* game){
        oled_Fill(Black);
		draw_field(game);
        draw_current(game);
        draw_panel(game);
        if (game->state == STATE_PAUSED) draw_pause();
        oled_UpdateScreen();
};
