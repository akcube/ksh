#include "libs.h"
#include "proclist.h"

void insert_process(int id, char *s, struct Process **head){
	struct Process *ll = malloc(sizeof(struct Process));
	ll-> id = id;
	ll->str = malloc(strlen(s)+1);
	strcpy(ll->str, s);
	ll->next = *head;
	ll->prev = NULL;
	if(*head)
		(*head)->prev = ll;
	*head = ll;
}

int remove_process(int id, struct Process **head){
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
			return 1;
		}
		prev = cur;
		cur = cur->next;	
	}
	return -1;
}

#ifdef DEBUG
	void printList(Process **head){
		Process *cur = *head;
		while(cur){
			printf("%s<->", cur->str);
			cur = cur->next;
		}
		printf("NULL\n");
	}
#endif

char* get_process_name(int id, Process **head){
	struct Process *cur = *head;
	while(cur!=NULL){
		if(cur->id==id){
			return cur->str;
		}
		cur = cur->next;	
	}
	return NULL;
}

void init_proclist(ProcList *ll){
	ll->head = NULL;
	ll->insert_process = &insert_process;
	ll->get_process_name = &get_process_name;
	ll->remove_process = &remove_process;
	#ifdef DEBUG
		ll->printList = &printList;
	#endif
}

void destroy_proclist(ProcList *ll){
	Process *head = ll->head;
	Process *cur = head;
	while(cur!=NULL){
		cur = cur->next;
		free(head);
		head = cur;
	}
}