#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "InstaQuack.h"

void initialize() { // int qID
	for (int i = 0; i < MAXQUEUES; i++) {
		queues[i].head = 0;
		queues[i].tail = 0;
		queues[i].counter = 0;
		queues[i].length = MAXENTRIES;
		//queues[i].qID = qID;
	}
}


struct topicEntry TE_create(char* photoURL, char* photoCaption) {  // int pubID, int entryNum, 
	struct topicEntry temp; // = (struct topicEntry *)malloc(sizeof(struct topicEntry));
	//temp.entryNum = entryNum;
	temp.pubID = 0;
	strcpy(temp.photoURL, photoURL);
	strcpy(temp.photoCaption, photoCaption);
	return temp;
} 


int enqueue(int TQ_ID, struct topicEntry TE) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			if ((queues[i].tail - queues[i].head) == -1) { // buffer is full
				return 0;
			}
			if (queues[i].buffer[queues[i].tail].entryNum != -1) {
				TE.entryNum = queues[i].counter;
				queues[i].counter++;
				queues[i].buffer[queues[i].tail] = TE;
				gettimeofday(&(queues[i].buffer[queues[i].tail].timeStamp), NULL);
				if (queues[i].tail == queues[i].length) {
					queues[i].tail = 0;
				}
				else {
					queues[i].tail++;
				}
				return 1;
			}
		}
	}
	return 0;
}


int dequeue(int TQ_ID, struct topicEntry *TE) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			if (queues[i].head == queues[i].tail) {  // buffer is empty
				return 0;
			}
			TE->entryNum = queues[i].buffer[queues[i].head].entryNum;
			TE->timeStamp = queues[i].buffer[queues[i].head].timeStamp;
			TE->pubID = queues[i].buffer[queues[i].head].pubID;
			strcpy(TE->photoURL, queues[i].buffer[queues[i].head].photoURL);
			strcpy(TE->photoCaption, queues[i].buffer[queues[i].head].photoCaption);

			// change the null entry to empty, might be unnecessary 
			if (queues[i].head == 0) {
				queues[i].buffer[queues[i].length].entryNum = 0;
			}
			else {
				queues[i].buffer[queues[i].head - 1].entryNum = 0;
			}
			// change the current entry to null
			queues[i].buffer[queues[i].head].entryNum = -1;

			// increment the head
			if (queues[i].head == queues[i].length) {
				queues[i].head = 0;
			}
			else {
				queues[i].head++;
			}
			return 1;
		}
	}
	return 0;
}


int isEmpty() {
	int empty = 0;
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].head == queues[i].tail) {
			empty++;
		}
	}
	if (empty == MAXQUEUES) {
		return 1;
	}
	return 0;
}


int getEntry(int TQ_ID, int lastEntry, struct topicEntry *TE) {
	int found = 0;
	int index;
	for (int i = 0; i < MAXQUEUES; i++) {
		if (queues[i].qID == TQ_ID) {
			// Case1: when queue is empty 
			if (queues[i].head == queues[i].tail) {
				return 0;
			}
			else {
				for (int j = 0; j < MAXENTRIES; j++) {
					index = (queues[i].head + j) % queues[i].length;
					if (queues[i].buffer[index].entryNum == lastEntry + 1) {
						found = 1;
						break;
					}
				}
				// Case 2: next entry is found in the topicQueue
				if (found) {
					TE->entryNum = queues[i].buffer[index].entryNum;
					TE->timeStamp = queues[i].buffer[index].timeStamp;
					TE->pubID = queues[i].buffer[index].pubID;
					strcpy(TE->photoURL, queues[i].buffer[index].photoURL);
					strcpy(TE->photoCaption, queues[i].buffer[index].photoCaption);
					return 1;
				}
				// Case 3: topicQueue is not empty but next entry is not found
				else {
					for (int k = 0; k < MAXENTRIES; k++) {
						index = (queues[i].head + k) % queues[i].length;
						if (queues[i].buffer[index].entryNum > lastEntry + 1) {
							found = 1;
							break;
						}
					}
					// Case 3-ii: The lastEntry + 1 was dequeued by the cleanup thread. 
					// The calling thread should update its lastEntry to the entryNum.
					if (found) {
						TE->entryNum = queues[i].buffer[index].entryNum;
						TE->timeStamp = queues[i].buffer[index].timeStamp;
						TE->pubID = queues[i].buffer[index].pubID;
						strcpy(TE->photoURL, queues[i].buffer[index].photoURL);
						strcpy(TE->photoCaption, queues[i].buffer[index].photoCaption);
						return queues[i].buffer[index].entryNum;
					}
					// Case 3-i: desired entry is yet to arrive
					else {
						return 0;
					}
				}
			}
		}
	}
	return 0;
}


