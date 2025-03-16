#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GB_STRING_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#include "main.h"

// -----

#ifdef _WIN32

void clrscr() {
	system("cls");
}

#else

#include <unistd.h> // _getch
#include <termios.h> // _getch
char getch() {
	char buf=0;
	struct termios old={0};
	fflush(stdout);
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	old.c_lflag&=~ICANON;
	old.c_lflag&=~ECHO;
	old.c_cc[VMIN]=1;
	old.c_cc[VTIME]=0;
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	old.c_lflag|=ICANON;
	old.c_lflag|=ECHO;
	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");
	return buf;
}

char getch_nonblocking() {
	char buf = 0;
	struct termios old = {0};
	
	fflush(stdout);
	
	if (tcgetattr(0, &old) < 0)
		perror("tcsetattr()");
	
	old.c_lflag&=~ICANON;
	old.c_lflag&=~ECHO;
	old.c_cc[VMIN] = 0;
	old.c_cc[VTIME] = 0;
	
	if (tcsetattr(0, TCSANOW, &old) < 0)
		perror("tcsetattr ICANON");
	if (read(0, &buf, 1) < 0)
		perror("read()");
	
	old.c_lflag|=ICANON;
	old.c_lflag|=ECHO;
	
	if (tcsetattr(0, TCSADRAIN, &old) < 0)
		perror("tcsetattr ~ICANON");
	return buf;
}

void clrscr() {
	printf("\e[1;H\e[2J");
}

#endif

// -----

// NOTE: Addon to gb_string.h lib
char gb_string_pop(gbString string) {
	gbUsize length = gb_string_length(string);
	char c = string[length - 1];
	string[length - 1] = '\0';

	gb_set_string_length(string, length - 1);

	return c;
}

// type = 4-bits, amt = 4-bits
// Max amt allowed = 15
OverlayItem createOverlayItem(char type, char amt) {
	return (amt << 4) | type;
}

// type = 7-bits, quality = 5-bits, weight = 4-bits
// Max quality is 31, max weight is 16
Item createItem(char type, char quality, char weight) {
	return (weight << 12) | (quality << 7) | type;
}

Item overlayToItem(OverlayItem overlayItem) {
	char type = OVERLAY_TYPE(overlayItem);

	switch (type) {
		case OVERLAY_ROCKS:
		{
			return createItem(ITEM_ROCK, 31 / 2, 3);
		} break;
		default:
		return ITEM_NONE;
	}
}

void printTileExplanation(Tile tile, bool plural, bool definiteArticle, int windSpeed) { // false for definiteArticle will use the indefinite article
	switch (TILE_GROUND(tile.type)) {
		case TILE_GRASS:
		{
			if (windSpeed <= 1) printf("a field of Grass");
			else if (windSpeed >=2 && windSpeed <= 7) printf("Grass swaying around you");
			else if (windSpeed >=8) printf("Grass violently rippling");

			int treeAmt = 0;
			for (int i = 0; i < 8; i++) {
				if (OVERLAY_TYPE(tile.overlayItems[i]) == OVERLAY_TREES) {
					treeAmt += OVERLAY_AMT(tile.overlayItems[i]);
				}
			}

			if (treeAmt > 0) {
				if (treeAmt == 1 || windSpeed <= 3) printf(" and a tree nearby");
				else if (treeAmt < 5 || (windSpeed >= 4 && windSpeed <= 5)) printf(" and a small whispering in the wind");
				else if (treeAmt < 13 || (windSpeed >= 6 && windSpeed <= 7)) printf(" and a humming noise nearby");
				else if (windSpeed >= 8) printf(" and a loud rustling of branches");
			}
		} break;
		/*case GRASS_TREE:
		{
			if (windSpeed <= 1) printf("Grass");
			else if (windSpeed >= 2 && windSpeed <= 7) printf("Grass swaying around you");
			else if (windSpeed >= 8) printf("Grass violently rippling");
			
			if (windSpeed <= 3) printf(" and a tree nearby");
			else if (windSpeed >= 4 && windSpeed <= 5) printf(" and a small whispering in the wind");
			else if (windSpeed >= 6 && windSpeed <= 7) printf(" and a humming noise nearby");
			else if (windSpeed >= 8) printf(" and a loud rustling of branches");
		} break;*/
		case TILE_GRASS_STONE_N:
		{
			if (windSpeed <= 1) printf("Grass");
			else if (windSpeed >= 2 && windSpeed <= 7) printf("Grass swaying around you");
			else if (windSpeed >= 8) printf("Grass violently rippling");
			
			printf(" and a stone wall filling the north half of the tile");
		} break;
		default:
		printf("Unknown");
		break;
	}
}

