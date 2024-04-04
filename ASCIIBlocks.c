// ASCIIBlocks Copyright (C) 2024 Hayden 
//
// This file is part of ASCIIBlocks. 
//
// ASCIIBlocks is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// ASCIIBlocks is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with ASCIIBlocks. If not, see <https://www.gnu.org/licenses/>.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#define WIDTH 64
#define HEIGHT 24

#define C_BORDER '+'
#define H_BORDER '-'
#define V_BORDER '|'

#define PLAYER 'X'
#define BLOCKS 25

#define WARP_1 15
#define WARP_2 16
#define WARP_3 17
#define WARP_4 18
#define WARP_5 19
#define WARP_6 20
#define WARP_7 21
#define WARP_8 22
#define WARP_TO_SPAWN 23
#define WARP_TO_RANDOM_COORDS 24

#define MAP_SIZE WIDTH + (HEIGHT - 1) * WIDTH

#define RELATIVE_BLOCK(Y, X) map[(y_pos + Y) * WIDTH + (x_pos + X)]
#define PLACE_RELATIVE_BLOCK(Y, X) (RELATIVE_BLOCK(Y, X) == 0) ? held_block : 0
#define BLOCK_AT(Y, X) map[Y * WIDTH + X]

#define LEVEL_SIGNATURE "ASCIIBLOCKS"
#define OPTIONS 9

int cursor_y;
int cursor_x;

int y_pos = 0;
int x_pos = 0;
FILE *file;
uint8_t *map;
uint8_t held_block = 1;
char block[BLOCKS] = {' ', '#', '%', '&', '$', '*', '@', '+', '[', ']', '~', '-', '=', '_', '!', '1', '2', '3', '4', '5', '6', '7', '8', '0', '?'};
char *block_name[BLOCKS] = {"Air", "Wood", "Leaves", "Grass", "Stone", "Bedrock", "Wool", "Fence",
							"Left-Facing Door", "Right-Facing Door", "Vines", "Path", "Entrance", "Carpet", "Barrier",
							"Warp 1", "Warp 2", "Warp 3", "Warp 4", "Warp 5", "Warp 6", "Warp 7", "Warp 8",
							"Warp to Spawn", "Warp to Random Co-ords"};
bool block_solid_status[BLOCKS] = {false, true, true, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false};

bool solidity = true;
bool warps = true;

char *options_list[OPTIONS] = {"Back",
							   "New Level",
							   "Load Level",
							   "Save Level",
							   "Teleport",
							   "Teleport Relative",
							   "Enable/Disable Painting",
							   "Enable/Disable Solidity",
							   "Enable/Disable Warps"};

