#ifndef __SHELL_PROC_LIST
#define __SHELL_PROC_LIST

typedef struct Process{
	pid_t id;
	uint64_t job_num;
	char *str;
	struct Process *next;
	struct Process *prev; 
} Process;

typedef struct ProcList{
	Process *head;
	void (*insert_process)(int, char *s, struct Process **h);
	int (*remove_process)(int id, Process **h);
	char* (*get_process_name)(int id, Process **h);
	uint32_t (*size)(struct ProcList *ll);
	#ifdef DEBUG
		void (*printList)(struct Process **h);
	#endif
} ProcList;

void init_proclist(ProcList *ll);
char* get_process_name(int id, Process **head);
void insert_process(int id, char *s, struct Process **head);
int remove_process(int id, struct Process **head);
void destroy_proclist(ProcList *ll);

#ifdef DEBUG
	void printList(Process **head);
#endif

#endif