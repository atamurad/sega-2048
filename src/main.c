#include "genesis.h"
#include "sprite.h"

/* 2048 game state */
#define ROWS 7
#define COLS 7
int board[ROWS][COLS];

int grid_x = (320 - COLS * 32) / 2;
int grid_y = (224 - ROWS * 32) / 2;

/* board transition state */
#define IDLE 0
#define MOVING_U 1
#define MOVING_D 2
#define MOVING_L 3
#define MOVING_R 4
#define GAMEOVER 5
int move_state = IDLE;

int animate[ROWS][COLS];

bool demo_mode = FALSE;
int palette_idx = 0;

Sprite *cell[ROWS][COLS];

int frames = 0;

/*
 * adds '2' or '4' tile to a random empty cell
 */
void add_random_tile() {

    if (move_state != IDLE)
        return;

    do {
        int x = random() % ROWS;
        int y = random() % COLS;
        int n = (random() % 2) + 1;

        if (board[x][y] == 0) {
            board[x][y] = n;
            break;
        }

    } while (1);
}

void init_board() {
    memset(board, 0, sizeof(board));
    memset(animate, 0, sizeof(animate));
    move_state = IDLE;
    add_random_tile();
}

/*
 * push tile at position x,y into dx,dy direction
 * merges two tiles if numbers match
 * returns TRUE if state has changed
 */
bool push_tile(int x, int y, int dx, int dy) {
    if (x + dx < 0 || x + dx >= ROWS || y + dy < 0 || y + dy >= COLS)
        return FALSE;
    if (board[x][y] == 0)
        return FALSE;
    if (board[x + dx][y + dy] == 0) {
        board[x + dx][y + dy] = board[x][y];
        animate[x + dx][y + dy] = 8;
        board[x][y] = 0;
        return TRUE;
    } else if (board[x + dx][y + dy] == board[x][y]) {
        board[x + dx][y + dy]++;
        animate[x + dx][y + dy] = 8;
        board[x][y] = 0;
        return TRUE;
    }
    return FALSE;
}

bool move_up() {
    int i, j;
    bool board_changed = FALSE;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (push_tile(i, j, -1, 0))
                board_changed = TRUE;
        }
    }
    return board_changed;
}

bool move_down() {
    int i, j;
    bool board_changed = FALSE;
    for (i = ROWS - 1; i >= 0; i--) {
        for (j = 0; j < COLS; j++) {
            if (push_tile(i, j, 1, 0))
                board_changed = TRUE;
        }
    }
    return board_changed;
}

bool move_left() {
    int i, j;
    bool board_changed = FALSE;

    for (j = 0; j < COLS; j++) {
        for (i = 0; i < ROWS; i++) {
            if (push_tile(i, j, 0, -1))
                board_changed = TRUE;
        }
    }
    return board_changed;
}

bool move_right() {
    int i, j;
    bool board_changed = FALSE;

    for (j = COLS - 1; j >= 0; j--) {
        for (i = 0; i < ROWS; i++) {
            if (push_tile(i, j, 0, 1))
                board_changed = TRUE;
        }
    }
    return board_changed;
}

void draw_board() {
    int i, j;

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {

            SPR_setVisibility(cell[i][j], VISIBLE);
            SPR_setFrame(cell[i][j], board[i][j] - 1);

            int anim_off_y = 0;
            int anim_off_x = 0;
            if (animate[i][j]) {
                if (move_state == MOVING_D) {
                    anim_off_y = animate[i][j] * -4;
                }
                if (move_state == MOVING_U) {
                    anim_off_y = animate[i][j] * 4;
                }
                if (move_state == MOVING_R) {
                    anim_off_x = animate[i][j] * -4;
                }
                if (move_state == MOVING_L) {
                    anim_off_x = animate[i][j] * 4;
                }
                animate[i][j]--;
            }

            SPR_setPosition(cell[i][j], grid_x + j * 32 + anim_off_x,
                            grid_y + i * 32 + anim_off_y);

            if (board[i][j] == 0) {
                SPR_setVisibility(cell[i][j], HIDDEN);
                continue;
            }
        }
    }
}

