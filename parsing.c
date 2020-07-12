#include "main.h"

// Gives back pointer starting at next non-whitespace character that appears after start
char *skipWhitespace(char *start, char *endBound, bool backwards) {
    char *current = start;

    if (backwards) {
        do {
            if (current <= endBound) break;
            --current;
        } while (*current == ' ' || *current == '\t' || *current == '\n' || *current == '\r');
    } else {
        while (*current == ' ' || *current == '\t' || *current == '\n' || *current == '\r') {
            ++current;
            if (current > endBound) break;
        }
    }

    return current;
}

// Will give back a pointer just after the word skipped.
char *skipWord(char *start, char *endBound, bool includeNumbers, bool includeSymbols, bool backwards) {
    char *current = start;
    while ((*current >= 'A' && *current <= 'Z') || (*current >= 'a' && *current <= 'z')
           || (includeNumbers && *current >= '0' && *current <= '9')
           || (includeNumbers && *current == '-') // Note: Technically this will also match numbers that have '-' in the middle and at end, but whatever TODO: Fix this
           || (includeSymbols && *current >= '!' && *current <= '/')
           || (includeSymbols && *current >= ':' && *current <= '@')
           || (includeSymbols && *current >= '[' && *current <= '`')
           || (includeSymbols && *current >= '{' && *current <= '~')) {
        if (backwards && current <= endBound) break;

        if (!backwards) ++current;
        else --current;

        if (!backwards && current > endBound) break;
    }
    return current;
}

// TODO
char *skipText() {
    return NULL;
}

// TODO
char *skipSymbols() {
    return NULL;
}

char *skipNumbers(char *start, char *endBound) {
    char *current = start;
    if (*current == '-') ++current; // Note: Not doing '+' here
    while (*current >= '0' && *current <= '9') {
        ++current;
        if (current > endBound) break;
    }
    return current;
}

// Specific to Edim Command Language
char *skipLineNumber(char *start, char *endBound) {
    char *current = start;
    if (*current == '\'') {
        ++current;
        current = skipWord(current, endBound, true, false, false);
    } else {
        // If symbol
        if ((*current >= '!' && *current <= '/')
            || (*current >= ':' && *current <= '@')
            || (*current >= '[' && *current <= '`')
            || (*current >= '{' && *current <= '~')) {
            //++current;
            if (*current == '.' || *current == '$') // Hacky, TODO
                ++current;
        } else {
            current = skipNumbers(current, endBound);
        }
    }
    return current;
}

/* Gets input, trims leading space, and puts into line char array
as long as it doesn't go over the max.

@line - array of characters to fill up
@max - maximum amount of characters allowed in line
@trimSpace - Whether to trim leading space (1) or not (0)
@return - the length of the line
*/
int parsing_getLine(char *line, int max, int trimSpace) {
    int c;
    int i = 0;
    
    /* Trim whitespace */
    while (trimSpace && ((c = getchar()) == ' ' || c == '\t'))
        ;
    
    if (!trimSpace) c = getchar();
    
    /* If there's nothing left, return */
    if (c == '\n') {
        line[i] = '\0';
        return 1; /* Including \0 */
    }
    
    /* Transfer input characters into line string */
    while (c != EOF && c != '\n' && i < max)
    {
        line[i] = (char) c;
        ++i;
        c = getchar();
    }
    
    /* End of string */
    line[i] = '\0';
    ++i; /* Includes '\0' in the length */
    
    return i;
}

// Gets input, trims leading space, and puts into a char stretchy buffer (dynamic array). Does not add null character at end.
// Returns the length of the buffer.
int parsing_getLine_dynamic(char **chars, int trimSpace) {
    int c;
    
    /* Trim whitespace */
    while (trimSpace && ((c = getchar()) == ' ' || c == '\t'))
        ;
    
    if (!trimSpace) c = getchar();
    
    /* If there's nothing left, return */
    if (c == '\n') {
        return 0;
    }
    
    /* Push input characters onto buffer */
    while (c != EOF && c != '\n') {
        arrput(*chars, (char) c);
        c = getchar();
    }
    
    return arrlen(*chars);
}

