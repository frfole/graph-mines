#ifndef _MINE_GRAPH
#define _MINE_GRAPH

typedef enum {
	EDGE_NONE,
	EDGE_WEAK,
	EDGE_STRONG
} edge_type;

typedef enum: int {
	NODE_CLEAR = 0,
	NODE_MINE = -1
} node_type;

typedef struct {
	int is_flag;
	int is_known;
	node_type type;
} node_state;

typedef enum {
	LAYER_INPUT,
	LAYER_INNER,
} graph_layer;

typedef struct {
	int input_count;
	node_state* inner_node_states;
	int* input2inner;
	edge_type* input_conn;
	edge_type* inner_conn;
} mine_graph;

mine_graph graph_create(int input_count);
void graph_free(mine_graph* graph);
void graph_gen_mines(mine_graph* graph, int count);

void graph_print_user_nodes(mine_graph* graph);
void graph_print_edges(mine_graph* graph, graph_layer layer);
node_state graph_input_state(mine_graph* graph, int input_idx);

void graph_add_edge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx, edge_type edge);
void graph_add_biedge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx, edge_type edge);
edge_type graph_get_edge(mine_graph* graph, graph_layer layer, int from_idx, int to_idx);

void graph_set_mine(mine_graph* graph, int input_idx);
void graph_set_flag(mine_graph* graph, int input_idx, int is_flag);
void graph_toggle_flag(mine_graph* graph, int input_idx);
void _graph_set_known(mine_graph* graph, int inner_idx);
void graph_set_known(mine_graph* graph, int input_idx);

void graph_build(mine_graph* graph);
void graph_unbuild(mine_graph* graph);
void graph_rebuild(mine_graph* graph);

#endif
