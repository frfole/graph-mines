#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mine-2d.h"
#include "mine-graph.h"

mine_2d mine2d_create(int height, int width, int mine_count, mine2d_kernel kernel) {
	mine_2d inst = {
		.height = height,
		.width = width,
		.graph = graph_create(height * width),
	};
	for (int y = 0; y < inst.height; y++) {
		for (int x = 0; x < inst.width; x++) {
			for (int dy = 0; dy < kernel.h; dy++) {
				for (int dx = 0; dx < kernel.w; dx++) {
					if (y + dy - kernel.oy < 0 || y + dy - kernel.oy >= inst.height) continue;
					if (x + dx - kernel.ox < 0 || x + dx - kernel.ox >= inst.width) continue;
					int kdata = kernel.data[dy * kernel.w + dx];
					if (kdata == 1) {
						graph_add_edge(&inst.graph, LAYER_INPUT, y * inst.width + x, (y + dy - kernel.oy) * inst.width + x + dx - kernel.ox, EDGE_WEAK);
					}
				}
			}
		}
		continue;
		for (int dy = -1; dy < 2; dy++) {
			for (int dx = -1; dx < 2; dx++) {
				if (dx == 0 && dy == 0) continue;
				if (y + dy < 0 || y + dy >= inst.height) continue;
				graph_add_biedge(&inst.graph, LAYER_INPUT, y * inst.width, (y + dy) * inst.width + inst.width - 1, EDGE_WEAK);
			}
		}
	}
	mine2d_kernel_free(&kernel);
	graph_gen_mines(&inst.graph, mine_count);
	graph_build(&inst.graph);
	return inst;
}

mine2d_kernel mine2d_kernel_create(int height, int width, int oy, int ox, int data[]) {
	mine2d_kernel kernel = {
		.h = height, .w = width,
		.oy = oy, .ox = ox,
		.data = malloc(height * width * sizeof(int))
	};
	memcpy(kernel.data, data, kernel.w * kernel.h * sizeof(int));
	return kernel;
}

void mine2d_free(mine_2d* inst) {
	graph_free(&inst->graph);
}

void mine2d_kernel_free(mine2d_kernel* kernel) {
	free(kernel->data);
}

void mine2d_set_mine(mine_2d* inst, int x, int y) {
	graph_set_mine(&inst->graph, y * inst->width + x);
}

void mine2d_toggle_flag(mine_2d* inst, int x, int y) {
	graph_toggle_flag(&inst->graph, y * inst->width + x);
}

void mine2d_set_known(mine_2d* inst, int x, int y) {
	graph_set_known(&inst->graph, y * inst->width + x);
}

node_state mine2d_get_state(mine_2d* inst, int x, int y) {
	return graph_input_state(&inst->graph, y * inst->width + x);
}

void mine2d_print(mine_2d* inst) {
	for (int y = inst->height - 1; y >= 0; y--) {
		printf("%2d|", y);
		for (int x = 0; x < inst->width; x++) {
			node_state state = graph_input_state(&inst->graph, y * inst->width + x);
			if (!state.is_known && !state.is_flag) {
				printf(" # ");
			} else if (state.is_flag) {
				printf(" L ");
			} else if (state.type == 0) {
				printf("   ");
			} else {
				printf("  %d", state.type);
			}
		}
		printf("\n");
	}
	printf("   ");
	for (int x = 0; x < inst->width; x++) {
		printf("-|-");
	}
	printf("\n");
	printf("   ");
	for (int x = 0; x < inst->width; x++) {
		printf("%2d ", x);
	}
	printf("\n");
}
