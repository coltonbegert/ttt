
#include "mcts.h"

// Coefficient to use in UCB for selection policy
#define SELECTION_COEFF 1.414

// Coefficient to use in UCB when selecting moves.
// 0.0 is good for exploitation of mean,
// -1.0 is good for minimizing loss.
#define POLICY_COEFF 0.0

// typedef struct state{
//     mini_board mini_board[9]; //malloc(9*sizeof(board));
//     int8_t big_magic[8];
//     int32_t visits;
//     int32_t wins;
//     struct state *child;
//     struct state *next;
//     struct state *prev;
//     struct state *parent;
// } state;

int select_count = 0;
double selct_clock = 0;
state *head;

void clean_up(state *node) {
    if (node->child == NULL) {
        if (node->next == NULL) {
            if(node->prev != NULL) {
                node->prev->next = NULL;
            }
            free(node);
            return;
        } else {
            clean_up(node->next);
        }
    } else {
        clean_up(node->child);
    }
}

// void mem_pool(state *pool, int num_states) {
//
// }
state *selection (state *in_state, float coeff) {
    // return in_state;
    // printf("selection\n");
    select_count++;
    state *next_child;
    state *best_child;
    float best_val =-INFINITY;
    int i = 0;

    float numer;
    if (in_state->parent->visits == 0)
        numer = INFINITY;
    else 
        numer = coeff*sqrtf(logf((float)in_state->parent->visits));

    // next_child = malloc(sizeof(state));
    for (next_child = in_state->parent->child; next_child->next != NULL; next_child = next_child->next) {
        //choosing next child to explore based on selection policy
        // printf("%d, %d\n", next_child->visits, next_child->wins);
        i++;
        float mean, score;
        if (next_child->visits == 0) {
            score = INFINITY;
        } else {
            mean = next_child->wins / next_child->visits;
            score = mean + numer / sqrtf((float)next_child->visits);
        }
        if (score > best_val) {
            best_child = next_child;
            best_val = score;
        }
    }
    // printf("num children: %d\n", i);
    // printf("e select\n");
    // printf("best w/l: %d/%d\n", best_child->wins, best_child->visits);
    return best_child;
}
// state *root;
int mcts(state *in_state) {
    // printf("monte carlo\n");
    // printf("%d\n", select_count);
    state *leaf;
    state *cur_child;
    clock_t begin = clock();
    // finds leaf node based on selection policy
    // for (leaf = in_state; leaf->child!=NULL; leaf = selection(leaf->child));
    leaf = in_state->best_leaf;
    if (leaf == NULL) {
        for (leaf = in_state; leaf->child!=NULL; leaf = leaf->best_child);
    }
    // leaf->mini_board->pos.val.board
    clock_t end = clock();
    selct_clock += (double)(end - begin) / CLOCKS_PER_SEC;

    cur_child = expand_node(leaf);
    int sim_result = simulate(cur_child);
    backprop(cur_child, sim_result);
    return 1;
}

void backprop(state *leaf, int result) {
    int need_select = 1;
    // int j=0;
    state *best_node;
    head->best_leaf=NULL;

    // printf("backprop\n");
    state *node;
    state *best_leaf = leaf;
    int player = leaf->last.player;
    int levels = 0, select_updates = 0;
    // player = player == 1 ? 2 : 1
    // printf("%d\n", result);
    for (node = leaf; node!=NULL;node = node->parent) {
        // printf("%d\n", node->visits);
        // printf("backprop:%d\n", i++);
        if (node != head) {
            best_node = selection(node->parent->child, SELECTION_COEFF);
            node->parent->best_child = best_node;
            if (best_node != node) {
                best_leaf = best_node->best_leaf;
            }
            node->parent->best_leaf = best_leaf;
        }

        levels++;
        // printf("pre vis: %d\n", node->visits);
        node->visits++;
        // printf("pos vis: %d\n", node->visits);
        if (result) {
            node->wins++;
        }
        result = !result;

    }
    // if (levels == select_updates) {
    //     head->best_leaf = leaf;
    // }
    // printf("level of node: %d, levels of select:%d\n", i, j);
    // printf("got out\n");
}


int simulate(state *node) {
    int r = rand();
    // return 1;
    return r%2;
}

state *expand_node(state *leaf) {
    // printf("expand_node\n");
    state *cur_child;

    int player = leaf->last.player;
    int next_player = ((player == 1 ) ? 2 : 1);
    cur_child = leaf->child;
    // leaf->mini_board[leaf->last.board].pos;

    //if last.board is 9, last move won a board so create states for all boards
    int i,end;
    if (leaf->last.board == 9) {
        i = 0, end = 8;
    } else {
        i = end = leaf->last.board;
    }
    int num_child = 0;
    for (; i<=end; i++) {
        for (int j = 0; j < 9; j++) {
            // printf("create child: %d on board %d\n", j,i);
            if (valid_move(&leaf->mini_board[i], j, player)) {
                cur_child = create_child(cur_child, leaf, i, j, next_player);
                //TODO actually add the move to child and check for win, flip player
                // cur_child->mini_board[i]
                if (!make_move(&cur_child->mini_board[i], j, next_player)){
                    // we have a winner
                };
                if (leaf->child == NULL) {
                    leaf->child = cur_child;
                }

                num_child++;
            }
        }
    }
    //  return random child for simulation
    int r = rand()%num_child;
    int k = 0;
    for (cur_child = leaf->child; k < r; k++, cur_child = cur_child->next);
    return cur_child;

}

