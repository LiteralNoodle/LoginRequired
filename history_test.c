
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <regex.h>
#include <curses.h>

#define ANSI_FOREGROUND_GREEN "\e[0;32m"
#define ANSI_FOREGROUND_RED "\e[0;31m"
#define ANSI_FOREGROUND_WHITE "\e[0;37m"

#define SKIP_CONNECTION_TEST true
#define DEBUG_NETWORK false

#define PASSWORD_MAX_LENGTH 512

typedef struct History {
	struct History* previous;
	char* message;
} tHistory; 

// push onto history list
void push_history(tHistory** head, char* message) {

	// don't add to history if it's the same as the last
	if (*head != NULL) {
		tHistory* p = *head;
		if (!strcmp(p->message, message)) {
			return;
		}
	}

	char* message_copy = (char*)malloc(sizeof(char) * PASSWORD_MAX_LENGTH);
	strcpy(message_copy, message);

	tHistory* h;
	h = (tHistory*)malloc(sizeof(tHistory));

	h->message = message_copy;
	h->previous = *head;
	*head = h; 

	// limit history to 10
	// since this is built-up one at a time, just have to delete the last one
	tHistory* cursor = *head;
	int limit = 3;
	for (int i = 0; i < limit; i++) {
		
		if (i == limit-1) {
			if (cursor->previous != NULL) {
                printw("Deleting... %s\n", cursor->previous->message);
				free(cursor->previous);
				cursor->previous = NULL;
			}
			return;
		}
				
		if (cursor->previous == NULL) {
			break;
		}

		cursor = cursor->previous;
	}

}

void get_input_with_message(WINDOW* win, char* message, char* userinput) {

	if (message) {
		printw("%s\n>>> ", message);
		refresh();
	}

	// using ncurses input 
	echo();
	wscanw(win, "%s", userinput);
	nl();
	noecho();
	refresh();

	return;

}

int main(void) {

	/*NCURSES STARTUP*/
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, true);

	// get screen size
	int yMax, xMax;
	getmaxyx(stdscr, yMax, xMax);

    tHistory* history_head = NULL;
    while (true) {
        // add to history
        char password[PASSWORD_MAX_LENGTH];
        get_input_with_message(stdscr, "Please enter a new password.", password);
        push_history(&history_head, password);
    }

	/*NCURSES CLEANUP*/
	endwin();
    
}