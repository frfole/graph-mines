#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/mine-graph.h"
#include "core/mine-2d.h"
#include "term-mines.h"

int main() {
	term_mines_start();
	return 0;
	return 0;
	mine_graph graph = graph_create(6);

	graph_add_biedge(&graph, LAYER_INPUT, 0, 1, EDGE_WEAK);
	graph_add_biedge(&graph, LAYER_INPUT, 1, 2, EDGE_WEAK);
	graph_add_biedge(&graph, LAYER_INPUT, 2, 3, EDGE_WEAK);
	graph_add_biedge(&graph, LAYER_INPUT, 3, 4, EDGE_WEAK);
	graph_add_biedge(&graph, LAYER_INPUT, 4, 5, EDGE_WEAK);
	graph_add_biedge(&graph, LAYER_INPUT, 5, 0, EDGE_WEAK);

	graph_set_mine(&graph, 2);
	graph_build(&graph);
	graph_print_user_nodes(&graph);
	graph_print_edges(&graph, LAYER_INNER);

	graph_set_flag(&graph, 5, 1);
	graph_print_user_nodes(&graph);
	graph_print_edges(&graph, LAYER_INNER);

	graph_set_known(&graph, 0);
	graph_print_user_nodes(&graph);
	graph_print_edges(&graph, LAYER_INNER);

	graph_free(&graph);
	return 0;
}