// TODO: Test on macOS and BSD!
// TODO: bool printNewLine
// TODO: Placeholder/Ghost text
// TODO: Autocomplete?
char *getInput(bool *canceled, char *inputBuffer, inputKeyCallback callback) {
    (*canceled) = false;
    int currentIndex = 0;
    int defaultLength = arrlen(inputBuffer);
    
    if (inputBuffer != NULL && arrlen(inputBuffer) > 0) {
        for (int i = 0; i < arrlen(inputBuffer); i++) {
            if (inputBuffer[i] == '\t')
                fputs("    ", stdout);
            else if (inputBuffer[i] == INPUT_ESC)
                colors_printf(COLOR_RED, "$");
            else putchar(inputBuffer[i]);
            ++currentIndex;
        }
    }
    
    char c;
    while ((c = getch()) != INPUT_ENDINPUT) { // Ctrl-Z for Windows, Ctrl-D for Linux
#ifdef _WIN32
		if (c == 0) continue;
        if (c == INPUT_SPECIAL1 || c == INPUT_SPECIAL2)
#else
        if (c == INPUT_SPECIAL1)
#endif
        {
#ifndef _WIN32
            if (getch_nonblocking() == INPUT_SPECIAL2) {
#endif
                if (callback != NULL) {
                    if (!callback(c, true, &inputBuffer, &currentIndex))
                        continue;
                }
                
                int specialkey = getch();
                if (specialkey == INPUT_LEFT) { // Left arrow
                    if (currentIndex != 0) {
                        if (inputBuffer[currentIndex - 1] == '\t')
                            fputs("\b\b\b\b", stdout);
                        else putchar('\b');
                        --currentIndex;
                    }
                } else if (specialkey == INPUT_RIGHT) { // Right arrow
                    if (currentIndex < arrlen(inputBuffer)) {
                        if (inputBuffer[currentIndex] == '\t')
                            fputs("    ", stdout);
                        else if (inputBuffer[currentIndex] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                        else putchar(inputBuffer[currentIndex]);
                        ++currentIndex;
                    }
#ifdef _WIN32
                } else if (specialkey == 83) { // Delete
#else
                } else if (specialkey == INPUT_DELETE1) {
                    int special2 = getch();
                    if (special2 == INPUT_DELETE2) {
#endif
                        if (currentIndex < arrlen(inputBuffer)) {
                            // Move all the characters down one
                            char *source = &(inputBuffer[currentIndex + 1]);
                            char *destination = &(inputBuffer[currentIndex]);
                            int amtToMove = &arrlast(inputBuffer) + 1 - source; // TODO
                            memmove(destination, source, sizeof(char) * amtToMove);
                            arrpop(inputBuffer);
                            // Print all of the characters that have been moved
                            for (int i = currentIndex; i < arrlen(inputBuffer); i++) {
                                if (inputBuffer[i] == '\t')
                                    fputs("    ", stdout);
                                if (inputBuffer[i] == INPUT_ESC)
                                    colors_printf(COLOR_RED, "$");
                                else putchar(inputBuffer[i]);
                            }
                            // Print spaces where the last character(s) used to be
                            fputs("    \b\b\b\b", stdout);
                            // Go back to where the cursor is
                            for (int i = 0; i < arrlen(inputBuffer) - currentIndex; i++) {
                                if (inputBuffer[arrlen(inputBuffer) - 1 - i] == '\t')
                                    fputs("\b\b\b\b", stdout);
                                else putchar('\b');
                            }
                        }
#ifndef _WIN32
                    }
#endif
                } else if (specialkey == INPUT_END) { // End key
                    for (int i = currentIndex; i < arrlen(inputBuffer); i++) {
                        if (inputBuffer[i] == '\t')
                            fputs("    ", stdout);
                        else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                        else putchar(inputBuffer[i]);
                    }
                    currentIndex = arrlen(inputBuffer);
                } else if (specialkey == INPUT_HOME) { // Home key
                    for (int i = 0; i < currentIndex; i++) {
                        if (inputBuffer[i] == '\t')
                            fputs("\b\b\b\b", stdout);
                        else putchar('\b');
                    }
                    currentIndex = 0;
                    //} else if (special == 115) { // Ctrl+Left // TODO
                    //} else if (special == 116) { // Ctrl+Right // TODO
                } else {
                    //printf("%d", special); // For debugging
                }
                continue;
#ifndef _WIN32
            } else { // TODO: Add to the location of the cursor
                // If escape key was hit and wasn't a special key
                // (most likely ESC hit by itself)
                // then print $ with next character after, and add
                // to buffer ESC + next character
                colors_printf(COLOR_RED, "$");
                //printf("%c", c);
                arrput(inputBuffer, 27);
                ++currentIndex;
                //arrlen(inputBuffer, c);
                continue;
            }
#endif
        }
        
        if (callback != NULL) {
            if (!callback(c, false, &inputBuffer, &currentIndex))
                continue;
        }
        
        // ASCII Control Keys
        if (c == INPUT_CTRL_X) {
            //arrsetlen(inputBuffer, 0);
            fputs("^X", stdout);
            arrfree(inputBuffer);
            inputBuffer = NULL;
            currentIndex = 0;
            (*canceled) = true;
            return NULL;
        } else if (c == INPUT_CTRL_C) {
            exit(0); // TODO
        } else if (c > 0 && c <= 26 && c != 13 && c != 8 && c != 9 && c != 10) { // Capture all other Control Keys (aside from Enter, backspace, and tab)
            continue;
        }
        
        if (c == '\n' || c == '\r') { // Enter
#ifdef _WIN32
            putchar('\r');
#endif
            putchar('\n');
            arrpush(inputBuffer, '\n');
            ++currentIndex;
            break;
        }
        if (c == '\t') { // Tab
            if (currentIndex == arrlen(inputBuffer)) {
                fputs("    ", stdout);
                arrput(inputBuffer, '\t');
            } else {
                fputs("    ", stdout);
                arrput(inputBuffer, 1);
                // Move all characters up one
                char *source = &(inputBuffer[currentIndex]);
                char *destination = &(inputBuffer[currentIndex + 1]);
                int amtToMove = &(inputBuffer[arrlen(inputBuffer) - 1]) - source;
                memmove(destination, source, sizeof(char) * amtToMove);
                // Change the character in the inputBuffer
                inputBuffer[currentIndex] = c;
                // Print the characters
                for (int i = currentIndex + 1; i < arrlen(inputBuffer); i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("    ", stdout);
                    else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                    else putchar(inputBuffer[i]);
                }
                // Move back to where the cursor is
                for (int i = 0; i < arrlen(inputBuffer) - currentIndex - 1; i++) {
                    if (inputBuffer[arrlen(inputBuffer) - 1 - i] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                }
            }
            
            ++currentIndex;
            continue;
        }
		if (c == INPUT_BACKSPACE || c == 8) // 8 = Ctrl+Backspace in Linux & regular backspace in Windows
        {
            if (currentIndex > 0) {
                if (currentIndex == arrlen(inputBuffer)) {
                    if (inputBuffer[currentIndex - 1] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else fputs("\b \b", stdout);
                    arrpop(inputBuffer);
                    --currentIndex;
                    continue;
                }
                
                if (inputBuffer[currentIndex - 1] == '\t')
                    fputs("\b\b\b\b", stdout);
                else fputs("\b \b", stdout);
                
                // Move all characters down one
                char *source = &(inputBuffer[currentIndex]);
                char *destination = &(inputBuffer[currentIndex - 1]);
                int amtToMove = &arrlast(inputBuffer) + 1 - source; // TODO
                memmove(destination, source, sizeof(char) * amtToMove);
                
                arrpop(inputBuffer);
                
                // Print all of the characters again
                for (int i = currentIndex - 1; i < arrlen(inputBuffer); i++) {
                    if (inputBuffer[i] == '\t')
                        fputs("    ", stdout);
                    else if (inputBuffer[i] == '\n' || inputBuffer[i] == '\r')
                        ; // Do Nothing
                    else if (inputBuffer[i] == INPUT_ESC)
                            colors_printf(COLOR_RED, "$");
                    else putchar(inputBuffer[i]);
                }
                
                // Print spaces where the last character(s) used to be
                fputs("    \b\b\b\b", stdout);
                
                // Move back to where the cursor is
                for (int i = 0; i <= arrlen(inputBuffer) - currentIndex; i++) {
                    if (inputBuffer[arrlen(inputBuffer) - 1 - i] == '\t')
                        fputs("\b\b\b\b", stdout);
                    else putchar('\b');
                }
                
                --currentIndex;
            }
            continue;
        }

#ifdef _WIN32
        if (c == INPUT_ESC) {
            colors_printf(COLOR_RED, "$");
            arrput(inputBuffer, c);
            continue;
        }
#endif
        
        if (currentIndex == arrlen(inputBuffer)) {
            putchar(c);
            arrput(inputBuffer, c);
        } else {
            putchar(c);

            // buf_add(inputBuffer, 1);
            arrsetlen(inputBuffer, arrlen(inputBuffer) + 1);

            // Move all characters up one
            char *source = &(inputBuffer[currentIndex]);
            char *destination = &(inputBuffer[currentIndex + 1]);
            int amtToMove = &(inputBuffer[arrlen(inputBuffer) - 1]) - source;
            memmove(destination, source, sizeof(char) * amtToMove);
            // Change the character in the inputBuffer
            inputBuffer[currentIndex] = c;
            // Print the characters
            for (int i = currentIndex + 1; i < arrlen(inputBuffer); i++) {
                if (inputBuffer[i] == '\t')
                    fputs("    ", stdout);
                else if (inputBuffer[i] == INPUT_ESC)
                    colors_printf(COLOR_RED, "$");
                else putchar(inputBuffer[i]);
            }
            // Move back to where the cursor is
            for (int i = 0; i < arrlen(inputBuffer) - currentIndex - 1; i++) {
                if (inputBuffer[arrlen(inputBuffer) - 1 - i] == '\t')
                    fputs("\b\b\b\b", stdout);
                else putchar('\b');
            }
        }
        ++currentIndex;
    }
    
    // If no input was made, free the buffer and return;
    if (arrlen(inputBuffer) - defaultLength <= 0) {
        if (inputBuffer != NULL)
            arrfree(inputBuffer);
    }
    
    // If there's no new line - which there shouldn't be -
    // then add it.
    if (inputBuffer != NULL && arrlast(inputBuffer) != '\n') {
        arrpush(inputBuffer, '\n'); // TODO: Was previously buf_add() for some reason
    }
    
    if (arrlen(inputBuffer) == 0) {
#ifdef _WIN32
        printf("^Z");
#else
        printf("^D");
#endif
    }
    
    return inputBuffer;
}

