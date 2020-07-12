#ifndef MAIN_H
#define MAIN_H

#ifndef __APPLE__
#include <malloc.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#define internal static

#define bool unsigned char
#define true 1
#define false 0

#include "libs/gb_string.h"
#include "libs/stb_ds.h"
#include "libs/stb_sprintf.h"

// --- main.c ---

// TODO: Remove this
typedef unsigned char BlockType;
#define GRASS 0
#define GRASS_TREE 1
#define GRASS_STONE 2 // TODO
// ------


// ---- NOTES/TODO: ----
// Separate into Ground Type and Overlay Type
// Stone, Tree, AppleTree, Coal
// Grass, Dirt, Grass with Stones, Stone Ground, Sand, Gravel

// Tree amts, stone amts, buildings, cave openings
// ----             ----


// Tiles (1 byte) = 2-bit Water Encoding, 6-bit Ground Type [63 total ground types; 0x00 - 0x3F]
typedef unsigned char TileType;
#define TILE_GROUND(t) ((t) & 0x3F)
#define TILE_WATER_ENCODING(t) (((t) & 0xC0) >> 6)

#define TILE_GRASS 0x00
#define TILE_GRASS_STONE_FULL 0x01
#define TILE_GRASS_STONE_N 0x02 // TODO
#define TILE_GRASS_STONE_E 0x03
#define TILE_GRASS_STONE_S 0x04
#define TILE_GRASS_STONE_W 0x05
#define TILE_GRASS_DIRT 0x06 // TODO: TILE_DIRT

#define TILE_WATER_NONE 0x00 // 0000,0000
#define TILE_WATER_VERT_LAKE 0x40 // 0100,0000
#define TILE_WATER_HORIZ_LAKE 0x80 // 1000,0000
#define TILE_WATER_POND 0xC0 // 1100,0000

// OverlayItems (1 byte) = 4-bit amt (15 total), 4-bit type (15 total)
// item >> 4 = amt
typedef unsigned char OverlayItem; // TODO: Put lakes into this, where amt determins vert/horiz/pond/etc.???
#define OVERLAY_TYPE(o) ((o) & 0x0F)
#define OVERLAY_AMT(o) (((o) & 0xF0) >> 4)

#define OVERLAY_NONE 0x00

#define OVERLAY_ROCKS 0x01
#define OVERLAY_TREES 0x02 // TODO: Tree Type
#define OVERLAY_FALLEN_TREES 0x03 // TODO: Tree Type
#define OVERLAY_FLOWERS 0x04 // AMT determins type, forage/bottanize with randomize and give different things based on this
#define OVERLAY_STONE 0x05 // AMT determins whether North (1), East (2), South (3), or West(4)
// OVERLAY_CLAY??? - only appears with water?

OverlayItem createOverlayItem(char type, char amt);

// Item (2 bytes) = 4-bit weight, 5-bit quality, 7-bit type (128 total item types; 0x00 - 0x7F)
typedef unsigned short Item;
#define ITEM_TYPE(i) ((i) & 0x007F)
#define ITEM_QUALITY(i) (((i) & 0x0F80) >> 7)
#define ITEM_WEIGHT(i) (((i) & 0xF000) >> 12)

#define ITEM_NONE 0x0000

#define ITEM_WOOD_AXE 0x01
#define ITEM_ROCK 0x02
#define ITEM_LOG 0x03
#define ITEM_BLUEBERRY 0x04
#define ITEM_LAST 0x04

Item createItem(char type, char quality, char weight);

typedef unsigned char WeatherType;
#define WEATHER_CLEAR 0
#define WEATHER_RAIN 1
#define WEATHER_FOG 2
#define WEATHER_CLOUDY 3
#define WEATHER_TSTORM 4
#define WEATHER_SNOW 5
#define WEATHER_BLIZZARD 6

typedef struct Tile {
	TileType type; // 2-bit water encoding, 6-bit ground type = 1 byte
	OverlayItem overlayItems[8]; // 8 bytes
	// TODO: Hashtable to list of Items where key is tile location?

	Item *items;
} Tile;

Tile createTile(TileType type);

typedef struct ItemHashMap {
	size_t key;
	Item *value;
} ItemHashMap;

typedef struct World {
	int width, height;
	int spawnX, spawnY;
	Tile *tiles;

	ItemHashMap *items; // Hashmap from integer key to Item Array value.
	
	// Weather
	int windSpeed;
	WeatherType weather;
} World;

char gb_string_pop(gbString string);

typedef enum PartOfSpeech {
	POS_UNKNOWN, POS_NOUN, POS_PRONOUN, POS_ARTICLE_DEF, POS_ARTICLE_INDEF,
	POS_ARTICLE_INDEF_AN, POS_VERB, POS_ADJECTIVE, POS_PREPOSITION,
	POS_QUANTIFIER
} PartOfSpeech;

typedef enum PhraseType {
	PHRASE_UNKNOWN, 
	PHRASE_NOUN, PHRASE_NOUN_DIRECT_OBJECT, PHRASE_NOUN_INDIRECT_OBJECT,
	PHRASE_VERB, // Subordinate Verb Phrase??
	PHRASE_PREPOSITION
} PhraseType;

