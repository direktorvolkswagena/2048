#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define D_INVALID -1
#define D_UP       1
#define D_DOWN     2
#define D_RIGHT    3
#define D_LEFT     4

const long values[] = {
    0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
};

const char *colors[] = {
    "39", "31", "32", "33", "34", "35", "36", "37", "91", "92", "93", "94"
};

struct gamestate_struct__ {
    int grid[4][4];
    int have_moved;
    long total_score;
    long score_last_move;
    int blocks_in_play;
} game;

struct termios oldt, newt;

void do_draw(void)
{
    printf("\033[2J\033[HScore: %ld", game.total_score); 
    // '\033' - escape char (ASCII 27), [2J - clears screen, [H - cursor home command
    if(game.score_last_move)
        printf(" (+%ld)", game.score_last_move);
    printf("\n");

    for (int i = 0; i < 25; ++i)
        printf("-");
    printf("\n");

    for(int y = 0; y < 4; ++y)
    {
        printf("|");
        for(int x = 0; x < 4; ++x)
        {
            if(game.grid[x][y])
                printf("\033[7m\033[%sm%*zd \033[0m|", colors[game.grid[x][y]], 4, values[game.grid[x][y]]);
            /* 
                \033[7m - escape seq to set the text to inverse, \033[%sm - escape sequence to set the text color. 
                %*zd - format specifier for a signed decimal integer with variable width (4 in this case)
                \033[0m| - escape seq to reset all text attributes (color, inverse, etc.)
            */
            else    
                printf("%*s |", 4, "");
        }
        printf("\n");
    }

    for(int i = 0; i < 25; ++i) 
        printf("-");
    printf("\n");
}

void do_merge(int d)
{
#define MERGE_DIRECTION(_v1, _v2, _xs, _xc, _xi, _ys, _yc, _yi, _x, _y) \
    do \
    { \
        for(int _v1 = _xs; _v1 _xc; _v1 += _xi) \
        { \
            for(int _v2 = _ys; _v2 _yc; _v2 += _yi) \
            { \
                if(game.grid[x][y] && (game.grid[x][y] == \
                                        game.grid[x + _x][y + _y])) \
                { \
                    game.grid[x][y] += (game.have_moved = 1); \
                    game.grid[x + _x][y + _y] = (0 * game.blocks_in_play--); \
                    game.score_last_move += values[game.grid[x][y]]; \
                    game.total_score += values[game.grid[x][y]]; \
                } \
            } \
        } \
    } while(0) \
    // do {...} while(0) idiom ensures that actions is done only once

    game.score_last_move = 0;

    switch(d)
    {
        case D_LEFT:
            MERGE_DIRECTION(x, y, 0, < 3, 1, 0, < 4, 1, 1, 0);
            break;
        case D_RIGHT:
            MERGE_DIRECTION(x, y,  3, > 0, -1, 0, < 4, 1, -1, 0);
            break;
        case D_DOWN:
            MERGE_DIRECTION(y, x, 3, > 0, -1, 0, < 4, 1, 0, -1);
            break;
        case D_UP:
            MERGE_DIRECTION(y, x, 0, < 3, 1, 0, < 4, 1, 0, 1);
            break;
    }

#undef MERGE_DIRECTION
} 

void do_gravity(int d)
{
#define GRAVITATE_DIRECTION(_v1, _v2, _xs, _xc, _xi, _ys, _yc, _yi, _x, _y) \
    do \
    { \
        int break_cond = 0; \
        while(!break_cond) \
        { \
            break_cond = 1; \
            for(int _v1 = _xs; _v1 _xc; _v1 += _xi) \
            { \
                for(int _v2 = _ys; _v2 _yc; _v2 += _yi) \
                { \
                    if(!game.grid[x][y] && game.grid[x + _x][y + _y]) \
                    { \
                        game.grid[x][y] = game.grid[x + _x][y + _y]; \
                        game.grid[x + _x][y + _y] = break_cond = 0; \
                        game.have_moved = 1; \
                    } \
                } \
            } \
            do_draw(); \
            usleep(40000); \
        } \
    } while(0) \

    switch(d)
    {
        case D_LEFT:
            GRAVITATE_DIRECTION(x, y, 0, < 3, 1, 0, < 4, 1, 1, 0);
            break;
        case D_RIGHT:
            GRAVITATE_DIRECTION(x, y,  3, > 0, -1, 0, < 4, 1, -1, 0);
            break;
        case D_DOWN:
            GRAVITATE_DIRECTION(y, x, 3, > 0, -1, 0, < 4, 1, 0, -1);
            break;
        case D_UP:
            GRAVITATE_DIRECTION(y, x, 0, < 3, 1, 0, < 4, 1, 0, 1);
            break;
    }

#undef GRAVITATE_DIRECTION
}