state *create_child(state *prev_child, state *leaf, int board, int move, int player){
    // printf("create_child\n");
    state *new_child;
    new_child = malloc(sizeof(state));
    // new_child = calloc(1, sizeof(state));
    // printf("1\n");
    memcpy(new_child, leaf, sizeof(state));
    // printf("2\n");
    new_child->parent = leaf;
    new_child->prev = prev_child;
    if (new_child->prev != NULL) {
        new_child->prev->next = new_child;

    }
    new_child->next = NULL;
    new_child->child = NULL;
    new_child->visits = 0;
    new_child->wins =0;
    new_child->last.board = board;
    new_child->last.player = player;
    new_child->best_leaf = new_child;


    return new_child;
}

state *create_head () {
    state *head;
    // head = malloc(sizeof(state));
    head = calloc(1, sizeof(state));
    head->parent = NULL;
    head->next = NULL;
    head->prev = NULL;
    head->child = NULL;
    head->visits = 0;
    head->wins =0;
    head->last.board = 9; // 9 means any board can be chosen
    head->last.player = 1;
    head->best_child = NULL;
    head->best_leaf = NULL;
    return head;
}
int valid_move(mini_board *board, int move, int player) {
    // printf("valid_move\n");
    // return 1;
    if (((board->pos << (2*move)) & 0x3) == 0) {
        // board->pos |= player<<(2*move);
        return 1;
    } else {
        return 0;
    }
}
int make_move(mini_board *board, int move, int player) {
    // printf("make_move\n");
    board->pos |= player<<(2*move);
    // return 1;
    if (check_win(move, board, player) == player) {
        return 1;
        printf("found winner\n");
    }
    return 0;
    // if (((board->pos << (2*move)) & 0x3) == 0) {
    //     return 1;
    // } else {
    //     return 0;
    // }
}


int main(int argc, char const *argv[]) {
    srand(time(NULL));
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;



    //  mini_board new_board;
    // new_board.pos = malloc(sizeof(position) * 81);
    // printf("Size of 1 position: %lu\n", sizeof(position));
    printf("Size of 1 board: %lu\n", sizeof(mini_board));
    printf("Size of 1 state: %lu\n", sizeof(state));

    // state *head;
    // head = malloc(sizeof(state));
    clock_t begin = clock();
    head = create_head();
    // state *head = create_head();
    for (int i = 0; i < 1000; i++) {
        // printf("iteration: %d\n", i);
        mcts(head);

    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("time spent:%f, %f.\n", time_spent, selct_clock);

    printf("did we escape? %d \n", select_count);
    printf("wins: %d, visits: %d.\n", head->wins, head->visits);
    int node_num = 0;
    for (state *next_child = head->child;next_child->next!=NULL; node_num++, next_child = next_child->next) {
        printf("node:%d ,wins: %d ,visits: %d.\n", node_num,next_child->wins, next_child->visits);
        /* code */
    }
    clean_up(head);
    return 0;
    int move;
    // player = 1;
    scanf("%d", &move);

    int8_t magic_move, magic_sum;
    char player = 1;
    if (player == 1) {
        magic_move = move;
        magic_sum = 15;
    } else if (player == 2){
        magic_move = move + 9;
        magic_sum = -15;
    } else {
        return 1;
    }

    return 0;
}


int check_win(int move, mini_board *new_board, char player) {
    // printf("check_win\n");
    int8_t magic_move, magic_sum;
    // int8_t MAGIC_SQUARE[] = {2,7,6,9,5,1,4,3,8,-2,-7,-6,-9,-5,-1,-4,-3,-8};
    // if(new_board.magic[3 +move/3] += MAGIC_SQUARE[magic_move] == magic_sum){
    //     return player;
    // }
    // if(new_board.magic[move%3] += MAGIC_SQUARE[magic_move] == magic_sum){
    //     return player;
    // }
    // if(!(move %4) && (new_board.magic[6] += MAGIC_SQUARE[magic_move] == magic_sum)){
    //     return player;
    // }
    // if(!(move / 2)&& !(move/7) && (new_board.magic[6] += MAGIC_SQUARE[magic_move] == magic_sum)) {
    //     return player;
    // }

    if (player == 1) {
        magic_move = move;
        magic_sum = 15;
    } else if (player == 2){
        magic_move = move + 9;
        magic_sum = -15;
    } else {
        return 1;
    }
    switch (move) {
        case 0:
        if(new_board->magic[0] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[3] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[6] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 1:
        if(new_board->magic[1] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[3] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 2:
        if(new_board->magic[2] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[3] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[7] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 3:
        if(new_board->magic[4] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[0] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }

        break;
        case 4:
        if(new_board->magic[1] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[4] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[7] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[6] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 5:
        if(new_board->magic[2] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[4] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 6:
        if(new_board->magic[0] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[5] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[7] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 7:
        if(new_board->magic[1] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[5] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
        case 8:
        if(new_board->magic[2] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[5] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        if(new_board->magic[6] += MAGIC_SQUARE[magic_move] == magic_sum){
            return player;
        }
        break;
    }
    return 0;
}
