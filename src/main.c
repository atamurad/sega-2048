#include "genesis.h"
#include "sprite.h"

/* 2048 game state */
#define ROWS 16
#define COLS 8
int board[ROWS][COLS];

/* board transition state */
#define IDLE 0
#define MOVING_U 1
#define MOVING_D 2
#define MOVING_L 3
#define MOVING_R 4
#define GAMEOVER 5
int move_state = IDLE;

bool demo_mode = FALSE;
bool sonic_easter = FALSE;

/* Sonic sprite animations */
Sprite *sonic;

#define ANIM_STAND 0
#define ANIM_WAIT 1
#define ANIM_WALK 2
#define ANIM_RUN 3
#define ANIM_BRAKE 4
#define ANIM_UP 5
#define ANIM_CROUNCH 6
#define ANIM_ROLL 7

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
        board[x][y] = 0;
        return TRUE;
    } else if (board[x + dx][y + dy] == board[x][y]) {
        board[x + dx][y + dy]++;
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
    char *numstr[] = {"",    "2",   "4",   "8",    "16",   "32",   "64",
                      "128", "256", "512", "1024", "2048", "4096", "8192"};
    int colors[] = {0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3};

    VDP_clearTextArea(1, 1, COLS * 4, ROWS);

    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (board[i][j] == 0)
                continue;
            /* set font color */
            VDP_setTextPalette(colors[board[i][j]]);
            /* draw text */
            VDP_drawText(numstr[board[i][j]], (j * 4) + 1, i + 1);
        }
    }
}

void show_win_screen() {
    VDP_setTextPalette(3);
    VDP_drawText("YOU WIN!", 16, 25);
    SPR_setAnim(sonic, ANIM_RUN);
}

void show_gameover_screen() {
    VDP_setTextPalette(3);
    VDP_drawText("GAME OVER!", 10, 25);
    SPR_setAnim(sonic, ANIM_BRAKE);
}

int frames = 0;

void update_board() {

    /* update game board in every other 4 frames, otherwise its moving too fast
     */
    frames++;
    if (frames % 4 != 0)
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

    draw_board();

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
        SPR_setAnim(sonic, ANIM_WAIT);
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
        sonic_easter = !sonic_easter;
        SPR_setVisibility(sonic, !sonic_easter);
    }

    /* board is already in moving state, ignore U/D/L/R buttons */
    if (move_state != IDLE) {
        return;
    }

    if (joy == JOY_1) {
        if (state & BUTTON_RIGHT) {
            move_state = MOVING_R;
            SPR_setAnim(sonic, ANIM_WALK);
            SPR_setHFlip(sonic, FALSE);
        } else if (state & BUTTON_LEFT) {
            move_state = MOVING_L;
            SPR_setAnim(sonic, ANIM_WALK);
            SPR_setHFlip(sonic, TRUE);
        }

        if (state & BUTTON_UP) {
            move_state = MOVING_U;
            SPR_setAnim(sonic, ANIM_UP);
        } else if (state & BUTTON_DOWN) {
            move_state = MOVING_D;
            SPR_setAnim(sonic, ANIM_CROUNCH);
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

    /* load Sonic sprite */
    SPR_init();
    sonic = SPR_addSprite(&sonic_sprite, 120, 160,
                          TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
    SPR_setAnim(sonic, ANIM_WAIT);
    SPR_setVisibility(sonic, HIDDEN);

    /* set color palette */
    VDP_setPalette(PAL1, palette_all.data);
    /* overwrite font colors */
    VDP_setPaletteColor((PAL0 * 16) + 15, RGB24_TO_VDPCOLOR(0x006600));
    VDP_setPaletteColor((PAL1 * 16) + 15, RGB24_TO_VDPCOLOR(0x009900));
    VDP_setPaletteColor((PAL2 * 16) + 15, RGB24_TO_VDPCOLOR(0x00CC00));
    VDP_setPaletteColor((PAL3 * 16) + 15, RGB24_TO_VDPCOLOR(0x00FF00));

    /* init board state */
    init_board();
    draw_board();

    while (TRUE) {

        if(demo_mode)
            random_move();

        update_board();
        SPR_update();

        // always call this method at the end of the frame
        SYS_doVBlankProcess();
    }

    return 0;
}
