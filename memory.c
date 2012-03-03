#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define COLOR_RED	"\033[1;31m"
#define COLOR_YELLOW	"\033[1;33m"
#define COLOR_BLUE	"\033[1;34m"
#define COLOR_GREEN	"\033[1;32m"
#define COLOR_GREEN_N	"\033[0;32m"
#define COLOR_RESET	"\033[0m"

/* Struct */
typedef struct process_stat_t {
	char name[256];
	int pid;
	size_t memory;
	
	struct process_stat_t *next;
	
} process_stat_t;

/* Prototypes */
int isnum(char *val);
void TrimLf(char *str);
void Insert(process_stat_t **head, process_stat_t *data);
void GetMemoryOfPid(char *pid, process_stat_t *data);


/* Useful Functions */
int isnum(char *val) {
	while(*val && *val >= '0' && *val <= '9')
		val++;
	
	return (*val == '\0');
}

void TrimLf(char *str) {
	int len;

	len = strlen(str) - 1;

	while(*(str+len) == '\n') {
		*(str+len) = '\0';
		len--;
	}
}

void diep(char *s) {
	perror(s);
	exit(1);
}

void PrintColorSize(size_t memory) {
	if(memory > 102400)			/* 100 Mo	*/
		printf("%s", COLOR_RED);
	
	else if(memory > 61440)			/* 60 Mo	*/
		printf("%s", COLOR_YELLOW);
	
	else if(memory > 20480)			/* 20 Mo	*/
		printf("%s", COLOR_BLUE);
		
	else if(memory > 0)
		printf("%s", COLOR_GREEN);
		
	else
		printf("%s", COLOR_GREEN_N);
}

void Insert(process_stat_t **head, process_stat_t *data) {
	process_stat_t *use, *prev;
	
	/* Check if empty */	
	if(*head == NULL) {
		*head = data;
		return;
	}
	
	/* Check if First */
	if((*head)->memory >= data->memory) {		
		data->next = *head;
		*head = data;
		return;
	}
	
	use = *head;
	prev = *head;
	
	while(use != NULL && use->memory < data->memory) {
		prev = use;
		use = use->next;
	}
	
	prev->next = data;
	data->next = use;
}

/* Working */
void GetMemoryOfPid(char *pid, process_stat_t *data) {
	FILE *fp;
	char path[128], buffer[512];	
	
	/* Init Data */
	data->next = NULL;
	
	/* Init Path */
	sprintf(path, "/proc/%s/status", pid);
	
	fp = fopen(path, "r");
	if(fp == NULL) {
		perror("fopen");
		return;
	}
	
	while(fgets(buffer, sizeof(buffer), fp) != NULL) {
		TrimLf(buffer);
		
		if(strncmp(buffer, "Name:", 5) == 0) {
			strncpy(data->name, buffer+6, sizeof(data->name));
			data->name[strlen(buffer+6)] = '\0';
			
		} else if(strncmp(buffer, "Pid:", 4) == 0)
			data->pid = atoi(buffer+5);
		
		else if(strncmp(buffer, "VmRSS:", 6) == 0)
			data->memory = atoi(buffer+7);
	}
	
	fclose(fp);
}

int main() {
	DIR *fold;
	struct dirent *content;
	process_stat_t **head, *data, *new_elem;
	
	data = NULL;
	head = &data;
	
	if((fold = opendir("/proc")) == NULL) {
		printf("Cannot read /proc\n");
		exit(EXIT_FAILURE);
	}
	
	while((content = readdir(fold)) != NULL) {
		if(isnum(content->d_name)) {
			new_elem = (process_stat_t *) malloc(sizeof(process_stat_t));
			if(!new_elem)
				diep("malloc");

			GetMemoryOfPid(content->d_name, new_elem);
			Insert(head, new_elem);
		}
	}
	
	closedir(fold);
	
	/* Listing List */
	printf(" PID    | Name                        | VmRSS\n");
	printf("--------+-----------------------------+-----------------\n");
	data = *head;
	
	while(data != NULL) {
		PrintColorSize(data->memory);
		
		printf(" %-6d | %-27s | %.3f Mo %s\n", data->pid, data->name, (float) data->memory / 1024, COLOR_RESET);
		
		new_elem = data->next;
		free(data);
		data = new_elem;
	}
	
	printf("--------+-----------------------------+-----------------\n");
	printf(" PID    | Name                        | VmRSS\n");
	
	return 0;
}
