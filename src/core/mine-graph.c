#include <stdlib.h>
#include <stdio.h>

#include "mine-graph.h"

mine_graph graph_create(int input_count) {
	mine_graph graph = {
		.input_count = input_count,
		.inner_node_states = malloc(input_count * sizeof(node_state)),
		.input2inner = malloc(input_count * sizeof(int)),
		.input_conn = malloc(input_count * input_count * sizeof(edge_type)),
		.inner_conn = malloc(input_count * input_count * sizeof(edge_type)),
	};
	for (int i = 0; i < input_count; i++) {
		graph.inner_node_states[i].is_flag = 0;
		graph.inner_node_states[i].is_known = 0;
		graph.inner_node_states[i].type = NODE_CLEAR;
		graph.input2inner[i] = i;
	}
	for (int i = 0; i < input_count * input_count; i++) {
		graph.input_conn[i] = EDGE_NONE;
		graph.inner_conn[i] = EDGE_NONE;
	}
	return graph; 
}

void graph_free(mine_graph* graph) {
	free(graph->inner_node_states);
	free(graph->input2inner);
	free(graph->input_conn);
	free(graph->inner_conn);
	graph->input_count = 0;
}

void graph_gen_mines(mine_graph* graph, int count) {
	int empty_inputs = graph->input_count;
	int* possible_mines = malloc(empty_inputs * sizeof(int));
	for (int i = 0; i < empty_inputs; i++) {
		possible_mines[i] = i;
	}
	for (int i = 0; i < count; i++) {
		int new_mine = random() % empty_inputs;
		graph_set_mine(graph, possible_mines[new_mine]);
		for (int j = new_mine + 1; j < empty_inputs; j++) {
			possible_mines[j-1] = possible_mines[j];
		}
		empty_inputs--;
	}
	free(possible_mines);
}

void graph_print_user_nodes(mine_graph* graph) {
	printf("node inner type\n");
	for (int i = 0; i < graph->input_count; i++) {
		node_state state = graph->inner_node_states[graph->input2inner[i]];
		printf("%4d %5d f%d k%d t%d\n", i, graph->input2inner[i], state.is_flag, state.is_known, state.type);
		for (int j = 0; j < graph->input_count; j++) {
			edge_type edge = graph_get_edge(graph, LAYER_INPUT, i, j);
			if (edge == EDGE_WEAK) {
				printf(" --> %5d weak\n", j);
			} else if (edge == EDGE_STRONG) {
				printf(" --> %5d strong\n", j);
			}
		}
	}
}

void graph_print_edges(mine_graph* graph, graph_layer layer) {
	printf("from   to type\n");
	for (int i = 0; i < graph->input_count; i++) {
		for (int j = 0; j < graph->input_count; j++) {
			edge_type edge = graph_get_edge(graph, layer, i, j);
			if (edge == EDGE_STRONG) {
				printf("%4d %4d strong\n", i, j);
			} else if (edge == EDGE_WEAK) {
				printf("%4d %4d weak\n", i, j);
			}
		}
	}
}

node_state graph_input_state(mine_graph* graph, int input_idx) {
	return graph->inner_node_states[graph->input2inner[input_idx]];
}

void graph_add_edge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx, edge_type edge) {
	if (layer == LAYER_INPUT) {
		graph->input_conn[graph->input_count * from_idx + to_idx] = edge;
	} else if (layer == LAYER_INNER) {
		graph->inner_conn[graph->input_count * from_idx + to_idx] = edge;
	}
}

void graph_add_biedge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx, edge_type edge) {
	graph_add_edge(graph, layer, from_idx, to_idx, edge);
	graph_add_edge(graph, layer, to_idx, from_idx, edge);
}

edge_type graph_get_edge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx) {
	if (layer == LAYER_INPUT) {
		return graph->input_conn[graph->input_count * from_idx + to_idx];
	} else if (layer == LAYER_INNER) {
		return graph->inner_conn[graph->input_count * from_idx + to_idx];
	}
	return EDGE_NONE;
}

void graph_set_mine(mine_graph* graph, int input_idx) {
	graph->inner_node_states[input_idx].type = NODE_MINE;
}

void graph_set_flag(mine_graph* graph, int input_idx, int is_flag) {
	if (graph->inner_node_states[graph->input2inner[input_idx]].is_flag == is_flag)
		return;
	if (graph->inner_node_states[graph->input2inner[input_idx]].is_known)
		return;
	// TODO: optimize (no need to unbuild whole graph, only part)
	graph_unbuild(graph);
	graph->inner_node_states[graph->input2inner[input_idx]].is_flag = is_flag;
	graph_rebuild(graph);
}

void graph_toggle_flag(mine_graph* graph, int input_idx) {
	int flag_state = graph->inner_node_states[graph->input2inner[input_idx]].is_flag;
	graph_set_flag(graph, input_idx, !flag_state);
}

void graph_set_known(mine_graph* graph, int input_idx) {
	graph_set_flag(graph, input_idx, 0);
	_graph_set_known(graph, graph->input2inner[input_idx]);
}