void *publisher(void *pub_file) {
	char *pubfile = (char *)pub_file;
	FILE *fp;
	fp = fopen(pubfile, "r");
	char line[128];
	char *token;
	char *arg_arr[128];
	int argv_1;
	int count; 
	int counter = 0;
	while (fgets(line, 128, fp) != NULL) {
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \"\n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \"\n");
			counter++;
		}
		if (arg_arr[1] != NULL) {
			sscanf(arg_arr[1], "%d", &argv_1);
		}
		if (strcmp(arg_arr[0], "stop") == 0) {
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			double mili_sec = argv_1 / 1000000;
			sleep(mili_sec);
		}
		else if (strcmp(arg_arr[0], "put") == 0) {
			struct topicEntry TE = TE_create(arg_arr[2], arg_arr[3]);
			pthread_mutex_lock(&(mutex[count]));
			if (enqueue(argv_1, TE)) {
				pthread_mutex_unlock(&(mutex[count]));
			}
			else {

				fprintf(stdout, "subscriber %d: is empty, qID = %s\n", count, queues[argv_1].name);
				pthread_mutex_unlock(&(mutex[count]));
				sleep(1);
			}
		}
		else {
			fprintf(stderr, "Cannot read publisher command\n");
			exit(EXIT_FAILURE);
		}
		count++;
	}
}


void *subscriber(void *sub_file) {
	char *subfile = (char *)sub_file;
	FILE *fp;
	fp = fopen(subfile, "r");
	char line[128];
	char *token;
	char *arg_arr[128];
	int argv_1;
	int counter = 0;
	int count = 0;
	struct topicEntry TE;
	// pthread_self();
	while (fgets(line, 128, fp) != NULL) {
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \n");
			counter++;
		}
		if (arg_arr[1] != NULL) {
			sscanf(arg_arr[1], "%d", &argv_1);
		}

		if (strcmp(arg_arr[0], "stop") == 0) {
			break;
		}
		else if (strcmp(arg_arr[0], "sleep") == 0) {
			double mili_sec = argv_1 / 1000000;
			sleep(mili_sec);
		}
		else if (strcmp(arg_arr[0], "get") == 0) {
			pthread_mutex_lock(&(mutex[count]));
			if (getEntry(argv_1, count, &TE)) {
				pthread_mutex_unlock(&(mutex[count]));
			}
			else {
				fprintf(stdout, "subscriber %d: is empty, qID = %s\n", count, queues[argv_1].name);
				pthread_mutex_unlock(&(mutex[count]));
				sleep(1);
			}
		}
		else {
			fprintf(stderr, "Cannot read subscriber command\n");
			exit(EXIT_FAILURE);
		}
		count++;
	}
}


void cleanup(void *delta) {
	time_t *delta_t = (time_t *) delta;
	struct timeval curr_time;
	double diff_t;
	struct topicEntry TE;
	while (condition) {
		for (int i = 0; i < MAXQUEUES; i++) {
			for (int j = 0; j < MAXENTRIES; j++) {
				gettimeofday(&curr_time, NULL);
				diff_t = difftime(curr_time.tv_sec, queues[i].buffer[j].timeStamp.tv_sec);
				if (diff_t > (double) delta_t) {
					dequeue(queues[i].name, &TE);
				}
				// If we hit an entry that's not old enough, all entries after that aren't either
				else {
					break; 
				}
			}
		}
	}
}