void printItemName(Item item, bool plural, bool definiteArticle, unsigned int quantifier) { // false for definiteArticle will use the indefinite article, 0 for quantifier will use an article instead
	gbString singularName;

	char type = ITEM_TYPE(item);
	char quality = ITEM_QUALITY(item);
	char weight = ITEM_WEIGHT(item);
	
	switch (type) {
		case ITEM_NONE:
		printf("[None]"); // TODO: error, should never happen
		return;
		
		case ITEM_WOOD_AXE:
		{
			singularName = gb_make_string_length("Wooden Axe", 10);
		} break;
		case ITEM_ROCK:
		{
			singularName = gb_make_string_length("Rock", 4);
		} break;
		case ITEM_LOG:
		{
			singularName = gb_make_string_length("Log", 3);
		} break;
		case ITEM_BLUEBERRY:
		{
			singularName = gb_make_string_length("Blueberry", 9);
		} break;
		default:
		{
			printf("[Unknown: %d]", type);
		} return;
	}
	
	if (definiteArticle) {
		printf("the ");
	} else if (!plural) {
		if (isVowel(singularName[0])) {
			printf("an ");
		} else {
			printf("a ");
		}
	} else if (quantifier > 1) {
		gbString text = numberToText(quantifier);
		printf("%s ", text);
		fflush(stdout);
		gb_free_string(text);
	}
	
	if (plural) {
		singularName = makePlural(singularName);
	}
	
	//printf("(%d) ", type);
	printf("%s", singularName);
	
	if (singularName != NULL)
		gb_free_string(singularName);
}

void printSurroundingExplanation(World *world, int x, int y, int *paths, int doPrint) {
	// Print current block explanation
	if (doPrint) {
		printf("You see ");
		printTileExplanation(world->tiles[x + y * world->width], false, false, world->windSpeed);
		printf(",\n");
	}
	
	// Set all paths accessible at start
	paths[0] = true; // West
	paths[1] = true; // East
	paths[2] = true; // North
	paths[3] = true; // South
	
	char currentTileType = world->tiles[x + y * world->width].type;

	// Rule out paths blocked by stone in current square (i)
	if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_W) paths[0] = false;
	if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_E) paths[1] = false;
	if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_N) paths[2] = false;
	if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_S) paths[3] = false;
	
	char westTileType = world->tiles[(x - 1) + y * world->width].type;
	char eastTileType = world->tiles[(x + 1) + y * world->width].type;
	char southTileType = world->tiles[x + (y + 1) * world->width].type;
	char northTileType = world->tiles[x + (y - 1) * world->width].type;

	// Look at surrounding blocks, ruling out paths, and printing an explanation for partial blockages.
	// Look at West first
	if (TILE_GROUND(westTileType) == TILE_GRASS_STONE_N && doPrint) {
		if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_N) {
			printf("     and a small path continuing to the west,\n");
		} else {
			printf("     and a small path to the west with a stone wall to its north,\n");
		}
	}
	
	// Look at East next
	if (TILE_GROUND(eastTileType) == TILE_GRASS_STONE_N && doPrint) {
		if (TILE_GROUND(currentTileType) == TILE_GRASS_STONE_N) {
			printf("     and a small path continuing to the east,\n");
		} else {
			printf("     and a small path to the east with a stone wall to its north,\n");
		}
	}
	
	// Look at South
	if (TILE_GROUND(southTileType) == TILE_GRASS_STONE_N) {
		if (doPrint) {
			printf("     and a stone wall to the south\n");
		}
		paths[3] = false;
	}
	
	// Print what's on the ground
	if (doPrint && (arrlen(world->tiles[x + y * world->width].items) > 0 || OVERLAY_TYPE(world->tiles[x + y * world->width].overlayItems[0]) == OVERLAY_ROCKS)) { // TODO
		printf("On the ground there is "); // TODO: Fix to say "are" when multiple items are listed

		bool overlayPrinted = false;
		if (OVERLAY_TYPE(world->tiles[x + y * world->width].overlayItems[0]) == OVERLAY_ROCKS) {
			int amt = OVERLAY_AMT(world->tiles[x + y * world->width].overlayItems[0]);
			printItemName(overlayToItem(world->tiles[x + y * world->width].overlayItems[0]), amt > 1, false, amt);
			overlayPrinted = true;
		}

		for (int i = 0; i < arrlen(world->tiles[x + y * world->width].items); i++) {
			if (overlayPrinted) {
				printf(", and ");
				overlayPrinted = false;
			}
			printItemName(world->tiles[x + y * world->width].items[i], false, false, 1);
			if (i != arrlen(world->tiles[x + y * world->width].items) - 1) printf(", ");
		}
		printf(".\n");
	}
	
	// List the paths still open
	if (doPrint) {
		// Find last available direction
		char lastAvailableDirection = 3; // 0 = West, 3 = South
		for (int i = 3; i >= 0; i--) {
			if (paths[i]) {
				lastAvailableDirection = i;
				break;
			}
		}
		
		printf("You can go ");
		if (paths[0]) printf("West, ");
		if (paths[1] && lastAvailableDirection == 1) printf("or East");
		else if (paths[1]) printf("East, ");
		if (paths[2] && lastAvailableDirection == 2) printf("or North");
		else if (paths[2]) printf("North, ");
		if (paths[3]) printf("or South");
		printf(".\n");
	}
}

bool insertItemIntoInventory(Item *inventoryItems, unsigned char *inventoryItemCount, Item item) { // Returns whether successful or not, increments the inventoryItemCount
	if (*inventoryItemCount >= 20) {
		return false;
	} else {
		// Find first empty slot
		unsigned char index = 0;
		for (int i = 0; i < 20; i++) {
			if (inventoryItems[i] == ITEM_NONE) {
				index = i;
				break;
			}
		}
		
		// Place item into slot
		inventoryItems[index] = item;
		(*inventoryItemCount)++;
		return true;
	}
}

