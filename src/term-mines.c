#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "term-mines.h"
#include "core/mine-2d.h"
#include "core/mine-graph.h"

void term_mines_start() {
	srand(time(NULL));
	initscr();
	keypad(stdscr, TRUE);
	cbreak();
	noecho();
	nonl();
	// nodelay(stdscr, TRUE);
	if (has_colors()) {
		start_color();
		init_color(11, 1000, 1000, 1000);
		init_color(12, 500, 500, 500);
		init_color(13, 700, 700, 0);
		init_color(14, 1000, 1000, 1000);
		init_color(15, 1000, 0, 0);
		init_color(16, 1000, 1000, 1000);
		init_pair(1, 11, COLOR_BLACK);
		init_pair(2, 12, COLOR_BLACK);
		init_pair(3, 13, COLOR_BLACK);
		init_pair(4, 14, COLOR_BLACK);
		init_pair(5, 15, COLOR_BLACK);
		init_pair(6, 16, COLOR_BLACK);
	}

	_term_state state = {
		.is_running = 1,
		.has_game = 0,
		.lost = 0,
		.cx = 0,
		.cy = 0,
	};
	do {
		if (!state.has_game) {
			term_m_new game_conf = term_mines_new();
			state.game = mine2d_create(game_conf.height, game_conf.width, game_conf.mines, game_conf.kernel);
			state.has_game = 1;
			state.lost = 0;
			term_mines_game(&state);
		}
		int c = getch();
		if (c == 'q') {
			state.is_running = 0;
		} else if (c == KEY_UP) {
			state.cy--;
		} else if (c == KEY_DOWN) {
			state.cy++;
		} else if (c == KEY_LEFT) {
			state.cx--;
		} else if (c == KEY_RIGHT) {
			state.cx++;
		} else if (c == 'f' && state.has_game) {
			mine2d_toggle_flag(&state.game, state.cx, state.cy);
		} else if (c == 'r' && state.has_game) {
			mine2d_set_known(&state.game, state.cx, state.cy);
			if (mine2d_get_state(&state.game, state.cx, state.cy).type == NODE_MINE) {
				state.lost = 1;
			}
		} else if (c == 'n') {
			if (state.has_game) {
				mine2d_free(&state.game);
			}
			term_m_new game_conf = term_mines_new();
			state.game = mine2d_create(game_conf.height, game_conf.width, game_conf.mines, game_conf.kernel);
			state.has_game = 1;	
			state.lost = 0;
		} else {
			continue;
		}
		if (state.cx < 0) {
			state.cx = 0;
		} else if (state.cx >= state.game.width) {
			state.cx = state.game.width - 1;
		}
		if (state.cy < 0) {
			state.cy = 0;
		} else if (state.cy >= state.game.height) {
			state.cy = state.game.height * 1 - 1;
		}
		term_mines_game(&state);
	} while(state.is_running);
	if (state.has_game) {
		mine2d_free(&state.game);
	}
	endwin();
}

