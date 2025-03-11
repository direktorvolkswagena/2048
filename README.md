# 2048 Game (C Implementation)

### This is a terminal-based implementation of the classic 2048 game written in C.
Features

    Playable in the terminal with intuitive controls
    Supports both arrow keys and WASD / HJKL for movement
    Randomly spawns new blocks after every valid move
    Displays current score and score per move
    Uses ANSI escape codes for colored output

Compilation

To compile the game, use the following command:

gcc 2048.c -o 2048

Usage

Run the compiled program:

./2048

Controls

    Arrow Keys: Move tiles up, down, left, or right
    W / A / S / D: Alternative movement keys
    H / J / K / L: Vim-style movement keys
    Q: Quit the game

How to Play

'''
    The game starts with two tiles (2 or 4) in random positions.
    Use the movement keys to slide all tiles in a direction.
    Tiles of the same value merge when they collide, doubling their value.
    The goal is to create a tile with the value 2048.
    The game ends when there are no valid moves left.
'''

Notes

    The game disables canonical mode and echoing in the terminal to allow for real-time input processing.
    The score is displayed at the top of the game screen.
    The game restores terminal settings upon exiting.
