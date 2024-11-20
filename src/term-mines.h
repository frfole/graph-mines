#include "core/mine-2d.h"

#ifndef _TERM_MINES
#define _TERM_MINES

typedef struct {
	int is_running;
	int has_game;
	int lost;
	int cx;
	int cy;
	mine_2d game;
} _term_state;

typedef struct {
	char name[32];
	mine2d_kernel kernel;
} term_kernel;

typedef struct {
	int height;
	int width;
	int mines;
	mine2d_kernel kernel;
} term_m_new;

void term_mines_start();
term_m_new term_mines_new();
void term_mines_game(_term_state* state);

#endif
