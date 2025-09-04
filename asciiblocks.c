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
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
	#include <curses.h>
	#define NAME "winASCIIBlocks"
#else
	#include <ncurses.h>
	#define NAME "ASCIIBlocks"
#endif

#define VERSION "1.5.0-alpha.11"

#define C_BORDER '+'
#define H_BORDER '-'
#define V_BORDER '|'

#define PLAYER 'X'
#define BLOCKS 34

#define WARP_1 16
#define WARP_2 17
#define WARP_3 18
#define WARP_4 19
#define WARP_5 20
#define WARP_6 21
#define WARP_7 22
#define WARP_8 23
#define FILEWARP_1 26
#define FILEWARP_2 27
#define FILEWARP_3 28
#define FILEWARP_4 29
#define WARP_SPAWN 24
#define WARP_RANDOM 25

#define ZIPWIRE_UP 30
#define ZIPWIRE_DOWN 31
#define ZIPWIRE_LEFT 32
#define ZIPWIRE_RIGHT 33

#define DEFAULT_WIDTH 96
#define DEFAULT_HEIGHT 28

#define MAP_SIZE width * height

#define RELATIVE_BLOCK(Y, X) map[(y + Y) * width + (x + X)]
#define PLACE_RELATIVE_BLOCK(Y, X) (RELATIVE_BLOCK(Y, X) == 0) ? held_block : 0
#define BLOCK_AT(Y, X) map[Y * width + X]

#define LEVEL_SIGNATURE "ASCIIBLOCKS"
#define OPTIONS 21
#define LEVELS 4

int width = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;

int y = 0;
int x = 0;

int cursor_y = 1;
int cursor_x = 2;

int spawn_y = 0;
int spawn_x = 0;

FILE* file;
uint8_t* map;
uint8_t held_block = 1;
char block[BLOCKS] = {' ', '#', '%', '&', '$', '*', '.', '@', '+',
					  '[', ']', '~', '-', '=', '_', '!', '1', '2',
					  '3', '4', '5', '6', '7', '8', '0', '?', '1',
					  '2', '3', '4', '^', 'v', '<', '>'};
char* block_name[BLOCKS] = {"Air", "Wood", "Leaves", "Grass", "Stone", "Bedrock", "Pebble", "Wool", "Fence",
							"Left-Facing Door", "Right-Facing Door", "Vines", "Path", "Gate", "Carpet", "Barrier", "Warp 1",
							"Warp 2", "Warp 3", "Warp 4", "Warp 5", "Warp 6", "Warp 7", "Warp 8", "Warp to Spawn",
							"Warp to Random Co-ords", "Filewarp 1", "Filewarp 2", "Filewarp 3", "Filewarp 4", "Zipwire Up", "Zipwire Down", "Zipwire Left",
							"Zipwire Right"};
bool block_solid_status[BLOCKS] = {false, true, false, false, true, true, false, true, true, false,
								   false, false, false, false, false, true, false, false, false,
								   false, false, false, false, false, false, false, false, false,
								   false, false, false, false, false, false};

bool text = true;
bool painting = false;
bool solidity = true;
bool warps = true;
bool zipwires = true;

char *option_list[OPTIONS] = {"Back",
							  "New Level",
							  "Load Level from level.abl",
							  "Load Level from level1.asciilvl",
							  "Load Level from level2.asciilvl",
							  "Load Level from level3.asciilvl",
							  "Load Level from level4.asciilvl",
							  "Save Level to level1.asciilvl",
							  "Save Level to level2.asciilvl",
							  "Save Level to level3.asciilvl",
							  "Save Level to level4.asciilvl",
							  "Teleport",
							  "Teleport Relative",
							  "Toggle Painting",
							  "Toggle Solidity",
							  "Toggle Warps",
							  "Toggle Zipwires",
							  "Toggle Solid Status of Held Block",
							  "Toggle Text",
							  "Replace All Instances of One Block with Held Block",
							  "Fill Level with Held Block"};

