#include "mine-graph.h"

#ifndef _MINE_2D
#define _MINE_2D

typedef struct {
	int height;
	int width;
	mine_graph graph;
} mine_2d;

typedef struct {
	int w;
	int h;
	int ox;
	int oy;
	int* data;
} mine2d_kernel;

mine_2d mine2d_create(int height, int width, int mine_count, mine2d_kernel kernel);
mine2d_kernel mine2d_kernel_create(int height, int width, int oy, int ox, int data[]);
void mine2d_free(mine_2d* inst);
void mine2d_kernel_free(mine2d_kernel* kernel);

void mine2d_set_mine(mine_2d* inst, int x, int y);
void mine2d_toggle_flag(mine_2d* inst, int x, int y);
void mine2d_set_known(mine_2d* inst, int x, int y);
node_state mine2d_get_state(mine_2d* inst, int x, int y);

void mine2d_print(mine_2d* inst);

#endif
