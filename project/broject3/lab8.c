#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#define TRUE 1
#define MAXQUEUES 4
#define MAXNAME 50
#define NUMTICKET 3

struct mealTicket {
	int ticketNum;
	char *dish;
};

struct MTQ {
	char name[MAXNAME];
	struct mealTicket * buffer;
	int head;
	int tail;
	int length;
};

struct MTQ registry[MAXQUEUES];

const char *name_array[] = {"Breakfast", "Lunch", "Dinner", "Bar"};

void initialize() {
	for (int i = 0; i < MAXQUEUES; i++) {
		strcpy(registry[i].name, name_array[i]); // unsure
		registry[i].buffer = (struct mealTicket *)malloc(sizeof(struct mealTicket) * NUMTICKET);
		registry[i].head = 0;
		registry[i].tail = 0;
		registry[i].length = 0;
	}
}

struct mealTicket* MT_create(int ticketNum, char *dish) {
	struct mealTicket* temp = (struct mealTicket*)malloc(sizeof(struct mealTicket));
	temp->dish = dish;
	temp->ticketNum = ticketNum;
	return temp;
}

int enqueue(char *MTQ_ID, struct mealTicket *MT) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (strcmp(MTQ_ID, registry[i].name) == 0) {
			registry[i].length++;
			if (registry[i].length > NUMTICKET) {
				registry[i].length--;
				return 0;
			}
			registry[i].buffer[registry[i].tail] = *MT;
			registry[i].tail++;
			return 1;
		}
	}
	return 0;
}

int dequeue(char *MTQ_ID, int ticketNum, struct mealTicket **MT) {
	for (int i = 0; i < MAXQUEUES; i++) {
		if (strcmp(MTQ_ID, registry[i].name) == 0) {
			registry[i].length--;
			if (registry[i].length < 0) {
				registry[i].length++;
				return 0;
			}
			*MT = &(registry[i].buffer[registry[i].head]);
			registry[i].head++;
			return 1;
			/*for (int j = 0; j < NUMTICKET; j++) {
				if (registry[i].buffer[j].ticketNum == ticketNum) {
					// memcpy();
					*MT = &(registry[i].buffer[j]);
					// registry[i].buffer[j] = NULL;
					registry[i].head++;
					registry[i].length--;
					if (registry[i].length < 0) {
						return 0;
					}
					return 1;
				}
			}*/
		}
	}
	return 0;
}

int isEmpty() {
	int empty = 0;
	for (int i = 0; i < MAXQUEUES; i++) {
		if (registry[i].length == 0) {
			empty++;
		}
	}
	// printf("empty: %d\n", empty);
	if (empty == MAXQUEUES) {
		return 1;
	}
	return 0;
}

int main(int argc, char const *argv[])
{
	initialize();

	struct mealTicket *mt1 = MT_create(0, "Sandwich");
	struct mealTicket *mt2 = MT_create(1, "Hamburger");
	struct mealTicket *mt3 = MT_create(2, "Mac-n-cheese");

	int enqueue_check = 0;

	enqueue_check = enqueue("Breakfast", mt1);
	if (enqueue_check) {
		printf("Enqueue when a queue is empty\n");
		printf("Test Case: D - Result: Success\n");
	}
	else {
		fprintf(stderr, "Breakfast enqueue Sandwich failed\n");
	}
	enqueue_check = enqueue("Breakfast", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Hamburger failed\n");
	}
	enqueue_check = enqueue("Breakfast", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Breakfast enqueue Mac-n-cheese failed\n");
	}


	enqueue_check = enqueue("Lunch", mt1);
	if (!enqueue_check) {
		fprintf(stderr, "Lunch enqueue Sandwich failed\n");
	}
	enqueue_check = enqueue("Lunch", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Lunch enqueue Hamburger failed\n");
	}
	enqueue_check = enqueue("Lunch", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Lunch enqueue Mac-n-cheese failed\n");
	}


	enqueue_check = enqueue("Dinner", mt1);
	if (!enqueue_check) {
		fprintf(stderr, "Dinner enqueue Sandwich failed\n");
	}
	enqueue_check = enqueue("Dinner", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Dinner enqueue Hamburger failed\n");
	}
	enqueue_check = enqueue("Dinner", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Dinner enqueue Mac-n-cheese failed\n");
	}


	enqueue_check = enqueue("Bar", mt1);
	if (!enqueue_check) {
		fprintf(stderr, "Bar enqueue Sandwich failed\n");
	}
	enqueue_check = enqueue("Bar", mt2);
	if (!enqueue_check) {
		fprintf(stderr, "Bar enqueue Hamburger failed\n");
	}
	enqueue_check = enqueue("Bar", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Bar enqueue Mac-n-cheese failed\n");
	}

	// Check: enqueue when a queue is full
	enqueue_check = enqueue("Bar", mt3);
	if (!enqueue_check) {
		fprintf(stderr, "Enqueue when a queue is full\n");
		fprintf(stderr, "Test Case: C - Result: Success\n");
	}

	printf("\n");

/*	for (int i = 0; i < MAXQUEUES; i++) {
		for (int j = 0; j < NUMTICKET; j++) {
			printf("%s %s %d\n", registry[i].name, registry[i].buffer[j].dish, registry[i].buffer[j].ticketNum);
		}
	}
*/

	struct mealTicket *MT;
	int dequeue_check = 0;
	int ticket_number = 0;
	while (TRUE) {

		if (isEmpty()) break;

		dequeue_check = dequeue("Breakfast", ticket_number, &MT);

		if (dequeue_check && ticket_number == 0) {
			printf("Dequeue when a queue is full\n");
			printf("Test Case: B - Result: Success\n");
		}

		if (dequeue_check) {
			printf("Queue: Breakfast - Ticket Number: %d - Dish: %s\n", MT->ticketNum, MT->dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
		}

		dequeue_check = dequeue("Lunch", ticket_number, &MT);
		if (dequeue_check) {
			printf("Queue: Lunch - Ticket Number: %d - Dish: %s\n", MT->ticketNum, MT->dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
		}
		
		dequeue_check = dequeue("Dinner", ticket_number, &MT);
		if (dequeue_check) {
			printf("Queue: Dinner - Ticket Number: %d - Dish: %s\n", MT->ticketNum, MT->dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
		}
		
		dequeue_check = dequeue("Bar", ticket_number, &MT);
		if (dequeue_check) {
			printf("Queue: Bar - Ticket Number: %d - Dish: %s\n", MT->ticketNum, MT->dish);
		}
		else {
			printf("Dequeue when the queue is empty, failed.\n");
			
		}

		printf("\n");
		
		ticket_number++;
	}

	// check Dequeue when a queue is empty
	dequeue_check = dequeue("Breakfast", ticket_number, &MT); 
	if (dequeue_check) {
			printf("Queue: Breakfast - Ticket Number: %d - Dish: %s\n", MT->ticketNum, MT->dish);
	}
	else {
		printf("Check: Dequeue when a queue is empty\n");
		printf("Test Case: A - Result: Success\n");
	}
	// free(MT);

	return 0;
}