void _graph_set_known(mine_graph* graph, int inner_idx) {
	if (graph->inner_node_states[inner_idx].is_known)
		return;
	graph->inner_node_states[inner_idx].is_known = 1;
	for (int i = 0; i < graph->input_count; i++) {
		if (graph_get_edge(graph, LAYER_INNER, inner_idx, i) != EDGE_STRONG)
			continue;
		if (graph->inner_node_states[i].is_flag) {
			graph_unbuild(graph);
			graph->inner_node_states[i].is_flag = 0;
			graph_rebuild(graph);
		}
		_graph_set_known(graph, i);
	}
}

void graph_unbuild(mine_graph* graph) {
	for (int i = 0; i < graph->input_count; i++) {
		int old_node = graph->input2inner[i];
		for (int j = 0; j < graph->input_count; j++) {
			graph_add_edge(graph, LAYER_INNER, i, j, EDGE_NONE);
		}
		if (old_node == i)
			continue;
		graph->input2inner[i] = i;
		graph->inner_node_states[i] = graph->inner_node_states[old_node];
	}
}

void graph_build(mine_graph* graph) {
	for (int i = 0; i < graph->input_count; i++) {
		graph->input2inner[i] = i;
		if (graph->inner_node_states[i].type != NODE_MINE)
			continue;
		for (int j = 0; j < graph->input_count; j++) {
			if (graph->inner_node_states[j].type == NODE_MINE)
				continue;
			edge_type edge = graph_get_edge(graph, LAYER_INPUT, i, j);
			if (edge != EDGE_WEAK)
				continue;
			graph->inner_node_states[j].type++;
		}
	}

	for (int i = 0; i < graph->input_count; i++) {
		if (graph->inner_node_states[graph->input2inner[i]].type != NODE_CLEAR)
			continue;
		for (int j = 0; j < graph->input_count; j++) {
			int inner_n_from = graph->input2inner[i];
			int inner_n_to = graph->input2inner[j];
			edge_type edge = graph_get_edge(graph, LAYER_INPUT, i, j);
			if (edge != EDGE_WEAK || inner_n_to == inner_n_from)
				continue;
			if (graph->inner_node_states[inner_n_to].type == NODE_MINE)
				continue;
			if (graph->inner_node_states[inner_n_to].type == NODE_CLEAR) {
				for (int k = 0; k < graph->input_count; k++) {
					if (graph_get_edge(graph, LAYER_INNER, inner_n_from, graph->input2inner[k]) == EDGE_STRONG) {
						graph_add_edge(graph, LAYER_INNER, inner_n_to, graph->input2inner[k], EDGE_STRONG);
						graph_add_edge(graph, LAYER_INNER, inner_n_from, graph->input2inner[k], EDGE_NONE);
					}
					if (graph_get_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_from) == EDGE_STRONG) {
						graph_add_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_to, EDGE_STRONG);
						graph_add_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_from, EDGE_NONE);
					}
					if (graph->input2inner[k] == inner_n_from) {
						graph->input2inner[k] = inner_n_to;
					}
				}
			} else if (graph->inner_node_states[inner_n_to].type > 0) {
				graph_add_edge(graph, LAYER_INNER, inner_n_from, inner_n_to, EDGE_STRONG);
			}
		}
	}
}

void graph_rebuild(mine_graph* graph) {
	for (int i = 0; i < graph->input_count; i++) {
		if (graph->inner_node_states[graph->input2inner[i]].type != NODE_CLEAR)
			continue;
		for (int j = 0; j < graph->input_count; j++) {
			int inner_n_from = graph->input2inner[i];
			node_state inner_s_from = graph->inner_node_states[inner_n_from];
			int inner_n_to = graph->input2inner[j];
			node_state inner_s_to = graph->inner_node_states[inner_n_to];
			edge_type edge = graph_get_edge(graph, LAYER_INPUT, i, j);
			if (edge != EDGE_WEAK || inner_n_to == inner_n_from)
				continue;
			if (inner_s_to.type == NODE_MINE)
				continue;
			if (inner_s_to.type == NODE_CLEAR && inner_s_to.is_known == inner_s_from.is_known && inner_s_to.is_flag == inner_s_from.is_flag) {
				for (int k = 0; k < graph->input_count; k++) {
					if (graph_get_edge(graph, LAYER_INNER, inner_n_from, graph->input2inner[k]) == EDGE_STRONG) {
						graph_add_edge(graph, LAYER_INNER, inner_n_to, graph->input2inner[k], EDGE_STRONG);
						graph_add_edge(graph, LAYER_INNER, inner_n_from, graph->input2inner[k], EDGE_NONE);
					}
					if (graph_get_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_from) == EDGE_STRONG) {
						graph_add_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_to, EDGE_STRONG);
						graph_add_edge(graph, LAYER_INNER, graph->input2inner[k], inner_n_from, EDGE_NONE);
					}
					if (graph->input2inner[k] == inner_n_from) {
						graph->input2inner[k] = inner_n_to;
					}
				}
			} else if (inner_s_to.type >= 0) {
				graph_add_edge(graph, LAYER_INNER, inner_n_from, inner_n_to, EDGE_STRONG);
			}
		}
	}
}
