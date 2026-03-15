// Copyright 2026 Hayden
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

#define VERSION "1.6.0-alpha.1"

#define C_BORDER '+'
#define H_BORDER '-'
#define V_BORDER '|'

#define BLOCKS 39

#define DEFAULT_WIDTH 96
#define DEFAULT_HEIGHT 28

#define MAP_SIZE width * height
#define BLOCK(Y, X) map[Y * width + X]
#define PLACE_BLOCK(Y, X) BLOCK(Y, X) ? 0 : held_block

#define LEVEL_SIGNATURE "ASCIIBLOCKS"
#define OPTIONS 18
#define LEVELS 8

#define NONSOLID 0
#define SOLID 1
#define BARRIER 2
#define WARP 3
#define SPAWN_WARP 4
#define RANDOM_WARP 5
#define FILEWARP 6
#define ZIPWIRE_UP 7
#define ZIPWIRE_DOWN 8
#define ZIPWIRE_LEFT 9
#define ZIPWIRE_RIGHT 10
#define COIN 11

#define BLOCK_TYPES 12

#define IS_ENABLED(CONDITION) CONDITION ? "Enabled" : "Disabled"

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
char username[33] = "Max";
char skin = 'X';
unsigned int coin_count = 0;
bool challenge = 0;
char block[BLOCKS] = {' ', '#', '%', '&', '$', '*', '.', '@', '+',
					  '[', ']', '~', '-', '=', '_', '!', '1', '2',
					  '3', '4', '5', '6', '7', '8', '0', '?', '1',
					  '2', '3', '4', '5', '6', '7', '8', '^', 'v',
					  '<', '>', 'o'};
char* block_name[BLOCKS] = {"Air",
							"Wood",
							"Leaves",
							"Grass",
							"Stone",
							"Bedrock",
							"Pebble",
							"Wool",
							"Fence",
							"Left-Facing Door",
							"Right-Facing Door",
							"Vines",
							"Path",
							"Gate",
							"Carpet",
							"Barrier",
							"Warp 1",
							"Warp 2",
							"Warp 3",
							"Warp 4",
							"Warp 5",
							"Warp 6",
							"Warp 7",
							"Warp 8",
							"Warp to Spawn",
							"Warp to Random Co-ords",
							"Filewarp 1",
							"Filewarp 2",
							"Filewarp 3",
							"Filewarp 4",
							"Filewarp 5",
							"Filewarp 6",
							"Filewarp 7",
							"Filewarp 8",
							"Zipwire Up",
							"Zipwire Down",
							"Zipwire Left",
							"Zipwire Right",
							"Coin"};
uint8_t block_type[BLOCKS] = {NONSOLID, SOLID, NONSOLID, NONSOLID, SOLID, SOLID, NONSOLID, SOLID, SOLID, NONSOLID,
							  NONSOLID, NONSOLID, NONSOLID, NONSOLID, NONSOLID, BARRIER, WARP, WARP, WARP, WARP,
							  WARP, WARP, WARP, WARP, SPAWN_WARP, RANDOM_WARP, FILEWARP, FILEWARP, FILEWARP, FILEWARP,
							  FILEWARP, FILEWARP, FILEWARP, FILEWARP, ZIPWIRE_UP, ZIPWIRE_DOWN, ZIPWIRE_LEFT, ZIPWIRE_RIGHT, COIN};
char *block_type_name[BLOCKS] = {"Non-Solid",
								 "Solid",
									"Barrier",
								   "Warp",
								   "Warp to Spawn",
								   "Warp to Random Co-ords",
								   "Filewarp",
								   "Zipwire Up",
								   "Zipwire Down",
								   "Zipwire Left",
								   "Zipwire Right",
								   "Coin"};

bool text = true;
bool painting = false;
bool solidity = true;
bool warps = true;
bool zipwires = true;
bool coins = true;

char *option_list[OPTIONS] = {"Back",
							  "New Level",
							  "Load Level from LEVEL.ABL",
							  "Load Level",
							  "Save Level",
							  "Teleport",
							  "Teleport Relative",
							  "Toggle Painting",
							  "Toggle Solidity",
							  "Toggle Warps",
							  "Toggle Zipwires",
							  "Change Type of Held Block",
							  "Toggle Text",
							  "Toggle Gamemode",
							  "Replace All Instances of One Block with Held Block",
							  "Fill Level with Held Block",
							  "Change Username",
							  "Change Skin"};

