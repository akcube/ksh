#include "libs.h"
#include "history.h"

/**
 * @brief Adds the last used command to local history
 * 
 * @param linebuf Last used command
 */
void log_history(string linebuf){
	if(KSH.history.used && !strncmp(linebuf, &KSH.history.data[0][0], MAX_COMMAND_LENGTH-2)) return;
	if(KSH.history.used > 0){
		int last = min(KSH.history.used-1, 18);
		for(int i=last; i>=0; i--){
			strncpy(&KSH.history.data[i+1][0], &KSH.history.data[i][0], MAX_COMMAND_LENGTH-2);
		}
	}
	strncpy(&KSH.history.data[0][0], linebuf, MAX_COMMAND_LENGTH-2);
	KSH.history.used++;
}