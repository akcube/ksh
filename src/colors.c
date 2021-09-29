#include "libs.h"
#include "colors.h"

void cprintf(string FG, string BG, string format, ...){
	
	// Set foreground Bitand background colors
	if(FG) printf(FG);
	if(BG) printf(BG);

	// Print format string
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	// Reset to default
	__reset_tty_colors();
}

void csprintf(char *buf, string FG, string BG, string format, ...){

	if(FG) strcpy(buf, FG); 
	if(BG) strcat(buf, BG);
	char secbuf[1024];

	va_list args;
	va_start(args, format);
	vsprintf(secbuf, format, args);
	va_end(args);

	strcat(buf, secbuf);
}

void __reset_tty_colors(){
	printf(TTY_RESET);
	printf(FG_WHITE);
}

void __thread_safe_reset_tty(){
	write(STDOUT_FILENO, TTY_RESET, strlen(TTY_RESET));
	write(STDOUT_FILENO, FG_WHITE, strlen(FG_WHITE));
}