char *level_list[LEVELS] = {"level1.asciilvl",
							"level2.asciilvl",
							"level3.asciilvl",
							"level4.asciilvl",
							"level5.asciilvl",
							"level6.asciilvl",
							"level7.asciilvl",
							"level8.asciilvl"};

const char* is_enabled(bool condition);
void init_colour();
void draw_borders();
void draw_map();
void draw_ui();
void new_level();
void load_level(int level);
void save_level(int level);
void load_abl();
void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps, bool respect_zipwires);
void teleport(bool relative);
void warp(uint8_t warp_id);
void place_block(int relative_block_y, int relative_block_x, bool condition);
void set_block(int set_block_y, int set_block_x, bool remove_block_if_present);
void replace_all();
void fill_all();
void change_username();
void option(int i);
void options_menu();
void level_menu(bool save);
int main(void);

const char* is_enabled(bool condition)
{
	return condition ? "Enabled" : "Disabled";
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
	init_pair(30, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(31, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(32, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(33, COLOR_BLACK,   COLOR_YELLOW);
	init_pair(34, COLOR_GREEN,   COLOR_BLACK);
	init_pair(35, COLOR_GREEN,   COLOR_BLACK);
	init_pair(36, COLOR_GREEN,   COLOR_BLACK);
	init_pair(37, COLOR_GREEN,   COLOR_BLACK);
	init_pair(38, COLOR_YELLOW,  COLOR_BLACK);

	init_pair(128, COLOR_MAGENTA, COLOR_BLACK);
	// colour of player character
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
		if (!(block_type[map[i]] == BARRIER && challenge)) {
			attron(COLOR_PAIR(map[i]));
			mvaddch((i / width) + 1, (i % width) + 1, block[map[i]]);
			attroff(COLOR_PAIR(map[i]));
		}
	}
}

void load_level(int level)
{
	if (level >= 0 && level < LEVELS) file = fopen(level_list[level], "rb");
	else return;
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
	else return;

	for (int i = 0; i < MAP_SIZE; i++) {
		fputc(map[i], file);
	}
	
	fclose(file);
}

void load_abl()
{
	file = fopen("LEVEL.ABL", "rb");
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
	if ((block_type[BLOCK(tp_y, tp_x)] == SOLID || block_type[BLOCK(tp_y, tp_x)] == BARRIER) && respect_solidity) return;

	attron(COLOR_PAIR(BLOCK(y, x)));
	mvaddch(cursor_y, cursor_x - 1, block[BLOCK(y, x)]);
	attroff(COLOR_PAIR(BLOCK(y, x)));
	attron(COLOR_PAIR(128));
	mvaddch(tp_y + 1, tp_x + 1, skin);
	attroff(COLOR_PAIR(128));

	y = tp_y;
	x = tp_x;

	if (respect_warps) {
		switch (block_type[BLOCK(y, x)]) {
			case WARP:
				warp(BLOCK(y, x));
				break;
			case SPAWN_WARP:
				tp(spawn_y, spawn_x, false, warps, zipwires);
				break;
			case RANDOM_WARP:
				tp(rand() % height, rand() % width, false, warps, zipwires);
				break;
		}
	}

	if (respect_zipwires) {
		switch (block_type[BLOCK(y, x)]) {
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

	if (coins && block_type[BLOCK(y, x)] == COIN) {
		coin_count++;
		BLOCK(y, x) = 0;
	}
}

void handle_zipwires()
{
	switch (BLOCK(y, x)) {
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
			if (BLOCK(i, j) == warp_id && (i != y || j != x)) {
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

void set_block(int block_y, int block_x, bool remove_block_if_present)
{
	if (block_y < 0 || block_y >= height) return;
	if (block_x < 0 || block_x >= width) return;

	attron(COLOR_PAIR(remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : block[held_block]));
	mvaddch(block_y + 1, block_x + 1, remove_block_if_present ? block[PLACE_BLOCK(block_y, block_x)] : block[held_block]);
	attroff(COLOR_PAIR(remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : block[held_block]));
	
	move(cursor_y, cursor_x);
	
	BLOCK(block_y, block_x) = remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : held_block;
}

void new_level()
{
	free(map);

	clear();
	curs_set(1);
	echo();

	printw("Leave the width or height blank to accept their default values.\n\n");

	width = DEFAULT_WIDTH;
	printw("Width (%d): ", DEFAULT_WIDTH);
	scanw(" %d", &width);

	height = DEFAULT_HEIGHT;
	printw("Height (%d): ", DEFAULT_HEIGHT);
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
	printw("Block ID to replace: ");
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

void change_username()
{
	clear();
	curs_set(1);
	echo();
	
	printw("Username: ");
	scanw(" %32s", &username);
	
	curs_set(0);
	noecho();
	
	draw_ui();
}

void change_skin()
{
	clear();
	curs_set(1);
	echo();
	
	printw("Skin: ");
	scanw(" %c", &skin);
	
	curs_set(0);
	noecho();
	
	draw_ui();
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
			level_menu(false);
			break;
		case 4:
			level_menu(true);
			break;
		case 5:
			teleport(false);
			break;
		case 6:
			teleport(true);
			break;
		case 7:
			painting ^= 1;
			break;
		case 8:
			solidity ^= 1;
			break;
		case 9:
			warps ^= 1;
			break;
		case 10:
			zipwires ^= 1;
			break;
		case 11:
			//block_solid_status[held_block] ^= 1;
			break;
		case 12:
			text ^= 1;
			break;
		case 13:
			challenge ^= 1;
			break;
		case 14:
			replace_all();
			break;
		case 15:
			fill_all();
			break;
		case 16:
			change_username();
			break;
		case 17:
			change_skin();
			break;
	}
	
	draw_ui();
}

void options_menu()
{
	int j = 0;
	
	clear();
	
	while (true) {
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

void level_menu(bool save)
{
	int j = 0;
	
	clear();
	
	while (true) {
		for (int i = 0; i < LEVELS; i++) {
			if (i == j) attron(A_REVERSE);
			mvprintw(i, 0, "%s\n", level_list[i]);
			if (i == j) attroff(A_REVERSE);
		}
		
		switch (getch()) {
			case 'w':
			case KEY_UP:
				j--;
				if (j == -1) j = LEVELS - 1;
				break;
			case 's':
			case KEY_DOWN:
				j++;
				if (j == LEVELS) j = 0;
				break;
			case '\n':
				if (save) save_level(j);
				else load_level(j);
				return;
		}
	}
}
// TODO consolidate options_menu and level_menu into one function that can be reused for both those purposes

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
	
	while (true) {
		getyx(stdscr, cursor_y, cursor_x);

		if (text) {
			mvprintw(10, width + 4, "X: %d\n", x);
			mvprintw(11, width + 4, "Y: %d\n", y);
		
			mvprintw(13, width + 4, "Spawn X: %d\n", spawn_x);
			mvprintw(14, width + 4, "Spawn Y: %d\n", spawn_y);
		
			attron(COLOR_PAIR(held_block));
			mvaddch(16, width + 19, block[held_block]);
			attroff(COLOR_PAIR(held_block));

			mvprintw(17, width + 4, "Block ID: %d\n", held_block);
			mvprintw(18, width + 4, "Block Name: %s\n", block_name[held_block]);
			mvprintw(19, width + 4, "Block Type: %s\n", block_type_name[block_type[held_block]]);

			mvprintw(21, width + 4, "Painting: %s\n", is_enabled(painting));		
			mvprintw(22, width + 4, "Solidity: %s\n", is_enabled(solidity));
			mvprintw(23, width + 4, "Warps: %s\n", is_enabled(warps));
			mvprintw(24, width + 4, "Zipwires: %s\n", is_enabled(zipwires));

			mvprintw(26, width + 4, "Username: %s\n", username);
			mvprintw(27, width + 4, "Game Mode: %s\n", challenge ? "Challenge" : "Build");
			mvprintw(28, width + 4, "Coins: %u\n", coin_count);

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
				if (!challenge) set_block(y - 1, x, !painting);
				break;
			case 'k':
				if (!challenge) set_block(y + 1, x, !painting);
				break;
			case 'j':
				if (!challenge) set_block(y, x - 1, !painting);
				break;
			case 'l':
				if (!challenge) set_block(y, x + 1, !painting);
				break;
			case '\n':
				if (!challenge) {
					spawn_y = y;
					spawn_x = x;
				}
				break;
			case 'r':
				if (!challenge) tp(spawn_y, spawn_x, false, warps, zipwires);
				break;
			case 'f':
				if (!challenge) held_block = (held_block - 1 + BLOCKS) % BLOCKS;
				break;
			case 'h':
				if (!challenge) held_block = (held_block + 1) % BLOCKS;
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
