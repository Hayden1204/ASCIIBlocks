#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <SDL3_net/SDL_net.h>

#if defined(_WIN32) || defined(_WIN64)
  #include <curses.h>
  #define NAME "winASCIIBlocks"
#else
  #include <ncurses.h>
  #define NAME "ASCIIBlocks"
#endif

#define VERSION "1.6.0-alpha.5"

#define C_BORDER '+'
#define H_BORDER '-'
#define V_BORDER '|'

#define MAP_SIZE width * height
#define BLOCK(Y, X) map[Y * width + X]
#define PLACE_BLOCK(Y, X) (BLOCK(Y, X) ? 0 : held_block)

#define LEVEL_SIGNATURE "ASCIIBLOCKS"

#define OPTION_COUNT 20 // number of options in option menu
#define LEVEL_COUNT 8 // number of levels available to load/save to/from

#define TICK 50 // how often to tick in ms

#define DEFAULT_HEIGHT 28
#define DEFAULT_WIDTH 96

enum BlockTypes {
  TYPE_NONSOLID,
  TYPE_SOLID,
  TYPE_BARRIER,
  TYPE_WARP,
  TYPE_WARP_SPAWN,
  TYPE_WARP_RANDOM,
  TYPE_FILEWARP,
  TYPE_ZIPWIRE_UP,
  TYPE_ZIPWIRE_DOWN,
  TYPE_ZIPWIRE_LEFT,
  TYPE_ZIPWIRE_RIGHT,
  TYPE_COIN,
  TYPE_GATE_1,
  TYPE_GATE_2,
  TYPE_GATE_5,
  TYPE_GATE_10,
  TYPE_GATE_20,
  TYPE_GATE_50,
  TYPE_KEY_RED,
  TYPE_KEY_BLUE,
  TYPE_KEY_GREEN,
  TYPE_COUNT
};

int height = DEFAULT_HEIGHT;
int width = DEFAULT_WIDTH;

int block_count = 54;
const int inventory_row_max = 10;

int cursor_y = 1;
int cursor_x = 2;

unsigned int y = 0;
unsigned int x = 0;

unsigned int spawn_y = 0;
unsigned int spawn_x = 0;

FILE* file;
uint8_t* map;
uint8_t held_block = 1; // Wood

char skin = 'X';
char username[33] = "Max";
char level_name[33] = "Level";

unsigned int coin_count = 0;
unsigned int ticks = 0;

char block[256] = {' ', '#', '%', '&', '$', '*', '.', '@', '@',
                   '@', '@', '@', '@', '@', '+', '[', ']', '~',
                   '-', '=', '_', '!', '1', '2', '3', '4', '5',
                   '6', '7', '8', '0', '?', '1', '2', '3', '4',
                   '5', '6', '7', '8', '^', 'v', '<', '>', 'o',
                   '=', '=', '=', '=', '=', '=', '%', '%', '%'};

char* block_name[256] = {"Air",
                         "Wood",
                         "Leaves",
                         "Grass",
                         "Stone",
                         "Bedrock",
                         "Pebble",
                         "White Wool",
                         "Red Wool",
                         "Green Wool",
                         "Yellow Wool",
                         "Blue Wool",
                         "Magenta Wool",
                         "Cyan Wool",
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
                         "Coin",
                         "1-Coin Gate",
                         "2-Coin Gate",
                         "5-Coin Gate",
                         "10-Coin Gate",
                         "20-Coin Gate",
                         "50-Coin Gate",
                         "Red Key",
                         "Blue Key",
                         "Green Key"};