Tile createTile(TileType type) {
	Tile tile = {0};

	tile.type = type;
	tile.items = NULL;

	return tile;
}

World initializeWorld(int width, int height, int seed) {
	stbds_rand_seed(time(0));

	World world = { width, height, width / 2, height / 2, NULL, NULL, 1, WEATHER_CLEAR };
	world.tiles = malloc(sizeof(Tile) * width * height);

	//world.blocks = malloc(sizeof(BlockType) * width * height);
	//world.items = malloc(sizeof(ItemType *) * width * height);
	
	// Set all item lists to NULL
	/*for (int i = 0; i < width * height; i++) {
		world.items[i] = NULL;
	}*/
	
	// Set seed
	srand(seed);
	
	world.spawnX = rand() % (width - 1);
	world.spawnY = rand() % (height - 1);
	
	// Generate Tiles
	for (int i = 0; i < width * height; i++) {
		int random = rand();

		//world.blocks[i] = random % 3;

		int overlayItemsIndex = 0;

		int mod3 = random % 3;
		if (mod3 == 0) { // GRASS
			world.tiles[i] = createTile(TILE_GRASS);
		} else if (mod3 == 1) { // GRASS_STONE_N
			world.tiles[i] = createTile(TILE_GRASS_STONE_N);
		} else if (mod3 == 2) { // Trees
			world.tiles[i] = createTile(TILE_GRASS);
			world.tiles[i].overlayItems[overlayItemsIndex] = createOverlayItem(OVERLAY_TREES, 15);
			overlayItemsIndex++;
		}

		if (rand() % 10 == 0) {
			world.tiles[i].overlayItems[overlayItemsIndex] = createOverlayItem(OVERLAY_ROCKS, 3);
		}
	}
	
	// Generate items on ground of squares
	for (int i = 0; i < width * height; i++) {
		if (world.spawnX + world.spawnY * width == i) {
			//arrput(world.items[i], WOOD_AXE);
			arrput(world.tiles[i].items, createItem(ITEM_WOOD_AXE, 31 / 2, 16 / 3));

			Item *itemArray = NULL;
			arrput(itemArray, createItem(ITEM_WOOD_AXE, 31 / 2, 16 / 3));
			hmput(world.items, stbds_hash_bytes(&i, 1, time(0)), itemArray);
		} /*else {
			if (rand() % 10 == 0) {
				arrput(world.items[i], STONE);
			}
		}*/
	}
	
	return world;
}

size_t hashSeed; // Global

void addWordToDictionary(DictHashMap **dictionary, char *key, char partOfSpeechCount, PartOfSpeech pos1, PartOfSpeech pos2, char itemType) {
	//static int int_ident = 
	DictValue value = {0};
	value.partOfSpeechCount = partOfSpeechCount;
	value.pos[0] = pos1;
	value.pos[1] = pos2;
	value.itemType = itemType;
	value.wordStart = NULL;
	value.wordEnd = NULL;

	size_t hash = stbds_hash_string_len(key, strlen(key), hashSeed);
	//printf("%s, hash: %ld\n", key, hash);
	hmput(*dictionary, hash, value);
}

