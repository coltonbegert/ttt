#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


// typedef struct{
//     uint8_t positions[17];
// } state;
typedef union {
    struct {
        uint8_t a:2;
        uint8_t b:2;
        uint8_t c:2;
        uint8_t d:2;
        uint8_t e:2;
        uint8_t f:2;
        uint8_t g:2;
        uint8_t h:2;
        uint8_t i:2;
        uint8_t last_player:2;
        uint8_t pad:4;
        uint8_t board;
    } val;
    int32_t num;
} position;

int8_t MAGIC_SQUARE[] = {2,7,6,9,5,1,4,3,8,-2,-7,-6,-9,-5,-1,-4,-3,-8};
typedef struct{
    // position pos;
    int32_t pos;
    int8_t magic[8];
} mini_board;

// typedef struct state state;
typedef struct state{
    mini_board mini_board[9]; //malloc(9*sizeof(board));
    struct {
        uint8_t player:2;
        uint8_t board:6; // init==9
    } last;
    int8_t big_magic[8];
    int32_t visits;
    int32_t wins;
    struct state *child;
    struct state *next;
    struct state *prev;
    struct state *parent;
    struct state *best_child;
    struct state *best_leaf;
} state;

state *create_head ();
state *expand_node(state *leaf);
void backprop(state *leaf, int result);
int simulate(state *node);
int make_move(mini_board *board, int move, int player);
state *create_child(state *next_child, state *leaf, int board, int move, int player);
state *selection (state *in_state);
int check_win(int move, mini_board *new_board, char player);
int mcts(state *in_state);
int valid_move(mini_board *board, int move, int player);