void init_colour()
{
	start_color();
	
	init_pair(0,  COLOR_BLACK,   COLOR_BLACK);
	init_pair(1,  COLOR_YELLOW,  COLOR_BLACK);
	init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
	init_pair(3,  COLOR_YELLOW,  COLOR_GREEN);
	init_pair(4,  COLOR_WHITE,   COLOR_WHITE);
	init_pair(5,  COLOR_BLACK,   COLOR_WHITE);
	init_pair(6,  COLOR_WHITE,   COLOR_BLACK);
	init_pair(7,  COLOR_YELLOW,  COLOR_BLACK);
	init_pair(8,  COLOR_RED,     COLOR_BLACK);
	init_pair(9,  COLOR_BLUE,    COLOR_BLACK);
	init_pair(10, COLOR_GREEN,   COLOR_BLACK);
	init_pair(11, COLOR_GREEN,   COLOR_YELLOW);
	init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(13, COLOR_WHITE,   COLOR_BLACK);
	init_pair(14, COLOR_BLACK,   COLOR_RED);
	init_pair(15, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(16, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(17, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(18, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(19, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(20, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(21, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(22, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(23, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(24, COLOR_BLACK,   COLOR_MAGENTA);
}

void draw_borders()
{
	int i;
	
	mvaddch(0, 0, C_BORDER);
	mvaddch(0, WIDTH + 1, C_BORDER);
	mvaddch(HEIGHT + 1, 0, C_BORDER);
	mvaddch(HEIGHT + 1, WIDTH + 1, C_BORDER);
	
	for (i = 1; i <= WIDTH; i++) {
		mvaddch(0, i, H_BORDER);
		mvaddch(HEIGHT + 1, i, H_BORDER);
	}

	for (i = 1; i <= HEIGHT; i++) {
		mvaddch(i, 0, V_BORDER);
		mvaddch(i, WIDTH + 1, V_BORDER);
	}
}

void draw_player()
{
	mvaddch(y_pos + 1, x_pos + 1, PLAYER);
}

void draw_map()
{
	int i;
	
	for (i = 0; i < MAP_SIZE; i++) {
		attron(COLOR_PAIR(map[i]));
		mvaddch((i / WIDTH) + 1, (i % WIDTH) + 1, block[map[i]]);
		attroff(COLOR_PAIR(map[i]));
	}
	
	draw_player();
}

void draw_ui()
{
	mvprintw(0, WIDTH + 4, "ASCIIBlocks 1.5.0-alpha.3\n");
	mvprintw(1, WIDTH + 4, "by Hayden\n");
	
	mvprintw(3, WIDTH + 4, "Use WASD to move,\n");
	mvprintw(4, WIDTH + 4, "IJKL to place and remove blocks\n");
	mvprintw(5, WIDTH + 4, "and F or H to select blocks.\n");
	
	mvprintw(7, WIDTH + 4, "Use M to open the menu\n");
	mvprintw(8, WIDTH + 4, "and CTRL-C to quit the game.\n");
	
	mvprintw(13, WIDTH + 4, "Block Texture:\n");

	draw_borders();
	draw_map();
}

void move_player(int relative_block_y, int relative_block_x, int *axis_as_position, bool condition, int position_increment)
{
	if (RELATIVE_BLOCK(relative_block_y, relative_block_x) == WARP_TO_SPAWN && warps) {
		attron(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		mvaddch(cursor_y, cursor_x - 1, block[RELATIVE_BLOCK(0, 0)]);
		attroff(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		
		y_pos = 0, x_pos = 0;
		draw_player();
		
		return;
	}
	
	if (RELATIVE_BLOCK(relative_block_y, relative_block_x) == WARP_TO_RANDOM_COORDS && warps) {
		attron(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		mvaddch(cursor_y, cursor_x - 1, block[RELATIVE_BLOCK(0, 0)]);
		attroff(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		
		y_pos = rand() % HEIGHT, x_pos = rand() % WIDTH;
		draw_player();
		
		return;
	}
	
	if (condition && (!block_solid_status[RELATIVE_BLOCK(relative_block_y, relative_block_x)] || !solidity)) {
		attron(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		mvaddch(cursor_y, cursor_x - 1, block[RELATIVE_BLOCK(0, 0)]);
		attroff(COLOR_PAIR(RELATIVE_BLOCK(0, 0)));
		mvaddch(cursor_y + relative_block_y, cursor_x + relative_block_x - 1, PLAYER);
		
		*axis_as_position += position_increment;
	}
}

void place_block(int relative_block_y, int relative_block_x, bool condition)
{
	if (condition) {
		attron(COLOR_PAIR(RELATIVE_BLOCK(relative_block_y, relative_block_x) ? 0 : held_block));
		mvaddch(cursor_y + relative_block_y, cursor_x + relative_block_x - 1, block[PLACE_RELATIVE_BLOCK(relative_block_y, relative_block_x)]);
		attroff(COLOR_PAIR(RELATIVE_BLOCK(relative_block_y, relative_block_x) ? 0 : held_block));
		move(cursor_y, cursor_x);
		
		RELATIVE_BLOCK(relative_block_y, relative_block_x) = PLACE_RELATIVE_BLOCK(relative_block_y, relative_block_x);
	}
}

void save_level()
{
	int i;
	
	file = fopen("level.asciilvl", "wb");
	
	for (i = 0; i < MAP_SIZE; i++) {
		fputc(map[i], file);
	}
	
	fclose(file);
}

void load_level()
{
	int i;
	
	file = fopen("level.asciilvl", "rb");
	
	for (i = 0; i < MAP_SIZE; i++) {
		map[i] = fgetc(file);
	}
	
	fclose(file);
	
	draw_map();
}

void option(int i)
{
	switch (i) {
		case 2:
			load_level();
			break;
		case 3:
			save_level();
			break;
		case 7:
			solidity = !solidity;
			break;
		case 8:
			warps = !warps;
	}
	
	draw_ui();
	move(cursor_y, cursor_x);
}

void options_menu()
{
	int i;
	int j = 0;
	int k;
	
	clear();
	
	while (1) {
		for (i = 0; i < OPTIONS; i++) {
			if (i == j) attron(A_REVERSE);
			mvprintw(i, 0, "%s\n", options_list[i]);
			if (i == j) attroff(A_REVERSE);
		}
		
		k = getch();
		
		switch (k) {
			case 'w':
			case KEY_UP:
				j--;
				if (j == -1)
					j = OPTIONS - 1;
				break;
			case 's':
			case KEY_DOWN:
				j++;
				if (j == OPTIONS)
					j = 0;
				break;
			case '\n':
				option(j);
				return;
		}
	}
}

int main() {
	srand(time(NULL));
	map = calloc(MAP_SIZE, sizeof (uint8_t));

	initscr();
	cbreak();
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);

	init_colour();

	draw_ui();
	
	while (1) {
		getyx(stdscr, cursor_y, cursor_x);
		mvprintw(10, WIDTH + 4, "X: %i\n", x_pos);
		mvprintw(11, WIDTH + 4, "Y: %i\n", y_pos);
		
		attron(COLOR_PAIR(held_block));
		mvaddch(13, WIDTH + 19, block[held_block]);
		attroff(COLOR_PAIR(held_block));
		
		mvprintw(14, WIDTH + 4, "Block Name: %s\n", block_name[held_block]);
		mvprintw(15, WIDTH + 4, "Block ID: %i\n", held_block);
		mvprintw(16, WIDTH + 4, "Block Solid Status: %s\n", block_solid_status[held_block] ? "Solid" : "Non-Solid");
		
		mvprintw(18, WIDTH + 4, "Solidity: %s\n", solidity ? "Enabled" : "Disabled");
		mvprintw(19, WIDTH + 4, "Warps: %s\n", warps ? "Enabled" : "Disabled");
		move(cursor_y, cursor_x);
		
		switch (getch()) {
			case 'w':
			case KEY_UP:
				move_player(-1, 0, &y_pos, y_pos > 0, -1);
				break;
			case 's':
			case KEY_DOWN:
				move_player(1, 0, &y_pos, y_pos < HEIGHT - 1, 1);
				break;
			case 'a':
			case KEY_LEFT:
				move_player(0, -1, &x_pos, x_pos > 0, -1);
				break;
			case 'd':
			case KEY_RIGHT:
				move_player(0, 1, &x_pos, x_pos < WIDTH - 1, 1);
				break;
			case 'i':
				place_block(-1, 0, y_pos > 0);
				break;
			case 'k':
				place_block(1, 0, y_pos < HEIGHT - 1);
				break;
			case 'j':
				place_block(0, -1, x_pos > 0);
				break;
			case 'l':
				place_block(0, 1, x_pos < WIDTH - 1);
				break;
			case 'f':
				held_block = (held_block - 1 + BLOCKS) % BLOCKS;
				break;
			case 'h':
				held_block = (held_block + 1 + BLOCKS) % BLOCKS;
				break;
			case 'm':
				options_menu();
				break;
		}
	}

	endwin();

	free(map);
	return 0;
}
