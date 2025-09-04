// Copyright 2025 Hayden
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ncurses.h>
#include <time.h>

#define C_BORDER '+'
#define H_BORDER '-'
#define V_BORDER '|'

#define PLAYER 'X'
#define BLOCKS 25

#define WARP_1 15
#define WARP_2 16
#define WARP_3 17
#define WARP_4 18
#define WARP_TO_SPAWN 19
#define WARP_TO_RANDOM_COORDS 20
#define FILEWARP_1 21
#define FILEWARP_2 22
#define FILEWARP_3 23
#define FILEWARP_4 24

#define DEFAULT_WIDTH 96
#define DEFAULT_HEIGHT 28

#define MAP_SIZE width + (height - 1) * width

#define RELATIVE_BLOCK(Y, X) map[(y + Y) * width + (x + X)]
#define PLACE_RELATIVE_BLOCK(Y, X) (RELATIVE_BLOCK(Y, X) == 0) ? held_block : 0
#define BLOCK_AT(Y, X) map[Y * width + X]

#define LEVEL_SIGNATURE "ASCIIBLOCKS"
#define OPTIONS 11

int width = 96;
int height = 28;

int cursor_y = 1;
int cursor_x = 2;

int y = 0;
int x = 0;

int spawn_y = 0;
int spawn_x = 0;

FILE* file;
uint8_t* map;
uint8_t held_block = 1;
char block[BLOCKS] = {' ', '#', '%', '&', '$', '*', '@', '+', '[',
					  ']', '~', '-', '=', '_', '!', '1', '2', '3',
					  '4', '0', '?', '1', '2', '3', '4'};
char* block_name[BLOCKS] = {"Air", "Wood", "Leaves", "Grass", "Stone", "Bedrock", "Wool", "Fence",
							"Left-Facing Door", "Right-Facing Door", "Vines", "Path", "Entrance", "Carpet", "Barrier",
							"Warp 1", "Warp 2", "Warp 3", "Warp 4", "Warp to Spawn", "Warp to Random Co-ords", "Filewarp 1", "Filewarp 2", "Filewarp 3", "Filewarp 4"};
bool block_solid_status[BLOCKS] = {false, true, true, true, true, true, true, true, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false};

bool solidity = true;
bool painting = false;
bool warps = true;

char *options_list[OPTIONS] = {"Back",
							   "New Level",
							   "Load Level",
							   "Save Level",
							   "Teleport",
							   "Teleport Relative",
							   "Toggle Painting",
							   "Toggle Solidity",
							   "Toggle Warps",
							   "Toggle Solid Status of Held Block",
							   "Roll a Dice"};

