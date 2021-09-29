#include "libs.h"
#include "prompt.h"

struct termios orig_termios;

string getline_inp;
int getline_pt;

/**
 * @brief Disables raw mode
 */
void disableRawMode() {
    if (tcsetattr(0, TCSAFLUSH, &orig_termios) == -1)
        throw_fatal_perror("tcsetattr");
}

/**
 * @brief Enables raw mode
 * @details Disables terminal echo. Can now directly read every character upon input.
 */
void enableRawMode() {
    if (tcgetattr(0, &orig_termios) == -1) throw_fatal_perror("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(0, TCSAFLUSH, &raw) == -1) throw_fatal_perror("tcsetattr");
}

// TODO: Make this look nicer :)
string get_line(){
    getline_inp = malloc(sizeof(char) * MAX_COMMAND_LENGTH);
    char c;
    char *retval = getline_inp;
    setbuf(stdout, NULL);
    enableRawMode();
    memset(getline_inp, 0, MAX_COMMAND_LENGTH);
    getline_pt = 0;
    int history_on = -1;
    while (read(0, &c, 1) == 1) {
        if(getline_pt==MAX_COMMAND_LENGTH){
            getline_inp = realloc(getline_inp, getline_pt<<1);
            retval = getline_inp;
        }
        if (iscntrl(c)) {
            if (c == 10) {
                printf("\n");
                break;
            }
            else if (c == 27) {
                char buf[3];
                buf[2] = 0;
                if (read(0, buf, 2) == 2) { // length of escape code
                    if(buf[1]=='A'){
                        // Up arrow was pressed
                        if(history_on < KSH.history.used-1)
                            history_on++;
                        while(getline_pt>0){
                            if (getline_inp[getline_pt-1] == 9) {
                                for (int i = 0; i < 7; i++) {
                                    printf("\b");
                                }
                            }
                            getline_inp[--getline_pt] = '\0';
                            printf("\b \b");
                        }
                        
                        int len = strlen(KSH.history.data[history_on]);
                        for(int k=0; k<len; k++){
                            getline_inp[getline_pt++] = KSH.history.data[history_on][k];
                            printf("%c", KSH.history.data[history_on][k]);
                        }
                    }
                    else if(buf[1]=='B'){
                        // Down arrow was pressed
                        if(history_on>=0)
                            history_on--;
                        while(getline_pt>0){
                            if (getline_inp[getline_pt-1] == 9) {
                                for (int i = 0; i < 7; i++) {
                                    printf("\b");
                                }
                            }
                            getline_inp[--getline_pt] = '\0';
                            printf("\b \b");
                        }
                        if(history_on >= 0){
                            int len = strlen(KSH.history.data[history_on]);
                            for(int k=0; k<len; k++){
                                getline_inp[getline_pt++] = KSH.history.data[history_on][k];
                                printf("%c", KSH.history.data[history_on][k]);
                            }
                        }
                    }
                }
            } else if (c == 127) { // backspace
                if (getline_pt > 0) {
                    if (getline_inp[getline_pt-1] == 9) {
                        for (int i = 0; i < 7; i++) {
                            printf("\b");
                        }
                    }
                    getline_inp[--getline_pt] = '\0';
                    printf("\b \b");
                }
            } else if (c == 9) { // TAB character
                getline_inp[getline_pt++] = c;
                for (int i = 0; i < 8; i++) { // TABS should be 8 spaces
                    printf(" ");
                }
            } else if (c == 4) {
                exit(0);
            } else {
                printf("%d\n", c);
            }
        } else {
            getline_inp[getline_pt++] = c;
            printf("%c", c);
        }
    }
    disableRawMode();
    return retval;
}

/**
 * @brief Displays the prompt, reads input and calls the parser.
 */
int prompt(){
	// Display prompt
    cprintf(FG_BLUE, 0, "<%s@%s:", KSH.username, KSH.hostname);
    cprintf(FG_YELLOW, 0, "%s", KSH.promptdir);
    cprintf(FG_BLUE, 0, "> ");

    // Read user command
    string linebuf = get_line();

   	if(!strcmp("exit", linebuf)) return 0;

    if(linebuf[0]!='\0') parse(linebuf);

    free(linebuf);
    linebuf = NULL;
    return 1;
}	