int main(int argc, char const *argv[])
{
	int numpubs = 10;
	int numsubs = 10;

	int counter = 0;
	char line[128];
	char *token;
	char *arg_arr[128];
	int argv_1;

	char *pubfile, *subfile;

	int numtopics = 0;
	int qID; 

	FILE *fp = fopen(argv[1], "r");
	FILE *fpub, *fsub;
	time_t delta_t;
	if (fp == NULL) {
		fprintf(stderr, "File <%s> open failure.\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	while (fgets(line, 128, fp) != NULL) {
		// clean up arg_arr in each iteration 
		for (int i = 0; i < counter; i++) {
			arg_arr[i] = NULL;
		}
		counter = 0;
		token = strtok(line, " \"\n");
		while (token != NULL) {
			arg_arr[counter] = token;
			token = strtok(NULL, " \"\n");
			counter++;
		}
		if (strcmp(arg_arr[0], "create") == 0 && strcmp(arg_arr[1], "topic") == 0) {
			sscanf(arg_arr[2], "%d", &qID);
			queues[numtopics].qID = qID;
			strcpy(queues[numtopics].name, arg_arr[3]);
			queues[numtopics].head = 0;
			queues[numtopics].tail = 0;
			queues[numtopics].length = MAXENTRIES;
			queues[numtopics].counter = 0;
			numtopics++;
		}
		else if (strcmp(arg_arr[0], "query") == 0) {
			if (strcmp(arg_arr[1], "topics") == 0) {
				for (int i = 0; i < MAXQUEUES; i++) {
					printf("topic ID: %d, topic length: %d\n",
					queues[i].qID, queues[i].length);
				}
			}
			else if (strcmp(arg_arr[1], "publishers") == 0) {

			}
			else if (strcmp(arg_arr[1], "subscribers") == 0) {

			} 
			else {
				fprintf(stderr, "Error! Can't query %s\n", arg_arr[1]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "add") == 0) {
			if (strcmp(arg_arr[1], "publishers")) {
				//fpub = fopen(arg_arr[2], "r");
				pubfile = arg_arr[2];
			}
			else if (strcmp(arg_arr[1], "subscribers")){
				//fsub = fopen(arg_arr[2], "r");
				subfile = arg_arr[2];
			}
			else {
				fprintf(stderr, "Error! Can't add %s\n", arg_arr[1]);
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(arg_arr[0], "delta") == 0) {
			sscanf(argv[1], "%d", &argv_1);
			delta_t = argv_1;
		}
		else if (strcmp(arg_arr[0], "start") == 0) {
			break;
		}
		else {
			fprintf(stderr, "Error! Doesn't recognize input command\n");
			exit(EXIT_FAILURE);
		}
	}

	// initialize();

	for (int i = 0; i < MAXPUBS; i++) {
		// sprintf(pubargs[i].name, "%d", i);
		pthread_create(&pubs[i], &attr, publisher, (void *) &pubfile);
	}

	for(int i = 0; i < MAXSUBS; i++) {
    	// sprintf(subargs[i].name, "%d", i);
    	pthread_create(&subs[i], &attr, subscriber, (void *) &subfile);
  	}


  	// join
  	for (int i = 0; i < MAXPUBS; i++) {
  		if (pthread_join(pubs[i], NULL) != 0) {
  			fprintf(stderr, "pubs[%d] thread join failure\n", i);
  			exit(EXIT_FAILURE);
  		}
  	}

  	for (int i = 0; i < MAXSUBS; i++) {
  		if (pthread_join(subs[i], NULL) != 0) {
  			fprintf(stderr, "subs[%d] thread join failure\n", i);
  			exit(EXIT_FAILURE);
  		}
  	}

  	// condition = 0;

	return 0;
}