uint8_t block_type[256] = {TYPE_NONSOLID, TYPE_SOLID, TYPE_NONSOLID, TYPE_NONSOLID, TYPE_SOLID,
                           TYPE_SOLID, TYPE_NONSOLID, TYPE_SOLID, TYPE_SOLID, TYPE_SOLID,
                           TYPE_SOLID, TYPE_SOLID, TYPE_SOLID, TYPE_SOLID, TYPE_SOLID,
                           TYPE_NONSOLID,
                           TYPE_NONSOLID, TYPE_NONSOLID, TYPE_NONSOLID, TYPE_NONSOLID, TYPE_NONSOLID,
                           TYPE_BARRIER, TYPE_WARP, TYPE_WARP, TYPE_WARP, TYPE_WARP,
                           TYPE_WARP, TYPE_WARP, TYPE_WARP, TYPE_WARP, TYPE_WARP_SPAWN,
                           TYPE_WARP_RANDOM, TYPE_FILEWARP, TYPE_FILEWARP, TYPE_FILEWARP, TYPE_FILEWARP,
                           TYPE_FILEWARP, TYPE_FILEWARP, TYPE_FILEWARP, TYPE_FILEWARP, TYPE_ZIPWIRE_UP,
                           TYPE_ZIPWIRE_DOWN, TYPE_ZIPWIRE_LEFT, TYPE_ZIPWIRE_RIGHT, TYPE_COIN, TYPE_GATE_1,
                           TYPE_GATE_2, TYPE_GATE_5, TYPE_GATE_10, TYPE_GATE_20, TYPE_GATE_50,
                           TYPE_KEY_RED, TYPE_KEY_BLUE, TYPE_KEY_GREEN};

char *block_type_name[TYPE_COUNT] = {"Non-Solid",
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
                                     "Coin",
                                     "1-Coin Gate",
                                     "2-Coin Gate",
                                     "5-Coin Gate",
                                     "10-Coin Gate",
                                     "20-Coin Gate",
                                     "50-Coin Gate",
                                     "Red Key",
                                     "Blue Key",
                                     "Green Key"};

bool text = true;
bool building = true;
bool painting = false;
bool solidity = true;
bool warps = true;
bool zipwires = true;
bool coins = true;
bool is_multiplayer = false;

bool key_red = false;
bool key_green = false;
bool key_blue = false;

char *option_list[OPTION_COUNT] = {"Back",
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
                                   "Toggle Text",
                                   "Toggle Building",
                                   "Replace All Instances of One Block with Held Block",
                                   "Fill Level with Held Block",
                                   "Change Username",
                                   "Change Skin",
                                   "Change Level Name",
                                   "Change Held Block Type",
                                   "Connect to Server"};

char *level_list[LEVEL_COUNT] = {"level1.asciilvl",
                                 "level2.asciilvl",
                                 "level3.asciilvl",
                                 "level4.asciilvl",
                                 "level5.asciilvl",
                                 "level6.asciilvl",
                                 "level7.asciilvl",
                                 "level8.asciilvl"};

const char* is_enabled(bool condition);
void init_colour(void);
void draw_borders(void);
void draw_map(void);
void draw_player(void);
void inventory_menu(void);
void load_level(int level);
void save_level(int level);
void load_abl(void);
void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps, bool respect_zipwires);
void teleport(bool relative);

void warp(uint8_t warp_id);
void draw_ui(void);
unsigned int count_blocks(uint8_t block);
void new_level(void);
void set_block(int block_y, int block_x, bool remove_block_if_present);
void replace_all(void);
void fill_all(void);
void change_value(void* variable, const char* prompt, const char* format);
void option(int i);
void options_menu(void);
void level_menu(bool save);
int main(void);

const char* is_enabled(bool condition) {
  return condition ? "Enabled" : "Disabled";
}