void update_color_palette()
{
    int i, j;
    for(i=0; i<ROWS; i++) {
        for(j=0; j<COLS; j++) {
            SPR_setPalette(cell[i][j], palette_idx);
        }
    }
}

void show_win_screen() { VDP_drawText("YOU WIN!", 16, 25); }

void show_gameover_screen() { VDP_drawText("GAME OVER!", 10, 25); }

void update_board() {

    frames++;
    draw_board();

    if (frames % 8 != 0)
        return;

    bool was_idle = (move_state == IDLE);

    if (move_state == MOVING_U) {
        if (!move_up())
            move_state = IDLE;
    }

    if (move_state == MOVING_D) {
        if (!move_down())
            move_state = IDLE;
    }

    if (move_state == MOVING_L) {
        if (!move_left())
            move_state = IDLE;
    }
    if (move_state == MOVING_R) {
        if (!move_right())
            move_state = IDLE;
    }

    /* check game state for wins or game over */
    bool game_over = TRUE;
    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (board[i][j] == 0)
                game_over = FALSE;
            if (board[i][j] == 11)
                show_win_screen();
        }
    }
    if (game_over) {
        move_state = GAMEOVER;
        show_gameover_screen();
    }

    /* add new tile */
    if (!was_idle && move_state == IDLE) {
        add_random_tile();
    }
}

void myJoyHandler(u16 joy, u16 changed, u16 state) {

    if (state & BUTTON_START) {
        init_board();
    }

    if (state & BUTTON_A) {
        /* toogle demo mode */
        demo_mode = !demo_mode;
    }

    if (state & BUTTON_B) {
        palette_idx = (palette_idx + 1) % 4;
        update_color_palette();
   }

    /* board is already in moving state, ignore U/D/L/R buttons */
    if (move_state != IDLE) {
        return;
    }

    if (joy == JOY_1) {
        if (state & BUTTON_RIGHT) {
            move_state = MOVING_R;
        } else if (state & BUTTON_LEFT) {
            move_state = MOVING_L;
        }

        if (state & BUTTON_UP) {
            move_state = MOVING_U;
        } else if (state & BUTTON_DOWN) {
            move_state = MOVING_D;
        }
    }
}

int idle_frames = 0;
void random_move() {
    if (move_state == IDLE) {
        idle_frames++;
        if (idle_frames == 10) {
            int move = random() % 4;
            if (move == 0)
                myJoyHandler(JOY_1, 0, BUTTON_UP);
            if (move == 1)
                myJoyHandler(JOY_1, 0, BUTTON_DOWN);
            if (move == 2)
                myJoyHandler(JOY_1, 0, BUTTON_LEFT);
            if (move == 3)
                myJoyHandler(JOY_1, 0, BUTTON_RIGHT);
        }
    } else {
        idle_frames = 0;
    }
}

int main(bool hardReset) {
    /* init joystick handler */
    JOY_init();
    JOY_setEventHandler(&myJoyHandler);

    /* load Tiles sprite */
    SPR_init();

    int i, j;
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            cell[i][j] =
                SPR_addSprite(&tiles_sprite, grid_x + j * 32, grid_y + i * 32,
                              TILE_ATTR(PAL2, TRUE, FALSE, FALSE));
        }
    }
    VDP_setPalette(PAL2, tiles_sprite.palette->data);
    palette_idx = 2;
    update_color_palette();

    /* init board state */
    init_board();
    draw_board();

    while (TRUE) {

        if (demo_mode)
            random_move();

        update_board();
        SPR_update();

        // always call this method at the end of the frame
        SYS_doVBlankProcess();
    }

    return 0;
}