void toggle(bool *boolean)
{
	*boolean = !(*boolean);
}

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
	init_pair(21, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(22, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(23, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(24, COLOR_BLACK,   COLOR_YELLOW);
}

void draw_borders()
{
	int i;
	
	mvaddch(0, 0, C_BORDER);
	mvaddch(0, width + 1, C_BORDER);
	mvaddch(height + 1, 0, C_BORDER);
	mvaddch(height + 1, width + 1, C_BORDER);
	
	for (i = 1; i <= width; i++) {
		mvaddch(0, i, H_BORDER);
		mvaddch(height + 1, i, H_BORDER);
	}

	for (i = 1; i <= height; i++) {
		mvaddch(i, 0, V_BORDER);
		mvaddch(i, width + 1, V_BORDER);
	}
}

void draw_map()
{
	for (int i = 0; i < MAP_SIZE; i++) {
		attron(COLOR_PAIR(map[i]));
		mvaddch((i / width) + 1, (i % width) + 1, block[map[i]]);
		attroff(COLOR_PAIR(map[i]));
	}
}

void save_level(int level)
{
	int i;
	
	if (level == 1) file = fopen("level1.asciilvl", "wb");
	if (level == 2) file = fopen("level2.asciilvl", "wb");
	if (level == 3) file = fopen("level3.asciilvl", "wb");
	if (level == 4) file = fopen("level4.asciilvl", "wb");
	
	for (i = 0; i < MAP_SIZE; i++) {
		fputc(map[i], file);
	}
	
	fclose(file);
}

void load_level(int level)
{
	if (level == 1) file = fopen("level1.asciilvl", "rb");
	if (level == 2) file = fopen("level2.asciilvl", "rb");
	if (level == 3) file = fopen("level3.asciilvl", "rb");
	if (level == 4) file = fopen("level4.asciilvl", "rb");
	
	if (!file) return;
	
	for (int i = 0; i < MAP_SIZE; i++) {
		map[i] = fgetc(file);
	}
	
	fclose(file);
	
	//tp(0, 0, false, false);
	draw_map();
}

void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps)
{
	if (tp_y < 0 || tp_y >= height) return;
	if (tp_x < 0 || tp_x >= width) return;
	if (block_solid_status[BLOCK_AT(tp_y, tp_x)] && solidity && respect_solidity) return;

	if (BLOCK_AT(tp_y, tp_x) == FILEWARP_1 && respect_warps) load_level(1);
	if (BLOCK_AT(tp_y, tp_x) == FILEWARP_2 && respect_warps) load_level(2);
	if (BLOCK_AT(tp_y, tp_x) == FILEWARP_3 && respect_warps) load_level(3);
	if (BLOCK_AT(tp_y, tp_x) == FILEWARP_4 && respect_warps) load_level(4);

	attron(COLOR_PAIR(BLOCK_AT(y, x)));
	mvaddch(cursor_y, cursor_x - 1, block[BLOCK_AT(y, x)]);
	attroff(COLOR_PAIR(BLOCK_AT(y, x)));
	mvaddch(tp_y + 1, tp_x + 1, PLAYER);

	y = tp_y;
	x = tp_x;
}

void draw_ui()
{
	clear();

	mvprintw(0, width + 4, "ASCIIBlocks 1.5.0-alpha.5\n");
	mvprintw(1, width + 4, "by Hayden\n");
	
	mvprintw(3, width + 4, "Use WASD to move,\n");
	mvprintw(4, width + 4, "IJKL to place and remove blocks\n");
	mvprintw(5, width + 4, "and F or H to select blocks.\n");
	
	mvprintw(7, width + 4, "Use M to open the menu\n");
	mvprintw(8, width + 4, "and CTRL-C to quit the game.\n");
	
	mvprintw(16, width + 4, "Block Texture:\n");

	draw_borders();
	draw_map();
	
	tp(y, x, false, false);
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

void set_block(int set_block_y, int set_block_x, bool remove_block_if_present)
{
	if (set_block_y < 0 || set_block_y >= height) return;
	if (set_block_x < 0 || set_block_x >= width) return;
	
}

void roll_a_dice()
{
	clear();

	printw("You rolled a %d.\n", rand() % 6 + 1);
	getch();

	draw_ui();
}

void new_level()
{
	free(map);

	clear();
	curs_set(1);
	echo();

	printw("Press Return for the width or height to accept their default values.\n\n");

	width = DEFAULT_WIDTH;
	printw("width (default %d): ", DEFAULT_WIDTH);
	scanw(" %d", &width);

	height = DEFAULT_HEIGHT;
	printw("height (default %d): ", DEFAULT_HEIGHT);
	scanw(" %d", &height);

	curs_set(0);
	noecho();

	map = calloc(MAP_SIZE, sizeof (uint8_t));
	
	draw_ui();
	tp(0, 0, false, false);
}

void option(int i)
{
	switch (i) {
		case 1:
			new_level();
			break;
		case 2:
			load_level(1);
			break;
		case 3:
			save_level(1);
			break;
		case 6:
			toggle(&painting);
			break;
		case 7:
			toggle(&solidity);
			break;
		case 8:
			toggle(&warps);
			break;
		case 9:
			toggle(&block_solid_status[held_block]);
			break;
		case 10:
			roll_a_dice();
			break;
	}
	
	draw_ui();
	move(cursor_y, cursor_x);
}

void options_menu()
{
	int j = 0;
	
	clear();
	
	while (1) {
		for (int i = 0; i < OPTIONS; i++) {
			if (i == j) attron(A_REVERSE);
			mvprintw(i, 0, "%s\n", options_list[i]);
			if (i == j) attroff(A_REVERSE);
		}
		
		switch (getch()) {
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
		mvprintw(10, width + 4, "X: %i\n", x);
		mvprintw(11, width + 4, "Y: %i\n", y);
		
		mvprintw(13, width + 4, "Spawn X: %i\n", spawn_x);
		mvprintw(14, width + 4, "Spawn Y: %i\n", spawn_y);
		
		attron(COLOR_PAIR(held_block));
		mvaddch(16, width + 19, block[held_block]);
		attroff(COLOR_PAIR(held_block));

		mvprintw(17, width + 4, "Block ID: %i\n", held_block);
		mvprintw(18, width + 4, "Block Name: %s\n", block_name[held_block]);
		mvprintw(19, width + 4, "Block Solid Status: %s\n", block_solid_status[held_block] ? "Solid" : "Non-Solid");

		mvprintw(21, width + 4, "Painting: %s\n", painting ? "Enabled" : "Disabled");		
		mvprintw(22, width + 4, "Solidity: %s\n", solidity ? "Enabled" : "Disabled");
		mvprintw(23, width + 4, "Warps: %s\n", warps ? "Enabled" : "Disabled");
		move(cursor_y, cursor_x);
		
		switch (getch()) {
			case 'w':
			case KEY_UP:
				tp(y - 1, x, true, true);
				break;
			case 's':
			case KEY_DOWN:
				tp(y + 1, x, true, true);
				break;
			case 'a':
			case KEY_LEFT:
				tp(y, x - 1, true, true);
				break;
			case 'd':
			case KEY_RIGHT:
				tp(y, x + 1, true, true);
				break;
			case 'i':
				place_block(-1, 0, y > 0);
				break;
			case 'k':
				place_block(1, 0, y < height - 1);
				break;
			case 'j':
				place_block(0, -1, x > 0);
				break;
			case 'l':
				place_block(0, 1, x < width - 1);
				break;
			case '\n':
				spawn_y = y;
				spawn_x = x;
				break;
			case 'r':
				tp(spawn_y, spawn_x, false, true);
				break;
			case 'f':
				held_block = (held_block - 1 + BLOCKS) % BLOCKS;
				break;
			case 'h':
				held_block = (held_block + 1) % BLOCKS;
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
