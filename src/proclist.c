#include "libs.h"
#include "proclist.h"

/**
 * @brief Insert process into doubly linked list.
 * 
 * @param id Process id
 * @param s Process name
 * @param Process Pointer to head of linked list
 */
void insert_process(pid_t id, string s, struct Process **head){
	struct Process *ll = malloc(sizeof(struct Process));
	ll-> id = id;
	ll->job_num = ++KSH.jobs_spawned;
	ll->str = malloc(strlen(s)+1);
	strcpy(ll->str, s);
	ll->next = *head;
	ll->prev = NULL;
	if(*head)
		(*head)->prev = ll;
	*head = ll;
}

/**
 * @brief Searches doubly linked list in O(n) and removes matching process by id.
 * 
 * @param id Process id to delete by
 * @param Process Pointer to head of linked list.
 * 
 * @return 0 on successful removal. -1 if entry not found.
 */
int remove_process(pid_t id, struct Process **head){
	struct Process *cur = *head;
	struct Process *prev = NULL;
	while(cur!=NULL){
		if(cur->id==id){
			if(cur->next){
				(cur->next)->prev = prev;
			}
			if(prev){
				prev->next = NULL;
				prev->next = cur->next;
			}
			else{
				*head = cur->next;
			}
			free(cur);
			return 0;
		}
		prev = cur;
		cur = cur->next;	
	}
	return -1;
}

#ifdef DEBUG
	// Prints list to terminal. Debug only.
	void printList(Process **head){
		Process *cur = *head;
		while(cur){
			printf("%s<->", cur->str);
			cur = cur->next;
		}
		printf("NULL\n");
	}
#endif

/**
 * @brief Returns the process name of a process given id
 * @param id Process id
 * 
 * @return Pointer to string on success. NULL otherwise.
 */
string get_process_name(pid_t id, Process **head){
	struct Process *cur = *head;
	while(cur!=NULL){
		if(cur->id==id){
			return cur->str;
		}
		cur = cur->next;	
	}
	return NULL;
}

/**
 * @brief Returns the size of the linked list
 */
uint32_t size(ProcList *ll){
	Process *cur;
	uint32_t sz = 0;
	for(cur = ll->head; cur!=NULL; cur = cur->next, sz++);
	return sz;
}

/**
 * @brief Initializes a doubly linked list 
 */
void init_proclist(ProcList *ll){
	ll->head = NULL;
	ll->insert_process = &insert_process;
	ll->get_process_name = &get_process_name;
	ll->remove_process = &remove_process;
	ll->size = &size;
	#ifdef DEBUG
		ll->printList = &printList;
	#endif
}

/**
 * @brief Destroys and cleans up any resources used by the linked list
 */
void destroy_proclist(ProcList *ll){
	Process *head = ll->head;
	Process *cur = head;
	while(cur!=NULL){
		cur = cur->next;
		free(head);
		head = cur;
	}
}