char *level_list[LEVELS] = {"level1.asciilvl",
							"level2.asciilvl",
							"level3.asciilvl",
							"level4.asciilvl"};

void toggle(bool *boolean);
void init_colour();
void draw_borders();
void draw_map();
void draw_ui();
void new_level();
void load_level(int level);
void save_level(int level);
void load_abl();
void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps, bool respect_zipwires);
void handle_warps();
void handle_zipwires();
void teleport(bool relative);
void warp(uint8_t warp_id);
void place_block(int relative_block_y, int relative_block_x, bool condition);
void set_block(int set_block_y, int set_block_x, bool remove_block_if_present);
void replace_all();
void fill_all();
void option(int i);
void options_menu();
int main(void);

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
	init_pair(7,  COLOR_WHITE,   COLOR_BLACK);
	init_pair(8,  COLOR_YELLOW,  COLOR_BLACK);
	init_pair(9,  COLOR_RED,     COLOR_BLACK);
	init_pair(10, COLOR_BLUE,    COLOR_BLACK);
	init_pair(11, COLOR_GREEN,   COLOR_BLACK);
	init_pair(12, COLOR_GREEN,   COLOR_YELLOW);
	init_pair(13, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(14, COLOR_WHITE,   COLOR_BLACK);
	init_pair(15, COLOR_BLACK,   COLOR_RED);
	init_pair(16, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(17, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(18, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(19, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(20, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(21, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(22, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(23, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(24, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(25, COLOR_BLACK,   COLOR_MAGENTA);
	init_pair(26, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(27, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(28, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(29, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(30, COLOR_GREEN,   COLOR_BLACK);
	init_pair(31, COLOR_GREEN,   COLOR_BLACK);
	init_pair(32, COLOR_GREEN,   COLOR_BLACK);
	init_pair(33, COLOR_GREEN,   COLOR_BLACK);
}

void draw_borders()
{
	mvaddch(0, 0, C_BORDER);
	mvaddch(0, width + 1, C_BORDER);
	mvaddch(height + 1, 0, C_BORDER);
	mvaddch(height + 1, width + 1, C_BORDER);
	
	for (int i = 1; i <= width; i++) {
		mvaddch(0, i, H_BORDER);
		mvaddch(height + 1, i, H_BORDER);
	}

	for (int i = 1; i <= height; i++) {
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

void load_level(int level)
{
	if (level >= 0 && level < LEVELS) file = fopen(level_list[level], "rb");
	if (!file) return;
	
	for (int i = 0; i < MAP_SIZE; i++) {
		map[i] = fgetc(file);
	}
	
	fclose(file);
	
	draw_map();
	tp(0, 0, false, false, false);
}

void save_level(int level)
{
	if (level >= 0 && level < LEVELS) file = fopen(level_list[level], "wb");

	for (int i = 0; i < MAP_SIZE; i++) {
		fputc(map[i], file);
	}
	
	fclose(file);
}

void load_abl()
{
	file = fopen("level.abl", "rb");
	if (!file) return;

	free(map);

	width = 32;
	height = 16;

	map = calloc(MAP_SIZE, sizeof (uint8_t));

	for (int i = 0; i < MAP_SIZE; i++) {
		switch (fgetc(file)) {
			case '.':
				map[i] = 6;
				break;
			case '#':
				map[i] = 1;
				break;
			case '@':
				map[i] = 7;
				break;
			case '*':
				map[i] = 5;
				break;
			case '&':
				map[i] = 3;
				break;
			case 'O':
				map[i] = 4;
				break;
			case '[':
				map[i] = 9;
				break;
			case ']':
				map[i] = 10;
				break;
			case '-':
				map[i] = 12;
				break;
			case '~':
				map[i] = 11;
				break;
			case '=':
				map[i] = 13;
				break;
		}
	}

	fclose(file);

	draw_ui();
	tp(0, 0, false, false, false);
}

void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps, bool respect_zipwires)
{
	getyx(stdscr, cursor_y, cursor_x);

	if (tp_y < 0 || tp_y >= height) return;
	if (tp_x < 0 || tp_x >= width) return;
	if (block_solid_status[BLOCK_AT(tp_y, tp_x)] && respect_solidity) return;

	attron(COLOR_PAIR(BLOCK_AT(y, x)));
	mvaddch(cursor_y, cursor_x - 1, block[BLOCK_AT(y, x)]);
	attroff(COLOR_PAIR(BLOCK_AT(y, x)));
	mvaddch(tp_y + 1, tp_x + 1, PLAYER);

	y = tp_y;
	x = tp_x;

	if (respect_warps) handle_warps();
	if (respect_zipwires) handle_zipwires();
}

void handle_warps()
{
	switch (BLOCK_AT(y, x)) {
		case WARP_SPAWN:
			tp(spawn_y, spawn_x, false, warps, zipwires);
			break;
		case WARP_RANDOM:
			tp(rand() % height, rand() % width, false, warps, zipwires);
			break;
		case WARP_1:
			warp(WARP_1);
			break;
		case WARP_2:
			warp(WARP_2);
			break;
		case WARP_3:
			warp(WARP_3);
			break;
		case WARP_4:
			warp(WARP_4);
			break;
		case WARP_5:
			warp(WARP_5);
			break;
		case WARP_6:
			warp(WARP_6);
			break;
		case WARP_7:
			warp(WARP_7);
			break;
		case WARP_8:
			warp(WARP_8);
			break;
		case FILEWARP_1:
			load_level(0);
			break;
		case FILEWARP_2:
			load_level(1);
			break;
		case FILEWARP_3:
			load_level(2);
			break;
		case FILEWARP_4:
			load_level(3);
			break;
	}
}

void handle_zipwires()
{
	switch (BLOCK_AT(y, x)) {
		case ZIPWIRE_UP:
			tp(y - 1, x, false, warps, zipwires);
			break;
		case ZIPWIRE_DOWN:
			tp(y + 1, x, false, warps, zipwires);
			break;
		case ZIPWIRE_LEFT:
			tp(y, x - 1, false, warps, zipwires);
			break;
		case ZIPWIRE_RIGHT:
			tp(y, x + 1, false, warps, zipwires);
			break;
	}
}

void teleport(bool relative)
{
	clear();
	curs_set(1);
	echo();

	int new_x = x;
	printw("X: %s", relative ? "~" : "");
	scanw(" %d", &new_x);

	int new_y = y;
	printw("Y: %s", relative ? "~" : "");
	scanw(" %d", &new_y);

	curs_set(0);
	noecho();

	draw_ui();

	if (relative) {
		tp(new_y + y, new_x + x, false, warps, zipwires);
	} else {
		tp(new_y, new_x, false, warps, zipwires);
	}
}

void warp(uint8_t warp_id)
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			if (BLOCK_AT(i, j) == warp_id && (i != y || j != x)) {
				tp(i, j, false, false, false);
				return;
			}
		}
	}
}

void draw_ui()
{
	clear();

	if (text) {
		mvprintw(0, width + 4, "%s %s\n", NAME, VERSION);
		mvprintw(1, width + 4, "by Hayden\n");
	
		mvprintw(3, width + 4, "Use WASD to move,\n");
		mvprintw(4, width + 4, "IJKL to place and remove blocks\n");
		mvprintw(5, width + 4, "and F or H to select blocks.\n");
	
		mvprintw(7, width + 4, "Use M to open the menu\n");
		mvprintw(8, width + 4, "and CTRL-C to quit the game.\n");
	
		mvprintw(16, width + 4, "Block Texture:\n");
	}

	draw_borders();
	draw_map();
	
	tp(y, x, false, false, false);
}

void place_block(int relative_block_y, int relative_block_x, bool condition)
{
	if (condition) {
		if (painting) {
			attron(COLOR_PAIR(held_block));
			mvaddch(cursor_y + relative_block_y, cursor_x + relative_block_x - 1, block[held_block]);
			attroff(COLOR_PAIR(held_block));
		} else {
			attron(COLOR_PAIR(RELATIVE_BLOCK(relative_block_y, relative_block_x) ? 0 : held_block));
			mvaddch(cursor_y + relative_block_y, cursor_x + relative_block_x - 1, block[PLACE_RELATIVE_BLOCK(relative_block_y, relative_block_x)]);
			attroff(COLOR_PAIR(RELATIVE_BLOCK(relative_block_y, relative_block_x) ? 0 : held_block));
		}
		
		move(cursor_y, cursor_x);

		if (painting) {
			RELATIVE_BLOCK(relative_block_y, relative_block_x) = held_block;
		} else {
			RELATIVE_BLOCK(relative_block_y, relative_block_x) = PLACE_RELATIVE_BLOCK(relative_block_y, relative_block_x);
		}
	}
}

void set_block(int set_block_y, int set_block_x, bool remove_block_if_present)
{
	if (set_block_y < 0 || set_block_y >= height) return;
	if (set_block_x < 0 || set_block_x >= width) return;
	
}

void new_level()
{
	free(map);

	clear();
	curs_set(1);
	echo();

	printw("Leave the width or height blank to accept their default values.\n\n");

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

	tp(0, 0, false, false, false);
}

void replace_all()
{
	clear();
	curs_set(1);
	echo();

	uint8_t id_to_replace = 0;
	printw("block id to replace: ");
	scanw(" %hhu", &id_to_replace);

	for (int i = 0; i < MAP_SIZE; i++) {
		if (map[i] == id_to_replace) map[i] = held_block;
	}

	curs_set(0);
	noecho();

	draw_ui();
}

void fill_all()
{
	for (int i = 0; i < MAP_SIZE; i++) {
		map[i] = held_block;
	}

	draw_map();
}

void option(int i)
{
	switch (i) {
		case 1:
			new_level();
			break;
		case 2:
			load_abl();
			break;
		case 3:
			load_level(0);
			break;
		case 4:
			load_level(1);
			break;
		case 5:
			load_level(2);
			break;
		case 6:
			load_level(3);
			break;
		case 7:
			save_level(0);
			break;
		case 8:
			save_level(1);
			break;
		case 9:
			save_level(2);
			break;
		case 10:
			save_level(3);
			break;
		case 11:
			teleport(false);
			break;
		case 12:
			teleport(true);
			break;
		case 13:
			toggle(&painting);
			break;
		case 14:
			toggle(&solidity);
			break;
		case 15:
			toggle(&warps);
			break;
		case 16:
			toggle(&zipwires);
			break;
		case 17:
			toggle(&block_solid_status[held_block]);
			break;
		case 18:
			toggle(&text);
			break;
		case 19:
			replace_all();
			break;
		case 20:
			fill_all();
			break;
	}
	
	draw_ui();
}

void options_menu()
{
	int j = 0;
	
	clear();
	
	while (1) {
		for (int i = 0; i < OPTIONS; i++) {
			if (i == j) attron(A_REVERSE);
			mvprintw(i, 0, "%s\n", option_list[i]);
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

int main(void)
{
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

		if (text) {
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
			mvprintw(24, width + 4, "Zipwires: %s\n", zipwires ? "Enabled" : "Disabled");

			move(cursor_y, cursor_x);
		}
		
		switch (getch()) {
			case 'w':
			case KEY_UP:
				tp(y - 1, x, solidity, warps, zipwires);
				break;
			case 's':
			case KEY_DOWN:
				tp(y + 1, x, solidity, warps, zipwires);
				break;
			case 'a':
			case KEY_LEFT:
				tp(y, x - 1, solidity, warps, zipwires);
				break;
			case 'd':
			case KEY_RIGHT:
				tp(y, x + 1, solidity, warps, zipwires);
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
				tp(spawn_y, spawn_x, false, warps, zipwires);
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
