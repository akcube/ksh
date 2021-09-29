#ifndef __TTY_COLORS_SHELL
#define __TTY_COLORS_SHELL

/**
 * Source: https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html
 * 
 * Quoting,
 * The way that most programs interact with the Unix terminal is through ANSI escape codes. 
 * These are special codes that your program can print in order to give the terminal instructions. 
 * Various terminals support different subsets of these codes, and it's difficult to find a "authoritative" 
 * list of what every code does. Wikipedia has a reasonable listing of them, as do many other sites.
 * 
 * Nevertheless, it's possible to write programs that make use of ANSI escape codes, and at least will work 
 * on common Unix systems like Ubuntu or OS-X
 */

#define TTY_RESET 	"\033[0m"

#define FG_BLACK	"\033[30;1m"
#define FG_RED 		"\033[31;1m"
#define FG_GREEN 	"\033[32;1m"
#define FG_YELLOW 	"\033[33;1m"
#define FG_BLUE 	"\033[34;1m"
#define FG_MAGENTA 	"\033[35;1m"
#define FG_CYAN 	"\033[36;1m"
#define FG_WHITE 	"\033[37;1m"

#define BG_BLACK 	"\033[40;1m"
#define BG_RED 		"\033[41;1m"
#define BG_GREEN 	"\033[42;1m"
#define BG_YELLOW 	"\033[43;1m"
#define BG_BLUE 	"\033[44;1m"
#define BG_MAGENTA 	"\033[45;1m"
#define BG_CYAN 	"\033[46;1m"
#define BG_WHITE 	"\033[47;1m"

void __reset_tty_colors();
void __thread_safe_reset_tty();
void cprintf(string FG, string BG, string format, ...);
void csprintf(char *buf, string FG, string BG, string format, ...);

#endif