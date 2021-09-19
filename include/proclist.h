#ifndef __SHELL_PROC_LIST
#define __SHELL_PROC_LIST

typedef struct Process{
	int id;
	char *str;
	struct Process *next;
	struct Process *prev; 
} Process;

typedef struct ProcList{
	Process *head;
	void (*insert_process)(int, char *s, struct Process **h);
	int (*remove_process)(int id, Process **h);
	char* (*get_process_name)(int id, Process **h);
	#ifdef DEBUG
		void (*printList)(struct Process **h);
	#endif
} ProcList;

void init_proclist(ProcList *ll);
char* get_process_name(int id, Process **head);
void insert_process(int id, char *s, struct Process **head);
int remove_process(int id, struct Process **head);

#ifdef DEBUG
	void printList(Process **head);
#endif

#endif