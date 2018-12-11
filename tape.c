#include "tape.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

int numThreads;
int size;
unsigned char *stream;

typedef struct{
	int boolean;
	int length;
	int start;
	int thread_num;
}head;

head *list;

int mod(int i){
	
	i = i % size;
	return i < 0 ?(i+size) : i;
	
}

void* tape_reader(void* args) {
	
	head *localData;
	localData = (head*)args;
	unsigned char *contents = malloc(sizeof(unsigned char)*localData->length);
	int start = localData->start;
	
	//printf("Thread %d: my start offset %d\n",localData->thread_num,start);
	if (localData->boolean == 0){
		
		for(int i = 0; i < localData->length; i++){
			contents[i]= stream[start];
			start = mod(start+1);
		}
		
	}else{
		for(int i = 0; i < localData->length; i++){
			start = mod(start-1);
			contents[i]= stream[start];
		}
	}
	//printf("Thread %d: my end offset %d\n",localData->thread_num ,start);
	localData->start = start;
	
	//write to file with correspnding threadnum
	char buff[7];
	sprintf(buff,"head%d",localData->thread_num);
	FILE* f = fopen(buff,"a");
	fwrite(contents,sizeof(unsigned char),localData->length,f);
	fclose(f);
	free(contents);
	return NULL;
}



int main(int argc, char **argv) {
	
	if (argc <= 1){
        printf("Tape Not Inserted\n");
        return 1;
    }
	
	FILE *fp;
    fp = fopen(argv[1], "r");
	if (fp == NULL){
		printf("Cannot Read Tape\n");
		return 1;
	}
	
	fseek( fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	stream = malloc(sizeof(unsigned char)*size);
	fread(stream,sizeof(unsigned char),size,fp);
	fclose(fp);	
	
	char input[100];
	
 	list = (head*)malloc(0);
	
	while(fgets(input, 100, stdin)){
		char* firstArg = strtok(input, " \n");
		char* secondArg = strtok(NULL, " \n");
		
		if (strcmp(firstArg,"HEAD") == 0){
			numThreads++;
			list = (head*)realloc(list,sizeof(head)*numThreads);
			
			char buff[7];
			sprintf(buff,"head%d",numThreads);
			
			fopen(buff,"w");
			remove(buff);
			char *ptr;
			int i = strtol(secondArg, &ptr, 10);
			
			list[numThreads-1].thread_num = numThreads;
			list[numThreads-1].start = mod(i);
			
			
			if (i < 0){
				printf("HEAD %d at %d\n",numThreads, i);
			}else{
				printf("HEAD %d at +%d\n",numThreads, i);
			}
			
			printf("\n");
			
			
		}else if (strcmp(firstArg,"READ") == 0){
			pthread_t heads[numThreads];
			char *ptr;
			int i = strtol(secondArg, &ptr, 10);
			
			int direction = i < 0 ? 1:0;
			int actual_num = i > 0 ? i: -i;
			
			for (int i = 0; i < numThreads; i++){
				// assign each struct length and direction
				list[i].length = actual_num;
				list[i].boolean = direction;
				if (pthread_create(heads+i, NULL, tape_reader, list+i) != 0) {
					
					perror("unable to create thread");
					return 1;
				}
			}
			for (int i = 0; i < numThreads; i++){
				if (pthread_join(heads[i], NULL) != 0) {
            		perror("unable to join thread");
					return 1;
				}
			}
			printf("Finished Reading\n");
			printf("\n");
			
			
		}else if (strcmp(firstArg,"QUIT") == 0){
			
			break;
		}
			
	}
	free(list);
	free(stream);
	
	
	
	return 0;
}