typedef struct DictValue {
	char partOfSpeechCount;
	PartOfSpeech pos[2];
	char itemType; // Should be an array
	char *wordStart;
	char *wordEnd;
} DictValue;

typedef struct NounPhrase { // Or PronounPhrase
	PhraseType type;
	int word_start_i;
	int word_end_i;

	DictValue article;
	// TODO: Adjectives and quantifiers
	DictValue noun; // Or Pronoun
} NounPhrase;

typedef struct PrepositionalPhrase {
	PhraseType type;
	int word_start_i;
	int word_end_i;

	DictValue preposition;
	NounPhrase nounPhrase;
} PrepositionalPhrase;

typedef struct Phrase {
	PhraseType type;
	int word_start_i;
	int word_end_i;

	union {
		NounPhrase nounPhrase;
		PrepositionalPhrase prepositionalPhrase;
	} info;
} Phrase;

typedef struct Sentence {
	DictValue mainVerb;
	PrepositionalPhrase p_phrase; // TODO
	NounPhrase directObj;
	NounPhrase indirectObj;
	Phrase *phrases;
} Sentence;

typedef struct DictHashMap {
	size_t key;
	DictValue value;
} DictHashMap;

void parseInput(char *input, DictHashMap *dictionary, DictValue **words, Phrase **phrases);

// --- language.c ---

bool isConsonant(char c);
bool isVowel(char c);
gbString makePlural(gbString buffer);
gbString numberToText(int n);

// --- ---

void clrscr();

#ifdef _WIN32
#include <conio.h>
#define getch _getch
#define kbhit _kbhit
#else
#include <alloca.h>
#include <unistd.h>
#include <termios.h>
char getch();
char getch_nonblocking();
#endif

// --- colors.c ---

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define FOREGROUND_YELLOW FOREGROUND_RED|FOREGROUND_GREEN
#define FOREGROUND_CYAN FOREGROUND_GREEN|FOREGROUND_BLUE
#define FOREGROUND_MAGENTA FOREGROUND_RED|FOREGROUND_BLUE
#define FOREGROUND_WHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define BACKGROUND_YELLOW BACKGROUND_RED|BACKGROUND_GREEN
#define BACKGROUND_CYAN BACKGROUND_GREEN|BACKGROUND_BLUE
#define BACKGROUND_MAGENTA BACKGROUND_RED|BACKGROUND_BLUE
#define BACKGROUND_WHITE BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE

#endif

#define COL_RED "\x1b[31m" // Error
#define COL_GREEN "\x1b[32m" // Prompt, Success
#define COL_YELLOW "\x1b[33m"
#define COL_BLUE "\x1b[34m"
#define COL_MAGENTA "\x1b[35m"
#define COL_CYAN "\x1b[36m" // Information
#define COL_RESET "\x1b[0m" // Input

typedef enum COLOR
{
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_CYAN,
	COLOR_MAGENTA,
	COLOR_WHITE,
	COLOR_BLACK
} COLOR;

#ifdef _WIN32
HANDLE hConsole;
#endif

void setColor(COLOR foreground);
void resetColor(void);
void colors_printf(COLOR foreground, const char *fmt, ...);
void colors_puts(COLOR foreground, const char *fmt, ...);
void printError(const char *fmt, ...);
void printPrompt(const char *fmt, ...);

// --- parsing.c ---

// Function pointer to function that can run user-code on specific keypresses during
// input (with getInput). If null, the function is not called.
// Return true if keypress should continue to use default action provided by getInput()
typedef bool (*inputKeyCallback)(char, bool isSpecial, char **, int *);

#define INPUT_ESC 27

// ANSI Control Characters
#define INPUT_CTRL_L 12 // Clear Screen
#define INPUT_CTRL_X 24 // Cancel
#define INPUT_CTRL_C 3 // Exit program
#define INPUT_CTRL_O 15

// Special Keys
#ifdef _WIN32

#define INPUT_SPECIAL1 -32
#define INPUT_SPECIAL2 224 // TODO
#define INPUT_LEFT 75
#define INPUT_RIGHT 77
#define INPUT_DELETE 83
#define INPUT_END 79
#define INPUT_HOME 71
#define INPUT_ENDINPUT 26 // Ctrl-Z
#define INPUT_BACKSPACE 8
#else
#define INPUT_SPECIAL1 27
#define INPUT_SPECIAL2 91
#define INPUT_LEFT 68
#define INPUT_RIGHT 67
#define INPUT_DELETE1 51
#define INPUT_DELETE2 126
#define INPUT_END 70
#define INPUT_HOME 72
#define INPUT_ENDINPUT 4 // Ctrl-D
#define INPUT_BACKSPACE 127
#endif

char *skipWhitespace(char *start, char *endBound, bool backwards);
char *skipWord(char *start, char *endBound, bool includeNumbers, bool includeSymbols, bool backwards);
char *skipNumbers(char *start, char *endBound);
char *skipLineNumber(char *start, char *endBound);

char *getInput(bool *canceled, char *inputBuffer, inputKeyCallback callback);
int parsing_getLine(char *line, int max, int trimSpace);
int parsing_getLine_dynamic(char **chars, int trimSpace);

#endif