term_m_new term_mines_new() {
	term_kernel term_kernels[] = {
		{.name = "basic", .kernel = mine2d_kernel_create(3, 3, 1, 1, (int[]) {
				1, 1, 1,
				1, 0, 1,
				1, 1, 1
				})
		},
		{.name = "5x5 rectangle", .kernel = mine2d_kernel_create(5, 5, 2, 2, (int[]) {
				1, 1, 1, 1, 1,
				1, 0, 0, 0, 1,
				1, 0, 0, 0, 1,
				1, 0, 0, 0, 1,
				1, 1, 1, 1, 1
				})
		},
		{.name = "5x5 ring", .kernel = mine2d_kernel_create(5, 5, 2, 2, (int[]) {
				0, 1, 1, 1, 0,
				1, 1, 1, 1, 1,
				1, 1, 0, 1, 1,
				1, 1, 1, 1, 1,
				0, 1, 1, 1, 0
				})
		}
	};

	int selected_kernel = 0;
	int selected_height = 40;
	int selected_width = 40;
	int selected_mines = 150;
	int selected_field = 0;
	int not_done = 1;
	char line[32];
	do {
		clear();
		move(0, 0);
		sprintf(line, "  height: %d", selected_height);
		addstr(line);
		move(1, 0);
		sprintf(line, "  width:  %d", selected_width);
		addstr(line);
		move(2, 0);
		sprintf(line, "  mines:  %d", selected_mines);
		addstr(line);
		move(3, 0);
		sprintf(line, "  kernel: %s", term_kernels[selected_kernel].name);
		addstr(line);
		for (int ky = 0; ky < term_kernels[selected_kernel].kernel.h; ky++) {
			move(4 + ky, 5);
			for (int kx = 0; kx < term_kernels[selected_kernel].kernel.w; kx++) {
				if (ky == term_kernels[selected_kernel].kernel.oy && kx == term_kernels[selected_kernel].kernel.ox) {
					addstr(" o");
				} else if (term_kernels[selected_kernel].kernel.data[ky * term_kernels[selected_kernel].kernel.w + kx] == 0) {
					addstr("  ");
				} else {
					sprintf(line, "% 2d", term_kernels[selected_kernel].kernel.data[ky * term_kernels[selected_kernel].kernel.w + kx]);
					addstr(line);
				}
			}
		}
		if (selected_field < 4) {
			move(selected_field, 0);
			addch('>');
			move(selected_field, 0);
		}
		int c = getch();
		int delta = 0;
		if (c == 'q') {
			not_done = 0;
		} else if (c == KEY_DOWN) {
			selected_field++;
			if (selected_field > 4)
				selected_field = 4;
		} else if (c == KEY_UP) {
			selected_field--;
			if (selected_field < 0)
				selected_field = 0;
		} else if (c == KEY_LEFT) {
			delta = -1;
		} else if (c == 555) { // CTRL + LEFT
			delta = -10;
		} else if (c == KEY_RIGHT) {
			delta = 1;
		} else if (c == 570) { // CTR: + RIGHT
			delta = 10;
		} else {
			continue;
		}

		if (selected_field == 0) selected_height += delta;
		else if (selected_field == 1) selected_width += delta;
		else if (selected_field == 2) selected_mines += delta;
		else if (selected_field == 3) selected_kernel += delta;

		if (selected_height < 1) selected_height = 1;
		else if (selected_height > 100) selected_height = 100;
		if (selected_width < 1) selected_width = 1;
		else if (selected_width > 100) selected_width = 100;
		if (selected_mines < 0) selected_mines = 0;
		else if (selected_mines > selected_width * selected_height) selected_mines = selected_width * selected_height;
		if (selected_kernel < 0) selected_kernel = (sizeof(term_kernels) / sizeof(term_kernel)) - 1;
		else if (selected_kernel > (sizeof(term_kernels) / sizeof(term_kernel)) - 1) selected_kernel = 0;
	} while (not_done);


	for (int i = 0; i < (sizeof(term_kernels) / sizeof(term_kernel)); i++) {
		if (i == selected_kernel)
			continue;
		mine2d_kernel_free(&term_kernels[i].kernel);
	}
	return (term_m_new) {
		.height = selected_height,
		.width = selected_width,
		.mines = selected_mines,
		.kernel = term_kernels[selected_kernel].kernel,
	};
}

void term_mines_game(_term_state* state) {
	if (state->has_game) {
		int flag_count = 0;
		int unknown_count = 0;
		int exposed_mines = 0;
		for (int y = 0; y < state->game.height; y++) {
			move(y, 0);
			for (int x = 0; x < state->game.width; x++) {
				node_state i_state = mine2d_get_state(&state->game, x, y);
				if (!i_state.is_known && !i_state.is_flag && ((state->lost && i_state.type != NODE_MINE) || (!state->lost))) {
					attrset(COLOR_PAIR(2));
					addstr(" #");
					unknown_count++;
				} else if (i_state.is_flag) {
					if (state->lost && i_state.type == NODE_MINE) {
						attrset(COLOR_PAIR(5));
					} else {
						attrset(COLOR_PAIR(3));
					}
					addstr(" L");
					flag_count++;
				} else if (i_state.type == 0) {
					attrset(COLOR_PAIR(4));
					addstr("  ");
				} else if (i_state.type == NODE_MINE) {
					attrset(COLOR_PAIR(5));
					addstr(" *");
					exposed_mines++;
				} else {
					attrset(COLOR_PAIR(6));
					char buf[8];
					sprintf(buf, "%2d", i_state.type);
					addstr(buf);
				}
			}
			attrset(COLOR_PAIR(1));
			addch('|');
		}
		move(state->game.height, 0);
		for (int x = 0; x < state->game.width; x++) {
			addstr("--");
		}

		move(state->game.height + 1, state->game.width);
		char status_line[64];
		sprintf(status_line, "flags: % 3d, unknown nodes: %-6d", flag_count, unknown_count);
		addstr(status_line);
		if (exposed_mines > 0 || state->lost) {
			move(state->game.height + 3, state->game.width - 6);
			attrset(COLOR_PAIR(5));
			addstr("Mine exposed");
			attrset(COLOR_PAIR(1));
		}

		touchline(stdscr, state->game.height + 1, 2);
		move(state->cy, state->cx * 2 + 1);
	}
	refresh();
}