void init_colour(void) {
  start_color();
  
  init_pair(0,  COLOR_BLACK,   COLOR_BLACK);
  init_pair(1,  COLOR_YELLOW,  COLOR_BLACK);
  init_pair(2,  COLOR_GREEN,   COLOR_BLACK);
  init_pair(3,  COLOR_YELLOW,  COLOR_GREEN);
  init_pair(4,  COLOR_WHITE,   COLOR_WHITE);
  init_pair(5,  COLOR_BLACK,   COLOR_WHITE);
  init_pair(6,  COLOR_WHITE,   COLOR_BLACK);
  init_pair(7,  COLOR_WHITE,   COLOR_BLACK);
  init_pair(8,  COLOR_RED,     COLOR_BLACK);
  init_pair(9,  COLOR_GREEN,   COLOR_BLACK);
  init_pair(10, COLOR_YELLOW,  COLOR_BLACK);
  init_pair(11, COLOR_BLUE,    COLOR_BLACK);
  init_pair(12, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(13, COLOR_CYAN,    COLOR_BLACK);
  init_pair(14, COLOR_YELLOW,  COLOR_BLACK);
  init_pair(15, COLOR_RED,     COLOR_BLACK);
  init_pair(16, COLOR_BLUE,    COLOR_BLACK);
  init_pair(17, COLOR_GREEN,   COLOR_BLACK);
  init_pair(18, COLOR_GREEN,   COLOR_YELLOW);
  init_pair(19, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(20, COLOR_WHITE,   COLOR_BLACK);
  init_pair(21, COLOR_BLACK,   COLOR_RED);
  init_pair(22, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(23, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(24, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(25, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(26, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(27, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(28, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(29, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(30, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(31, COLOR_BLACK,   COLOR_MAGENTA);
  init_pair(32, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(33, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(34, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(35, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(36, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(37, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(38, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(39, COLOR_BLACK,   COLOR_YELLOW);
  init_pair(40, COLOR_GREEN,   COLOR_BLACK);
  init_pair(41, COLOR_GREEN,   COLOR_BLACK);
  init_pair(42, COLOR_GREEN,   COLOR_BLACK);
  init_pair(43, COLOR_GREEN,   COLOR_BLACK);
  init_pair(44, COLOR_YELLOW,  COLOR_BLACK);
  init_pair(45, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(46, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(47, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(48, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(49, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(50, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(51, COLOR_RED,     COLOR_BLACK);
  init_pair(52, COLOR_BLUE,    COLOR_BLACK);
  init_pair(53, COLOR_GREEN,   COLOR_BLACK);

  init_pair(128, COLOR_MAGENTA, COLOR_BLACK);
  // colour of player character
}

void draw_borders(void) {
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

void draw_map(void) {
  for (int i = 0; i < MAP_SIZE; i++) {
    if (!(block_type[map[i]] == TYPE_BARRIER && building)) {
      attron(COLOR_PAIR(map[i]));
      mvaddch((i / width) + 1, (i % width) + 1, block[map[i]]);
      attroff(COLOR_PAIR(map[i]));
    }
  }
}

void draw_player(void) {
  attron(COLOR_PAIR(128));
  mvaddch(y + 1, x + 1, skin);
  attroff(COLOR_PAIR(128));
}

void inventory_menu(void) {
  char input;

  clear();
  timeout(-1); // disable ticking for input

  for (int i = 0; i < block_count; i++) {
    attron(COLOR_PAIR(i));
    mvaddch(i / inventory_row_max, 3 * (i % inventory_row_max) + 2, block[i]);
    attroff(COLOR_PAIR(i));
  }

  do {
    int i;
    for (i = 0; i < block_count; i++) {
      if (i == held_block) {
        mvaddch(i / inventory_row_max, 3 * (i % inventory_row_max) + 1, '>');
        mvaddch(i / inventory_row_max, 3 * (i % inventory_row_max) + 3, '<');
        break;
      }
    }

    input = getch();

    mvaddch(i / inventory_row_max, 3 * (i % inventory_row_max) + 1, ' ');
    mvaddch(i / inventory_row_max, 3 * (i % inventory_row_max) + 3, ' ');

    switch (input) {
      case 'd':
        if (held_block + 1 != block_count && (held_block + 1) % inventory_row_max) held_block++;
        break;
      case 'a':
        if (held_block % inventory_row_max) held_block--;
        break;
      case 's':
        if (held_block + inventory_row_max < block_count) held_block += inventory_row_max;
        break;
      case 'w':
        if (held_block - inventory_row_max >= 0) held_block -= inventory_row_max;
        break;
    }

  } while (input != '\n');

  timeout(TICK); // reenable ticking
  draw_ui();
}

void load_level(int level) {
  if (level >= 0 && level < LEVEL_COUNT) file = fopen(level_list[level], "rb");
  else return;
  if (!file) return;
  
  for (int i = 0; i < MAP_SIZE; i++) {
    map[i] = fgetc(file);
  }
  
  fclose(file);
  
  draw_map();
  tp(0, 0, false, false, false);
}

void save_level(int level) {
  if (level >= 0 && level < LEVEL_COUNT) file = fopen(level_list[level], "wb");
  else return;

  for (int i = 0; i < MAP_SIZE; i++) {
    fputc(map[i], file);
  }
  
  fclose(file);
}

void load_abl(void) {
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

void tp(int tp_y, int tp_x, bool respect_solidity, bool respect_warps, bool respect_zipwires) {
  getyx(stdscr, cursor_y, cursor_x);

  if (tp_y < 0 || tp_y >= height) return;
  if (tp_x < 0 || tp_x >= width) return;

  switch (block_type[BLOCK(tp_y, tp_x)]) {
    case TYPE_GATE_1:
      if (coin_count < 1) return;
      else coin_count -= 1;
      break;
    case TYPE_GATE_2:
      if (coin_count < 2) return;
      else coin_count -= 2;
      break;
    case TYPE_GATE_5:
      if (coin_count < 5) return;
      else coin_count -= 5;
      break;
    case TYPE_GATE_10:
      if (coin_count < 10) return;
      else coin_count -= 10;
      break;
    case TYPE_GATE_20:
      if (coin_count < 20) return;
      else coin_count -= 20;
      break;
    case TYPE_GATE_50:
      if (coin_count < 50) return;
      else coin_count -= 50;
      break;
  } // TODO fix horrible code

  if ((block_type[BLOCK(tp_y, tp_x)] == TYPE_SOLID || block_type[BLOCK(tp_y, tp_x)] == TYPE_BARRIER) && respect_solidity) return;

  attron(COLOR_PAIR(BLOCK(y, x)));
  mvaddch(cursor_y, cursor_x - 1, block[BLOCK(y, x)]);
  attroff(COLOR_PAIR(BLOCK(y, x)));
  
  y = tp_y;
  x = tp_x;

  draw_player();

  if (respect_warps) {
    switch (block_type[BLOCK(y, x)]) {
      case TYPE_WARP:
        warp(BLOCK(y, x));
        break;
      case TYPE_WARP_SPAWN:
        tp(spawn_y, spawn_x, false, warps, zipwires);
        break;
      case TYPE_WARP_RANDOM:
        tp(rand() % height, rand() % width, false, warps, zipwires);
        break;
      case TYPE_FILEWARP:
        load_level((BLOCK(y, x) + 6) % LEVEL_COUNT);
        break;
    }
  }

  if (respect_zipwires) {
    switch (block_type[BLOCK(y, x)]) {
      case TYPE_ZIPWIRE_UP:
        tp(y - 1, x, false, warps, zipwires);
        draw_player();
        break;
      case TYPE_ZIPWIRE_DOWN:
        tp(y + 1, x, false, warps, zipwires);
        draw_player();
        break;
      case TYPE_ZIPWIRE_LEFT:
        tp(y, x - 1, false, warps, zipwires);
        draw_player();
        break;
      case TYPE_ZIPWIRE_RIGHT:
        tp(y, x + 1, false, warps, zipwires);
        draw_player();
        break;
    }
  }

  if (coins && block_type[BLOCK(y, x)] == TYPE_COIN) {
    coin_count++;
    BLOCK(y, x) = 0;
  }

  if (block_type[BLOCK(y, x)] == TYPE_KEY_RED) {
    key_red = true;
    BLOCK(y, x) = 0;
  }

  if (block_type[BLOCK(y, x)] == TYPE_KEY_BLUE) {
    key_blue = true;
    BLOCK(y, x) = 0;
  }

  if (block_type[BLOCK(y, x)] == TYPE_KEY_GREEN) {
    key_green = true;
    BLOCK(y, x) = 0;
  }
}

void teleport(bool relative) {
  clear();
  curs_set(1);
  echo();

  timeout(-1); // disable ticking for input

  int new_x = x;
  printw("X: %s", relative ? "~" : "");
  scanw(" %d", &new_x);

  int new_y = y;
  printw("Y: %s", relative ? "~" : "");
  scanw(" %d", &new_y);

  timeout(TICK); // reenable ticking

  curs_set(0);
  noecho();

  draw_ui();

  if (relative) {
    tp(new_y + y, new_x + x, false, warps, zipwires);
  } else {
    tp(new_y, new_x, false, warps, zipwires);
  }
}

void connect_to_server(void) {
  clear();
  curs_set(1);
  echo();

  timeout(-1); // disable ticking for input

  printw("Server IP: ");
  getch();

  timeout(TICK); // reenable ticking

  curs_set(0);
  noecho();

  draw_ui();
}

void warp(uint8_t warp_id) {
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      if (BLOCK(i, j) == warp_id && (i != y || j != x)) {
        tp(i, j, false, false, false);
        return;
      }
    }
  }
}

void draw_ui(void) {
  clear();

  if (text) {
    mvprintw(0, width + 4, "%s %s\n", NAME, VERSION);
    mvprintw(1, width + 4, "by Hayden\n");
  
    mvprintw(8, width + 4, "Block Texture:\n");
  }

  draw_borders();
  draw_map();
  
  tp(y, x, false, false, false);
}

void set_block(int block_y, int block_x, bool remove_block_if_present) {
  if (block_y < 0 || block_y >= height) return;
  if (block_x < 0 || block_x >= width) return;

  attron(COLOR_PAIR(remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : block[held_block]));
  mvaddch(block_y + 1, block_x + 1, remove_block_if_present ? block[PLACE_BLOCK(block_y, block_x)] : block[held_block]);
  attroff(COLOR_PAIR(remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : block[held_block]));
  
  move(cursor_y, cursor_x);
  
  BLOCK(block_y, block_x) = remove_block_if_present ? PLACE_BLOCK(block_y, block_x) : held_block;
}

void new_level(void) {
  free(map);

  clear();
  curs_set(1);
  echo();

  printw("Leave the width or height blank to accept their default values.\n\n");

  timeout(-1); // disable ticking for input

  height = DEFAULT_HEIGHT;
  printw("Height (%d): ", DEFAULT_HEIGHT);
  scanw(" %d", &height);

  width = DEFAULT_WIDTH;
  printw("Width (%d): ", DEFAULT_WIDTH);
  scanw(" %d", &width);

  curs_set(0);
  noecho();

  timeout(TICK); // reenable ticking

  map = calloc(MAP_SIZE, sizeof (uint8_t));
  draw_ui();

  tp(0, 0, false, false, false);
}

void replace_all(void) {
  uint8_t block_to_replace = 0;

  clear();
  curs_set(1);
  echo();

  printw("Block ID to replace: ");
  timeout(-1); // disable ticking for input
  scanw(" %hhu", &block_to_replace);
  timeout(TICK); // reenable ticking

  for (int i = 0; i < MAP_SIZE; i++) {
    if (map[i] == block_to_replace) map[i] = held_block;
  }

  curs_set(0);
  noecho();

  draw_ui();
}

void fill_all(void) {
  for (int i = 0; i < MAP_SIZE; i++) {
    map[i] = held_block;
  }

  draw_map();
}

void change_held_block_type(void) {
  clear();
  curs_set(1);
  echo();
  
  for (int i = 0; i < TYPE_COUNT; i++) {
    printw("%d = %s\n", i, block_type_name[i]);
  }

  printw("\nBlock Type: ");
  timeout(-1); // disable ticking for input
  scanw(" %hhd", &block_type[held_block]);
  timeout(TICK); // reenable ticking
  
  curs_set(0);
  noecho();
  
  draw_ui();
}

void change_value(void* variable, const char* prompt, const char* format) {
  clear();
  curs_set(1);
  echo();

  printw("%s", prompt);
  timeout(-1); // disable ticking for input
  scanw(format, variable);
  timeout(TICK); // reenable ticking
  
  curs_set(0);
  noecho();
  
  draw_ui();
}

void option(int i) {
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
      text ^= 1;
      break;
    case 12:
      building ^= 1;
      break;
    case 13:
      replace_all();
      break;
    case 14:
      fill_all();
      break;
    case 15:
      change_value(username, "Username: ", " %32s");
      break;
    case 16:
      change_value(&skin, "Skin: ", " %c");
      break;
    case 17:
      change_value(level_name, "Level Name: ", " %32s");
      break;
    case 18:
      change_held_block_type();
      break;
    case 19:
      connect_to_server();
      break;
  }
  
  draw_ui();
}

void options_menu(void) {
  int j = 0;
  
  clear();
  
  while (true) {
    for (int i = 0; i < OPTION_COUNT; i++) {
      if (i == j) {
        attron(A_REVERSE);
      }
      mvprintw(i, 0, "%s\n", option_list[i]);
      if (i == j) {
        attroff(A_REVERSE);
      }
    }
    
    switch (getch()) {
      case 'w':
      case KEY_UP:
        j--;
        if (j == -1) {
          j = OPTION_COUNT - 1;
        }
        break;
      case 's':
      case KEY_DOWN:
        j++;
        if (j == OPTION_COUNT) {
          j = 0;
        }
        break;
      case '\n':
        option(j);
        return;
    }
  }
}

void level_menu(bool save) {
  int j = 0;
  
  clear();
  
  while (true) {
    for (int i = 0; i < LEVEL_COUNT; i++) {
      if (i == j) attron(A_REVERSE);
      mvprintw(i, 0, "%s\n", level_list[i]);
      if (i == j) attroff(A_REVERSE);
    }
    
    switch (getch()) {
      case 'w':
      case KEY_UP:
        j--;
        if (j == -1) j = LEVEL_COUNT - 1;
        break;
      case 's':
      case KEY_DOWN:
        j++;
        if (j == LEVEL_COUNT) j = 0;
        break;
      case '\n':
        if (save) save_level(j);
        else load_level(j);
        return;
    }
  }
}
// TODO consolidate options_menu and level_menu into one function that can be reused for both those purposes

int main(void) {
  srand(time(NULL));
  map = calloc(MAP_SIZE, sizeof (uint8_t));

  initscr();
  cbreak(); // quit game with CTRL+C
  noecho();
  timeout(TICK); // enable ticking
  curs_set(0);
  keypad(stdscr, TRUE);

  init_colour();
  draw_ui();

  while (true) {
    getyx(stdscr, cursor_y, cursor_x);

    if (text) {
      mvprintw(3, width + 4, "XY: (%d, %d)\n", x, y);
      mvprintw(4, width + 4, "Spawn XY: (%d, %d)\n", spawn_x, spawn_y);
      mvprintw(5, width + 4, "Ticks: %u\n", ticks);
      mvprintw(6, width + 4, "Session Type: %s\n", is_multiplayer ? "Multiplayer" : "Singleplayer");

      attron(COLOR_PAIR(held_block));
      mvaddch(8, width + 19, block[held_block]);
      attroff(COLOR_PAIR(held_block));

      mvprintw(9, width + 4, "Block ID: %d\n", held_block);
      mvprintw(10, width + 4, "Block Name: %s\n", block_name[held_block]);
      mvprintw(11, width + 4, "Block Type: %s\n", block_type_name[block_type[held_block]]);

      mvprintw(13, width + 4, "Building: %s\n", is_enabled(building));
      mvprintw(14, width + 4, "Painting: %s\n", is_enabled(painting));    
      mvprintw(15, width + 4, "Solidity: %s\n", is_enabled(solidity));
      mvprintw(16, width + 4, "Warps: %s\n", is_enabled(warps));
      mvprintw(17, width + 4, "Zipwires: %s\n", is_enabled(zipwires));

      mvprintw(19, width + 4, "Coins: %u\n", coin_count);
      mvprintw(20, width + 4, "Red Key: %s\n", key_red ? "You Have This!" : "You Don't Have This");
      mvprintw(21, width + 4, "Blue Key: %s\n", key_blue ? "You Have This!" : "You Don't Have This");
      mvprintw(22, width + 4, "Green Key: %s\n", key_green ? "You Have This!" : "You Don't Have This");

      mvprintw(24, width + 4, "Username: %s\n", username);
      mvprintw(25, width + 4, "Level Name: %s\n", level_name);

      // TODO redrawing this text every tick is inefficient so don't do it

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
        if (building) set_block(y - 1, x, !painting);
        break;
      case 'k':
        if (building) set_block(y + 1, x, !painting);
        break;
      case 'j':
        if (building) set_block(y, x - 1, !painting);
        break;
      case 'l':
        if (building) set_block(y, x + 1, !painting);
        break;
      case 'e':
        inventory_menu();
        break;
      case '\n':
        if (building) {
          spawn_y = y;
          spawn_x = x;
        }
        break;
      case 'r':
        if (building) tp(spawn_y, spawn_x, false, warps, zipwires);
        break;
      case 'f':
        if (building) held_block = (held_block - 1 + block_count) % block_count;
        break;
      case 'h':
        if (building) held_block = (held_block + 1) % block_count;
        break;
      case 'm':
        options_menu();
        break;
    }

    ticks++;
  }
}