DictHashMap *constructDictionary() {
	DictHashMap *dictionary = NULL;

	addWordToDictionary(&dictionary, "cut down", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	//addWordToDictionary(&dictionary, "cut", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "chop up", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "cut up", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "pick up", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "forage", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "the", 1, POS_ARTICLE_DEF, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "a", 1, POS_ARTICLE_INDEF, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "an", 1, POS_ARTICLE_INDEF_AN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "tree", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "log", 1, POS_NOUN, POS_UNKNOWN, ITEM_LOG);
	addWordToDictionary(&dictionary, "with", 1, POS_PREPOSITION, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "wooden axe", 1, POS_NOUN, POS_UNKNOWN, ITEM_WOOD_AXE);
	addWordToDictionary(&dictionary, "wood axe", 1, POS_NOUN, POS_UNKNOWN, ITEM_WOOD_AXE);
	addWordToDictionary(&dictionary, "axe", 1, POS_NOUN, POS_UNKNOWN, ITEM_WOOD_AXE);
	addWordToDictionary(&dictionary, "go", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "walk", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "give", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "goblin", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "to", 1, POS_PREPOSITION, POS_UNKNOWN, ITEM_NONE);

	addWordToDictionary(&dictionary, "north", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "south", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "east", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "west", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);

	addWordToDictionary(&dictionary, "quit", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "exit", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	//addWordToDictionary(&dictionary, "look through", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "shuffle through", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "search through", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE); // search vs. search through
	addWordToDictionary(&dictionary, "see", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "show", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "look into", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "look around", 1, POS_VERB, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "inventory", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);
	addWordToDictionary(&dictionary, "backpack", 1, POS_NOUN, POS_UNKNOWN, ITEM_NONE);

	return dictionary;
}

bool parseVerb(DictValue **words, int *currentWordIndex, Sentence *sentence) {
	if ((*words)[*currentWordIndex].pos[0] == POS_VERB) {
		sentence->mainVerb = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parseDefiniteArticle(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_ARTICLE_DEF) {
		phrase->article = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parseIndefiniteArticle(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_ARTICLE_INDEF) {
		phrase->article = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parseIndefiniteAnArticle(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_ARTICLE_INDEF_AN) {
		phrase->article = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parseNoun(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_NOUN) {
		phrase->noun = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parseNounPhrase(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	phrase->type = PHRASE_NOUN; // TODO
	if (parseDefiniteArticle(words, currentWordIndex, phrase)) {
		return parseNoun(words, currentWordIndex, phrase);
	} else if (parseIndefiniteArticle(words, currentWordIndex, phrase)) {
		return parseNoun(words, currentWordIndex, phrase);
	} else if (parseIndefiniteAnArticle(words, currentWordIndex, phrase)) {
		return parseNoun(words, currentWordIndex, phrase);
	} else {
		return parseNoun(words, currentWordIndex, phrase);
	}
}

bool parsePronoun(DictValue **words, int *currentWordIndex, NounPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_PRONOUN) {
		phrase->noun = (*words)[*currentWordIndex];
		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parsePreposition(DictValue **words, int *currentWordIndex, PrepositionalPhrase *phrase) {
	if ((*words)[*currentWordIndex].pos[0] == POS_PREPOSITION) {
		phrase->preposition = (*words)[*currentWordIndex];

		(*currentWordIndex)++;
		return true;
	}

	return false;
}

bool parsePrepositionalPhrase(DictValue **words, int *currentWordIndex, PrepositionalPhrase *phrase) {
	PrepositionalPhrase p_phrase = {0};
	p_phrase.type = PHRASE_PREPOSITION;
	p_phrase.word_start_i = *currentWordIndex;

	if (parsePreposition(words, currentWordIndex, &p_phrase)) {
		NounPhrase nounPhrase = {0}; // TODO
		bool success = parseNounPhrase(words, currentWordIndex, &nounPhrase) || parsePronoun(words, currentWordIndex, &nounPhrase);

		if (success) {
			p_phrase.nounPhrase = nounPhrase;
			*phrase = p_phrase;
		}

		return success;
	}

	phrase->type = PHRASE_UNKNOWN;

	return false;
}

void printDictValue(DictValue word) {
	int wordLength = word.wordEnd - word.wordStart;

	char *posString;
	switch(word.pos[0]) {
		case POS_UNKNOWN:
		posString = "Unk";
		break;
		
		case POS_ARTICLE_DEF:
		posString = "D_ART";
		break;

		case POS_ARTICLE_INDEF:
		posString = "I_ART";
		break;

		case POS_ARTICLE_INDEF_AN:
		posString = "I_ART";
		break;

		case POS_NOUN:
		posString = "N";
		break;

		case POS_VERB:
		posString = "V";
		break;

		case POS_PREPOSITION:
		posString = "P";
		break;

		case POS_PRONOUN:
		posString = "Pro";
		break;

		case POS_QUANTIFIER:
		posString = "Quant";
		break;

		case POS_ADJECTIVE:
		posString = "Adj";
		break;

		default:
		posString = "Unk";
		break;
	}

	printf("[%s: %.*s] ", posString, wordLength, word.wordStart);
}

void printPhrase(Phrase phrase) {
	switch (phrase.type) {
		case PHRASE_NOUN:
		{
			NounPhrase nounPhrase = *((NounPhrase *) &phrase);
			printf("[NP: ");
			printDictValue(nounPhrase.article);
			printDictValue(nounPhrase.noun);
			printf("]");
		} break;

		case PHRASE_NOUN_DIRECT_OBJECT:
		{
			NounPhrase nounPhrase = *((NounPhrase *) &phrase);
			printf("[DO_NP: ");
			printDictValue(nounPhrase.article);
			printDictValue(nounPhrase.noun);
			printf("]");
		} break;

		case PHRASE_NOUN_INDIRECT_OBJECT:
		{
			NounPhrase nounPhrase = *((NounPhrase *) &phrase);
			printf("[IO_NP: ");
			printDictValue(nounPhrase.article);
			printDictValue(nounPhrase.noun);
			printf("]");
		} break;

		case PHRASE_VERB:
		{
			printf("VP: ");
		} break;

		case PHRASE_PREPOSITION:
		{
			PrepositionalPhrase p_phrase = *((PrepositionalPhrase *) &phrase);
			printf("[PP: ");
			printDictValue(p_phrase.preposition);
			printPhrase(*((Phrase *) &p_phrase.nounPhrase));
			printf("]");
		} break;
	}

	printf(" ");
}

void printSentence(Sentence sentence) {
	printDictValue(sentence.mainVerb);
	puts("");
	printPhrase(*((Phrase *) &sentence.indirectObj));
	puts("");
	printPhrase(*((Phrase *) &sentence.directObj));
	puts("");
	printPhrase(*((Phrase *) &sentence.p_phrase));
	puts("");

	for (int i = 0; i < arrlen(sentence.phrases); i++) {
		Phrase phrase = sentence.phrases[i];
		printPhrase(*((Phrase *) &phrase));
		puts("");
	}
}

Sentence parseInput(char *input, DictHashMap *dictionary, DictValue **words, Phrase **phrases) { // TODO: parseInputEnglish?
	char *current = input;
	char *wordStart = current;
	bool appendWord = false;
	bool potentialPluralNoun = false; // TODO

	// Parse Words
	while (current < input + arrlen(input)) {
		// Skip whitespace and punctuation
		while ((*current == ' ' || *current == '\n' || *current == '\r' || *current == '\t' || *current == '.' || *current == ',') && current < input + arrlen(input))
			current++;

		//char *wordStart = current;
		if (!appendWord) {
			wordStart = current;
			//appendWord = false; // TODO
		}

		// Look for end of next word
		while (*current != ' ' && *current != '\n' && *current != '\r' && *current != '\t' && *current != '.' && *current != ',' && current < input + arrlen(input))
			current++;
		
		char *wordEnd = current;
		size_t wordLength = wordEnd - wordStart;

		// TODO: Check if wordStart == wordEnd. If so, must be at end of line

		//printf("'%.*s', %d\n", wordEnd - wordStart, wordStart, appendWord);

		// TODO: Parse quantifiers here

		// TODO: Handling plural verbs and nouns
		size_t hash = stbds_hash_string_len(wordStart, wordLength, hashSeed);

		//printf("Hash: %ld\n", hash);

		DictValue wordValue = hmget(dictionary, hash);
		//printf("{ %d, %d, %d, %d, %p, %p }\n\n", wordValue.partOfSpeechCount, wordValue.pos[0], wordValue.pos[1], wordValue.itemType, wordValue.wordStart, wordValue.wordEnd);

		if (wordValue.partOfSpeechCount > 0) {
			// Found Word
			wordValue.wordStart = wordStart;
			wordValue.wordEnd = wordEnd;

			arrput(*words, wordValue);

			appendWord = false; // aka. 0
		} else {
			// Did not find word
			if (appendWord < 1) {
				appendWord++; // TODO
			} else {
				appendWord = 0;
			}
		}
	}

	/*for (int i = 0; i < arrlen(*words); i++) {
		printDictValue((*words)[i]);
	}*/
	puts("");


	// give the man the gift
	// give the gift to the man
	// give to the man the gift
	// ------------------------
	// give - verb
	// the man - indirect object
	// the gift - direct object
	// to the man - prepositional phrase

	if (arrlen(*words) == 0) return;
	int currentWordIndex = 0;
	Sentence sentence = {0};
	bool success = parseVerb(words, &currentWordIndex, &sentence);
	if (success) {
		NounPhrase indirectObj = {0};
		NounPhrase directObj = {0};
		success = parseNounPhrase(words, &currentWordIndex, &indirectObj);
		if (success) {
			indirectObj.type = PHRASE_NOUN_INDIRECT_OBJECT;
			//arrput(sentence.phrases, *((Phrase *) &indirectObj));
			sentence.indirectObj = indirectObj;

			success = parseNounPhrase(words, &currentWordIndex, &directObj);
			if (success) {
				directObj.type = PHRASE_NOUN_DIRECT_OBJECT;
				//arrput(sentence.phrases, *((Phrase *) &directObj));
				sentence.directObj = directObj;
			} else {
				sentence.indirectObj = (NounPhrase) {0};
				directObj = indirectObj;
				indirectObj = (NounPhrase) {0};
				directObj.type = PHRASE_NOUN_DIRECT_OBJECT;
				sentence.directObj = directObj;
			}
		}


		PrepositionalPhrase p_phrase = {0};
		success = parsePrepositionalPhrase(words, &currentWordIndex, &p_phrase);
		if (success) {
			p_phrase.type = PHRASE_PREPOSITION;
			sentence.p_phrase = p_phrase;

			if (directObj.type == PHRASE_UNKNOWN) {
				// Try parsing direct object again, but after p_phrase in case it's located after it
				success = parseNounPhrase(words, &currentWordIndex, &directObj);
				if (success) {
					directObj.type = PHRASE_NOUN_DIRECT_OBJECT;
					sentence.directObj = directObj;
				}
			}
		}
	}

	//printSentence(sentence);
	puts("");

	return sentence;

	// Parse Phrases
	bool hasArticle = false;
	bool verbDone = false;
	bool directObjectDone = false; // TODO
	Phrase currentPhrase = {0};

	for (int i = 0; i < arrlen(*words); i++) {
		DictValue word = (*words)[i];

		if (!verbDone && word.pos[0] == POS_VERB) {
			currentPhrase.type = PHRASE_VERB;
			currentPhrase.word_end_i = i;

			arrput(*phrases, currentPhrase);
			currentPhrase = (struct Phrase) { 0, 0, 0 };
			verbDone = true;
		} else if (!verbDone && word.pos[0] != POS_VERB) {
			// Error
		} else {
			if ((word.pos[0] == POS_ARTICLE_DEF || word.pos[0] == POS_ARTICLE_INDEF || word.pos[0] == POS_ARTICLE_INDEF_AN)) {
				if (currentPhrase.type == PHRASE_UNKNOWN) {
					currentPhrase.type = PHRASE_NOUN_DIRECT_OBJECT;
					currentPhrase.word_start_i = i;
				}
			}

			if (word.pos[0] == POS_PREPOSITION) {
				currentPhrase.type = PHRASE_PREPOSITION;
				currentPhrase.word_start_i = i;
			}

			// TODO: Quantifier and Adjectives

			if (word.pos[0] == POS_NOUN && (currentPhrase.type == PHRASE_NOUN || currentPhrase.type == PHRASE_NOUN_DIRECT_OBJECT || currentPhrase.type == PHRASE_NOUN_INDIRECT_OBJECT)) { // && currentPhrase.type == PHRASE_NOUN_DIRECT/INDIRECT_OBJECT
				currentPhrase.word_end_i = i;

				arrput(*phrases, currentPhrase);
				currentPhrase = (struct Phrase) { 0, 0, 0 };
			} else if (word.pos[0] == POS_NOUN && currentPhrase.type == PHRASE_PREPOSITION) {
				currentPhrase.word_end_i = i;

				arrput(*phrases, currentPhrase);
				currentPhrase = (struct Phrase) { 0, 0, 0 };
			}
		}
	}


	// DEBUG: Print the words and phrases
	printf("Words:\n");
	for (int i = 0; i < arrlen(*words); i++) {
		DictValue word = (*words)[i];
		printDictValue(word);
		puts("");
	}

	printf("\nPhrases:\n");

	for (int i = 0; i < arrlen(*phrases); i++) {
		Phrase phrase = (*phrases)[i];
		printPhrase(phrase);
		//puts("");
	}
}

char handlePlayer(Sentence sentence) {
	char *verb = sentence.mainVerb.wordStart;
	NounPhrase directObj = sentence.directObj;
	NounPhrase indirectObj = sentence.indirectObj;

	if (strncmp(verb, "forage", MAX(6, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
		return 'f';
	} else if (strncmp(verb, "quit", MAX(4, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "exit", MAX(4, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
		return 'q';
	} else if (strncmp(verb, "look through", MAX(12, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "shuffle through", MAX(15, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "search through", MAX(14, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "see", MAX(3, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "show", MAX(4, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "look into", MAX(9, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
					if (strncmp(directObj.noun.wordStart, "backpack", MAX(8, directObj.noun.wordEnd - directObj.noun.wordStart)) == 0
						|| strncmp(directObj.noun.wordStart, "inventory", MAX(9, directObj.noun.wordEnd - directObj.noun.wordStart)) == 0) {
							return 'i';
					}
	} else if (strncmp(verb, "pick up", MAX(7, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
		switch (directObj.noun.itemType) {
			case ITEM_WOOD_AXE:
			{
				return 'p';
			} break;
		}
	} else if (strncmp(verb, "go", MAX(2, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0
				|| strncmp(verb, "walk", MAX(4, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
		DictValue noun;
		noun = directObj.noun;
		if (noun.wordEnd - noun.wordStart <= 0) {
			PrepositionalPhrase p_phrase = *((PrepositionalPhrase *) &sentence.p_phrase);
			DictValue preposition = p_phrase.preposition;
			if (strncmp(preposition.wordStart, "to", MAX(2, preposition.wordEnd - preposition.wordStart)) == 0) {
				noun = p_phrase.nounPhrase.noun;
			} else {
				return 0;
			}
		}

		if (strncmp(noun.wordStart, "north", MAX(5, directObj.noun.wordEnd - directObj.noun.wordStart)) == 0) {
			return 'n';
		} else if (strncmp(noun.wordStart, "south", MAX(5, noun.wordEnd - noun.wordStart)) == 0) {
			return 's';
		} else if (strncmp(noun.wordStart, "east", MAX(4, noun.wordEnd - noun.wordStart)) == 0) {
			return 'e';
		} else if (strncmp(noun.wordStart, "west", MAX(5, noun.wordEnd - noun.wordStart)) == 0) {
			return 'w';
		}
	} else if (strncmp(verb, "look around", MAX(11, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
		return 0;
	}

	return 0;
}

char handleInstrument(Sentence sentence) {
	PrepositionalPhrase p_phrase = *((PrepositionalPhrase *) &sentence.p_phrase);
	DictValue preposition = p_phrase.preposition;
	if (strncmp(preposition.wordStart, "with", MAX(4, preposition.wordEnd - preposition.wordStart)) == 0) {
		char *verb = sentence.mainVerb.wordStart;
		NounPhrase n_phrase = p_phrase.nounPhrase;
		switch (n_phrase.noun.itemType) {
			case ITEM_WOOD_AXE:
			{
				if (strncmp(verb, "cut down", MAX(8, sentence.mainVerb.wordEnd - sentence.mainVerb.wordStart)) == 0) {
					//printf("Cutting down the tree!\n");
					return 'c';
				}
			} break;
		}
	}

	return 0;
}

int main() {
	hashSeed = time(0);
	World world = initializeWorld(700, 700, time(0));
	
	int playerPositionX = world.spawnX;
	int playerPositionY = world.spawnY;
	float playerHealth = 100.0; // Out of 100.0
	const float maxHealth = 100.0;
	float playerEnergy = 100.0; // Out of 100.0
	const float maxEnergy = 100.0;

	// TODO: Should depend on player's total weight?
	const float movementEnergy = 2.0; // You can move 49 tiles at a time
	
	unsigned char inventoryItemCount = 0;
	Item inventoryItems[20] = {};
	
	int doPrintSurroundings = true;

	DictHashMap *dictionary = constructDictionary();
	
	char *input = NULL;
	while (true) {
		// New Wind Speed
		world.windSpeed = (rand() % 9) + 1; // 1 to 10
		
		int paths[4] = {};
	
		printSurroundingExplanation(&world, playerPositionX, playerPositionY, paths, doPrintSurroundings);
		if (!doPrintSurroundings)
			doPrintSurroundings = true;
		
		printPrompt("\n(%d, %d) > ", playerPositionX, playerPositionY);
		
		bool canceled = false;
		input = getInput(&canceled, input, NULL);
		if (canceled || input == NULL || arrlen(input) == 0 || (arrlen(input) == 1 && input[0] == '\n')) {
			if (input != NULL || arrlen(input) > 0)
				arrsetlen(input, 0);
			else printf("\n");
			
			playerEnergy += 1;

			continue;
		}

		if (input[0] == 'q' && input[1] == '\n') exit(0);

		
		DictValue *words = NULL;
		Phrase *phrases = NULL;
		Sentence sentence = parseInput(input, dictionary, &words, &phrases);
		char success = handlePlayer(sentence);
		if (!success) success = handleInstrument(sentence);

		//arrsetlen(input, 0);
		//continue; // TODO
		
		
		// TODO: Add command to (re)print surroundings.
		switch (/*input[0]*/ success) {
			case 'n':
			case 'u':
			{
				if (!paths[2]) {
					printf("You cannot go that way.\n\n");
					doPrintSurroundings = false;
					arrsetlen(input, 0);
					continue;
				}
				printf("You walk to the north.\n");
				playerPositionY--;
				if (playerPositionY < 0) playerPositionY = 0;
				playerEnergy -= movementEnergy;
			} break;
			case 's':
			case 'd':
			{
				if (!paths[3]) {
					printf("You cannot go that way.\n\n");
					doPrintSurroundings = false;
					arrsetlen(input, 0);
					continue;
				}
				printf("You walk to the south.\n");
				playerPositionY++;
				if (playerPositionY >= world.height) playerPositionY = world.height - 1;
				playerEnergy -= movementEnergy;
			} break;
			case 'w':
			case 'l':
			{
				if (!paths[0]) {
					printf("You cannot go that way.\n\n");
					doPrintSurroundings = false;
					arrsetlen(input, 0);
					continue;
				}
				printf("You walk to the west.\n");
				playerPositionX--;
				if (playerPositionX < 0) playerPositionX = 0;
				playerEnergy -= movementEnergy;
			} break;
			case 'e':
			case 'r':
			{
				if (!paths[1]) {
					printf("You cannot go that way.\n\n");
					doPrintSurroundings = false;
					arrsetlen(input, 0);
					continue;
				}
				printf("You walk to the east.\n");
				playerPositionX++;
				if (playerPositionX >= world.width) playerPositionX = world.width - 1;
				playerEnergy -= movementEnergy;
			} break;
			case 'p':
			{
				// Pick up the item, place into first empty slot of inventory, and increment the inventoryItemCount
				if (inventoryItemCount < 20) {
					if (arrlen(world.tiles[playerPositionX + playerPositionY * world.width].items) <= 0) {
						// TODO: Check overlay items
						OverlayItem overlayItem = world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0];
						if (OVERLAY_TYPE(overlayItem) != OVERLAY_NONE) {
							Item item = overlayToItem(overlayItem);

							if (ITEM_TYPE(item) != ITEM_NONE) {
								insertItemIntoInventory(inventoryItems, &inventoryItemCount, item); // TODO: Handle fail/success
								inventoryItemCount++;

								printf("You pick up ");
								printItemName(item, false, true, 1);
								printf(".\n");

								// TODO: Support whole list of overlayItems
								if (OVERLAY_AMT(overlayItem) - 1 > 0) {
									world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0] = createOverlayItem(OVERLAY_TYPE(overlayItem), OVERLAY_AMT(overlayItem) - 1);
								} else {
									world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0] = OVERLAY_NONE;
								}

								playerEnergy -= movementEnergy / 2.0; // TODO: Should also depend on weight of item
							}
						} else {
							printf("You search for something to pick up, but do not find anything.\n");
						}
					} else {
						Item item = arrlast(world.tiles[playerPositionX + playerPositionY * world.width].items);
						//printf("%d\n", item);
						arrpop(world.tiles[playerPositionX + playerPositionY * world.width].items);
	
						insertItemIntoInventory(inventoryItems, &inventoryItemCount, item); // TODO: handle fail/success (fail = inventory was full)
						inventoryItemCount++;
						
						printf("You pick up ");
						printItemName(item, false, true, 1);
						printf(".\n");

						playerEnergy -= movementEnergy / 2.0; // TODO: Should also depend on weight of item
					}
				} else {
					printf("Your inventory is full.\n");
				}
				
				doPrintSurroundings = false;
			} break;
			case 'c': // TODO: If inventory full, leave the logs on the ground.
			{
				// Check if any axe type is in inventory
				bool axeInInventory = false;
				for (int i = 0; i < 20; i++) {
					if (ITEM_TYPE(inventoryItems[i]) == ITEM_WOOD_AXE) {
						axeInInventory = true;
						break;
					}
				}
				
				if (!axeInInventory) {
					printf("You try pushing down the tree with your hands, but are unsuccessful.\n");
					playerEnergy -= movementEnergy * 6.0;
				} else if (OVERLAY_TYPE(world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0]) != OVERLAY_TREES) { // TODO: Make findOverlay() function to automatically go through the array
					printf("You search for a tree nearby to cut down, but do not find any.\n");
					playerEnergy -= movementEnergy / 2.0;
				} else {
					//world.tiles[playerPositionX + playerPositionY * world.width] = GRASS;
					int previousAmt = OVERLAY_AMT(world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0]);

					if (previousAmt > 1) {
						world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0] = createOverlayItem(OVERLAY_TREES, previousAmt - 1);
					} else {
						world.tiles[playerPositionX + playerPositionY * world.width].overlayItems[0] = OVERLAY_NONE;
					}

					printf("You cut down the tree. It crashes to the ground in front of you. You chop the tree up into logs and take them.\n");
					playerEnergy -= movementEnergy * 4.0;

					// NOTE: 2 Logs per tree - for now
					if (!insertItemIntoInventory(inventoryItems, &inventoryItemCount, createItem(ITEM_LOG, 31 / 2, 5))) { // TODO: Handle when failed (inventory is full)
						// TODO: Add two logs to ground items
						printf("You have no room to carry the Logs. You leave them on the ground.\n");
					} else if (!insertItemIntoInventory(inventoryItems, &inventoryItemCount, createItem(ITEM_LOG, 31 / 2, 5))) {
						// TODO: Add one log to ground items
						printf("You have no room to carry one of the Logs. You leave it on the ground.\n");
					}
				}
				doPrintSurroundings = false;
			} break;
			case 'i':
			{
				printf("Health: %.1f%%\n", playerHealth / maxHealth * 100.0);
				printf("Energy: %.1f%%\n", playerEnergy / maxEnergy * 100.0);
	
				// List items in inventory
				if (inventoryItemCount > 0) {
					int counts[ITEM_LAST + 1] = {}; // Counts for Stone, Log, and Axe
					
					// Go through inventory to get counts of each item type
					for (int i = 0; i < 20; i++) {
						counts[ITEM_TYPE(inventoryItems[i])] += 1;
					}
	
					// Find the last item type that has a count of more than 1
					unsigned char lastFilledSlot = ITEM_LAST + 1;
					for (int i = ITEM_LAST; i >= 0; i--) {
						if (counts[i] > 0) {
							lastFilledSlot = i;
							break;
						}
					}
					
					if (inventoryItemCount > 1)
						printf("You search through your backpack and find ");
					else printf("You look into your backpack and find ");
					
					// TODO
					for (int i = 0; i <= ITEM_LAST; i++) {
						if (i == lastFilledSlot && i != 0 && i != 1 && inventoryItemCount > 1) printf("and ");
						if (i != ITEM_NONE) {
							if (counts[i] > 1) printItemName(i, true, false, counts[i]);
							else if (counts[i] == 1) printItemName(i, false, false, 1);
							else if (counts[i] <= 0) continue;
							if (i != lastFilledSlot && counts[i] > 0) printf(", ");
						}
					}
					printf(".");
				} else {
					printf("Your backpack is empty.\n");
				}

				playerEnergy -= movementEnergy / 6.0;
				doPrintSurroundings = false;
			} break;
			case 'f': // Forage (foods and resources)
			{
				// Only works if current tile is grass
				// TODO
				Tile currentTile = world.tiles[playerPositionX + playerPositionY * world.width];
				if (TILE_GROUND(currentTile.type) == TILE_GRASS || TILE_GROUND(currentTile.type) == TILE_GRASS_STONE_N) {
					int random = rand();

					if (random % 10 == 0)
						printf("You forage through the grass and find nothing.\n");
					else {
						Item blueberry = createItem(ITEM_BLUEBERRY, 31 / 3, 2);
						printf("You forage through the grass and find ");
						printItemName(blueberry, false, false, 1);
						printf(".\n");

						if (!insertItemIntoInventory(inventoryItems, &inventoryItemCount, blueberry)) {
							printf("You have no room to carry ");
							printItemName(blueberry, false, true, 1);
							printf(". You leave it on the ground.\n");

							// TODO: Insert blueberry in tile's items
						}
					}
				}

				playerEnergy -= 2.0;
				doPrintSurroundings = false;
			} break;
			case 'b': // Botanize (herbs, plants, cotton, etc.)
			{

				playerEnergy -= 2.0;
				doPrintSurroundings = false;
			} break;
			case 'Q':
			case 'q':
			{
				exit(0);
			} break;
			case '?':
			case 'h':
			{
				printf("Commands:\nq - Quit\ni - Player info\nu - Up one square\nd - Down one square\nl - Left one square\nr - Right one square\nn - North\ns - South\ne - East\nw - West\np - Pick up item in current tile\nc - Cut down the tree and take the log\nf - Forage through the grass\n[Enter] - Print surrounding description\nh - Help\n");
				doPrintSurroundings = false;
			} break;
			case 0:
			break;
			default:
			{
				printf("Unknown command!\n");
			} break;
		}
		
		printf("\n");
		arrsetlen(input, 0);

		if (doPrintSurroundings) {
			playerEnergy += 1;
		}
	}
	
	return(0);
}
