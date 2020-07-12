#include "main.h"

bool isConsonant(char c) {
	char n = c;
	
	if (n > 90) n -= 32;
	
	switch (n) {
		case 'B':
		case 'C':
		case 'D':
		case 'F':
		case 'G':
		case 'H':
		case 'J':
		case 'K':
		case 'L':
		case 'M':
		case 'N':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
		return true;
		break;
		default:
		return false;
	}
}

bool isVowel(char c) {
	char n = c;
	
	if (n > 90) n -= 32;
	
	switch (n) {
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		return true;
		break;
		default:
		return false;
	}
}

gbString makePlural(gbString buffer) { // TODO
	gbString returnBuffer = buffer;
	// Manual check for irregulars
	
	switch (buffer[gb_string_length(buffer) - 1]) { // Switch on last character
		case 's':
		case 'o':
		{
			// Add es
			returnBuffer = gb_append_string_length(buffer, "es", 2);
		} break;
		case 'h':
		{
			// Check second to last for c or s (ch, sh)
			if (buffer[gb_string_length(buffer) - 2] == 'c' || buffer[gb_string_length(buffer) - 2] == 's') {
				returnBuffer = gb_append_string_length(buffer, "es", 2);
			}
		} break;
		case 'z':
		{
			returnBuffer = gb_append_string_length(buffer, "zes", 3);
		} break;
		case 'f':
		{
			gb_string_pop(buffer);
			returnBuffer = gb_append_string_length(buffer, "ves", 3);
		} break;
		case 'e':
		{
			if (buffer[gb_string_length(buffer) - 2] == 'f') {
				gb_string_pop(buffer);
				gb_string_pop(buffer);
				returnBuffer = gb_append_string_length(buffer, "ves", 3);
			} else {
				returnBuffer = gb_append_string_length(buffer, "s", 1);
			}
		} break;
		case 'x':
		{
			if (buffer[gb_string_length(buffer) - 2] == 'e') {
				// ex -> ices
				gb_string_pop(buffer);
				gb_string_pop(buffer);
				returnBuffer = gb_append_string_length(buffer, "ices", 4);
			} else {
				// x -> xes
				returnBuffer = gb_append_string_length(buffer, "es", 2);
			}
		} break;
		case 'y':
		{
			if (isConsonant(buffer[gb_string_length(buffer) - 2])) {
				// cons + y -> cons + ies
				gb_string_pop(buffer);
				returnBuffer = gb_append_string_length(buffer, "ies", 3);
			} else {
				// vowel + y -> vowel + ys
				returnBuffer = gb_append_string_length(buffer, "s", 1);
			}
		} break;
		default:
		{
			returnBuffer = gb_append_string_length(buffer, "s", 1);
		} break;
	}

	return returnBuffer;
}

gbString numberToText(int n) {
	//char *buffer = NULL;
	gbString buffer = gb_make_string("");
	
	if (n >= 100) {
		// Keep as number chars
		int numOfDigits = 0;
		int x = n;
		while (x > 0) {
			numOfDigits++;
			x = x / 10;
		}
		
		if (x == 0) numOfDigits = 1;

		char *s = malloc((numOfDigits + 1) * sizeof(char));
		stbsp_snprintf(s, numOfDigits, "%d", n);
		buffer = gb_append_string_length(buffer, s, numOfDigits);

		free(s);
	} else {
		unsigned char ones = 0;
		unsigned char tens = 0;
		
		ones = n % 10;
		tens = n / 10;
		
		// Handle 10 - 19, the teens
		if (tens == 1) {
			switch (ones) {
				case 0:
				{
					buffer = gb_append_string_length(buffer, "ten", 3);
				} break;
				case 1:
				{
					buffer = gb_append_string_length(buffer, "eleven", 6);
				} break;
				case 2:
				{
					buffer = gb_append_string_length(buffer, "twelve", 6);
				} break;
				default:
				{
					if (ones == 3) {
						buffer = gb_append_string_length(buffer, "thir", 4);
					} else if (ones == 4) {
						buffer = gb_append_string_length(buffer, "four", 4);
					} else if (ones == 5) {
						buffer = gb_append_string_length(buffer, "fif", 3);
					} else if (ones == 6) {
						buffer = gb_append_string_length(buffer, "six", 3);
					} else if (ones == 7) {
						buffer = gb_append_string_length(buffer, "seven", 5);
					} else if (ones == 8) {
						buffer = gb_append_string_length(buffer, "eigh", 4);
					} else if (ones == 9) {
						buffer = gb_append_string_length(buffer, "nine", 4);
					}
					
					buffer = gb_append_string_length(buffer, "teen", 4);
				} break;
			}
		} else if (tens == 0 || tens > 1) {
			if (tens > 1) {
				switch (tens) {
					case 2:
					{
						buffer = gb_append_string_length(buffer, "twenty", 6);
					} break;
					case 3:
					{
						buffer = gb_append_string_length(buffer, "thirty", 6);
					} break;
					case 4:
					{
						buffer = gb_append_string_length(buffer, "fourty", 6);
					} break;
					case 5:
					{
						buffer = gb_append_string_length(buffer, "fifty", 5);
					} break;
					case 6:
					{
						buffer = gb_append_string_length(buffer, "sixty", 5);
					} break;
					case 7:
					{
						buffer = gb_append_string_length(buffer, "seventy", 7);
					} break;
					case 8:
					{
						buffer = gb_append_string_length(buffer, "eighty", 6);
					} break;
					case 9:
					{
						buffer = gb_append_string_length(buffer, "ninety", 6);
					} break;
				}
				
				if (ones != 0) {
					buffer = gb_append_string_length(buffer, "-", 1);
				}
			}
	
			switch (ones) {
				case 0:
				{
					if (tens == 0) {
						buffer = gb_append_string_length(buffer, "zero", 4);
					}
				} break;
				case 1:
				{
					buffer = gb_append_string_length(buffer, "one", 3);
				} break;
				case 2:
				{
					buffer = gb_append_string_length(buffer, "two", 3);
				} break;
				case 3:
				{
					buffer = gb_append_string_length(buffer, "three", 5);
				} break;
				case 4:
				{
					buffer = gb_append_string_length(buffer, "four", 4);
				} break;
				case 5:
				{
					buffer = gb_append_string_length(buffer, "five", 4);
				} break;
				case 6:
				{
					buffer = gb_append_string_length(buffer, "six", 3);
				} break;
				case 7:
				{
					buffer = gb_append_string_length(buffer, "seven", 5);
				} break;
				case 8:
				{
					buffer = gb_append_string_length(buffer, "eight", 5);
				} break;
				case 9:
				{
					buffer = gb_append_string_length(buffer, "nine", 4);
				} break;
			}
		}
	}
	
	return buffer;
}