int do_check_end_condition(void)
{
    int ret = -1;
    for(int x = 0; x < 4; ++x)
    {
        for(int y = 0; y < 4; y++)
        {
            if(values[game.grid[x][y]] == 2048)
                return 1;
            if(!game.grid[x][y] ||
                ((x + 1 < 4) && (game.grid[x][y] == game.grid[x + 1][y])) || 
                ((y + 1 < 4) && (game.grid[x][y] == game.grid[x][y + 1])))
                    ret = 0;           
        }
    }
    return ret;
}

int do_tick(int d)
{
    game.have_moved = 0;
    do_gravity(d);
    do_merge(d);
    do_gravity(d);
    return game.have_moved;
}

void do_newblock(void)
{
    if(game.blocks_in_play >= 16) return;

    int bn = rand() % (16 - game.blocks_in_play);
    int pn = 0;

    for(int x = 0; x < 4; ++x)
    {
        for(int y = 0; y < 4; ++y)
        {
            if(game.grid[x][y])
                continue;

            if(bn == pn)
            {
                game.grid[x][y] = rand() % 10 ? 1 : 2; // 0-9 with ternary operator gives 1 if rand() % 10 == 1
                // else 2 (rand() % 10 == 0)
                game.blocks_in_play += 1;
                return;
            } 
            else 
            {
                ++pn;
            }
        }
    }
}

int main(void)
{
    tcgetattr(STDIN_FILENO, &oldt); 
    /*
    tcgetattr - function that retrieves the current terminal attributes
    STDIN_FILENO - descriptor representing a standard input stream
    &oldt - address of structure that stores current terminal settings
    */
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    /*
    c_flags refers to the local flags built in the termios structure. These flag control various aspects of terminal input processing
    ICANON - (canonical mode) is a local flag. When set it enables line-by-line input processing(inputting until the newline char)
    ECHO - another local flag. When set causes charachters typed by the user to be echoed on the terminal
    ~(ICANON | ECHO) creates a bitmask that has ICANON and ECHO bits inverted 
    Bitwise AND assignment effectively clears flags while used in conjunction with newt.c_flags 
    */ 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    /*
    TCSANOW - constant that specifies that the changes should take effect immediately 
    Generally code above does 2 things:
        Disable canonical mode, allowing character-by-character input.
        Disable echoing, preventing typed characters from being displayed.
    */
    srand(time(NULL));
    memset(&game, 0, sizeof(game));
    do_newblock();
    do_newblock();
    do_draw();

    while(1)
    {
        int found_valid_key, direction, value;
        do
        {
            found_valid_key = 1;
            direction = D_INVALID;
            value = getchar();

            switch(value)
            {
                case 'h': case 'a':
                    direction = D_LEFT;
                    break;
                case 'l': case 'd':
                    direction = D_RIGHT;
                    break;
                case 'j': case 's':
                    direction = D_DOWN;
                    break;
                case 'k': case 'w':
                    direction = D_UP; 
                    break;
                case 'q':
                    goto game_quit;
                    break;
                case 27: // this case handles escape sequences, used for arrow keys
                    if(getchar() == 91) // checks whether next char is '[' which is a part of arrow key escape sequence
                    {
                        value = getchar();
                        switch(value)
                        {
                            case 65:
                                direction = D_UP;
                                break;
                            case 66: 
                                direction = D_DOWN;
                                break;
                            case 67: 
                                direction = D_RIGHT;
                                break;
                            case 68: 
                                direction = D_LEFT; 
                                break;
                            default:
                                found_valid_key = 0;
                                break;
                        }
                    }
                break;

                default:
                    found_valid_key = 0;
                    break;
            }
        } while(!found_valid_key);

        do_tick(direction);
        if(game.have_moved != 0)
            do_newblock();
        do_draw();

        switch(do_check_end_condition())
        {
            case -1:
                goto game_lose;
            case 1:
                goto game_win;
            case 0:
                break;
        }
    }

    if(0) 
game_lose:
    printf("You lost!\n");
    goto game_quit;
    if(0)
game_win:
    printf("You won!\n");
    goto game_quit;
    if(0)
game_quit:
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restoring default terminal settings

    /*
    Why Use if(0)?

    It's a way to create labels without having the code directly following them executed unless a goto statement explicitly jumps to them.
    It is used because C requires that labels be within a function body. By placing them within an always false if statement, they can exist, 
    without the code within them always running.
    */

    return 0;
}