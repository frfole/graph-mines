#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "term-mines.h"
#include "core/mine-2d.h"
#include "core/mine-graph.h"

void term_mines_start() {
	// set random seed
	srand(time(NULL));

	// configure curses
	initscr();
	keypad(stdscr, TRUE);
	cbreak();
	noecho();
	nonl();
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

	term_m_new game_conf = {
		.height = 40,
		.width = 40,
		.mines = 150,
		.kernel_idx = 0,
	};
	_term_state state = {
		.is_running = 1,
		.has_game = 0,
		.lost = 0,
		.cx = 0,
		.cy = 0,
	};
	do {
		if (!state.has_game) {
			game_conf = term_mines_new(&game_conf, 0);
			state.game = mine2d_create(game_conf.height, game_conf.width, game_conf.mines, game_conf.kernel);
			state.has_game = 1;
			state.lost = 0;
			term_mines_game(&state, game_conf.mines);
		}
		int c = getch();
		if (c == 'q') state.is_running = 0;
		else if (c == KEY_UP) state.cy--;
		else if (c == KEY_DOWN) state.cy++;
		else if (c == KEY_LEFT) state.cx--;
		else if (c == KEY_RIGHT) state.cx++;
		else if (c == 'f' && state.has_game) mine2d_toggle_flag(&state.game, state.cx, state.cy);
		else if (c == 'r' && state.has_game) {
			mine2d_set_known(&state.game, state.cx, state.cy);
			if (mine2d_get_state(&state.game, state.cx, state.cy).type == NODE_MINE) {
				state.lost = 1;
			}
		} else if (c == 'n' || c == 'g') {
			if (state.has_game) {
				mine2d_free(&state.game);
			}
			game_conf = term_mines_new(&game_conf, c == 'g');
			state.game = mine2d_create(game_conf.height, game_conf.width, game_conf.mines, game_conf.kernel);
			state.has_game = 1;	
			state.lost = 0;
		} else {
			continue;
		}

		if (state.cx < 0) state.cx = 0;
		else if (state.cx >= state.game.width) state.cx = state.game.width - 1;
		if (state.cy < 0) state.cy = 0;
		else if (state.cy >= state.game.height) state.cy = state.game.height * 1 - 1;

		// draw game
		term_mines_game(&state, game_conf.mines);
	} while(state.is_running);

	// freee things we used
	if (state.has_game) {
		mine2d_free(&state.game);
	}
	endwin();
}

term_m_new term_mines_new(term_m_new* prev_conf, int regen) {
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

	int selected_kernel = prev_conf->kernel_idx;
	int selected_height = prev_conf->height;
	int selected_width = prev_conf->width;
	int selected_mines = prev_conf->mines;
	int selected_field = 0;
	char line[32];
	attrset(COLOR_PAIR(1));
	do {
		if (regen)
			break;
		clear();
		// draw all fields and theirs value
		sprintf(line, "  height: %d", selected_height);
		mvaddstr(0, 0, line);
		sprintf(line, "  width:  %d", selected_width);
		mvaddstr(1, 0, line);
		sprintf(line, "  mines:  %d", selected_mines);
		mvaddstr(2, 0, line);
		sprintf(line, "  kernel: %s", term_kernels[selected_kernel].name);
		mvaddstr(3, 0, line);

		// draw selected kernel
		int* kernel_dat = term_kernels[selected_kernel].kernel.data;
		for (int ky = 0; ky < term_kernels[selected_kernel].kernel.h; ky++) {
			move(4 + ky, 5);
			for (int j = 0; j < term_kernels[selected_kernel].kernel.w; j++) {
				if (*kernel_dat == 0) {
					addstr("  ");
				} else {
					sprintf(line, "% 2d", *kernel_dat);
					addstr(line);
				}
				kernel_dat++;
			}
		}
		mvaddch(4 + term_kernels[selected_kernel].kernel.oy, 5 + term_kernels[selected_kernel].kernel.ox * 2 + 1, 'o');
		mvaddch(selected_field, 0, '>');
		move(selected_field, 0);

		// wait for next key
		int c = getch();
		int delta = 0;
		if (c == 'q') break;
		else if (c == KEY_DOWN) selected_field++;
		else if (c == KEY_UP) selected_field--;
		else if (c == KEY_LEFT)	delta = -1;
		else if (c == 555) delta = -10; // CTRL + LEFT
		else if (c == KEY_RIGHT) delta = 1;
		else if (c == 570) delta = 10; // CTR: + RIGHT
		else continue;

		if (selected_field < 0) selected_field = 0;
		else if (selected_field == 0) selected_height += delta;
		else if (selected_field == 1) selected_width += delta;
		else if (selected_field == 2) selected_mines += delta;
		else if (selected_field == 3) selected_kernel += delta;
		else if (selected_field > 4) selected_field = 4;

		if (selected_height < 1) selected_height = 1;
		else if (selected_height > 100) selected_height = 100;
		if (selected_width < 1) selected_width = 1;
		else if (selected_width > 100) selected_width = 100;
		if (selected_mines < 0) selected_mines = 0;
		else if (selected_mines > selected_width * selected_height) selected_mines = selected_width * selected_height;
		if (selected_kernel < 0) selected_kernel = (sizeof(term_kernels) / sizeof(term_kernel)) - 1;
		else if (selected_kernel > (sizeof(term_kernels) / sizeof(term_kernel)) - 1) selected_kernel = 0;
	} while (1);

	// free kernels, we are not going to use
	for (int i = 0; i < (sizeof(term_kernels) / sizeof(term_kernel)); i++) {
		if (i == selected_kernel)
			continue;
		mine2d_kernel_free(&term_kernels[i].kernel);
	}

	return (term_m_new) {
		.height = selected_height,
		.width = selected_width,
		.mines = selected_mines,
		.kernel_idx = selected_kernel,
		.kernel = term_kernels[selected_kernel].kernel,
	};
}

void term_mines_game(_term_state* state, int mines) {
	if (state->has_game) {
		clear();
		int flag_count = 0;
		int unknown_count = 0;
		int exposed_mines = 0;
		char buf[64];
		// draw game field
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
					sprintf(buf, "%2d", i_state.type);
					addstr(buf);
				}
			}
			attrset(COLOR_PAIR(1));
			addch('|');
		}
		// draw bottom border
		move(state->game.height, 0);
		for (int x = 0; x < state->game.width; x++) {
			addstr("--");
		}

		// draw game status
		sprintf(buf, "flags: % 3d, unknown nodes: %-6d", flag_count, unknown_count);
		mvaddstr(state->game.height + 1, state->game.width, buf);
		if (exposed_mines > 0 || state->lost) {
			attrset(COLOR_PAIR(5));
			mvaddstr(state->game.height + 3, state->game.width - 6, "Mine exposed");
		} else if (flag_count + unknown_count == mines) {
			attrset(COLOR_PAIR(3));
			mvaddstr(state->game.height + 3, state->game.width - 3, "YOU WON");
		}

		// move cursor to current field
		move(state->cy, state->cx * 2 + 1);
		refresh();
	}
}
