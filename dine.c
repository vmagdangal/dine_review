#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "dawdle.c"

/*
* dine.c is the code for the dining philosophers
* problem. It takes in the constants
* NUM_PHILOSOPHERS and NUM_CYCLES to print out
* the states that each pthread are currently
* in. It runs through the main() function.
*
* @author: Vinpatrik Magdangal
*/

/* Modify the number of philosophers (processes) */
#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

/* Modify the number of eating/thinking 
cycles per philosopher */
int NUM_CYCLES = 1;

sem_t forks[NUM_PHILOSOPHERS];
sem_t printSem;

/* 0 : Empty State
*  1 : Changing
*  2 : Eating
*  3 : Thinking */
int philState[NUM_PHILOSOPHERS];

/* void PrintHeader()
* Prints out the header of the philosopher table.
*
* Takes no inputs, returns no value. */
void printHeader(){
    int i, j;
    for(i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|");
        for(j = 0; j < NUM_PHILOSOPHERS*2+3; j++){
            printf("=");
        }
    }
    printf("|\n");

    for(i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|");
        for(j = 0; j <= NUM_PHILOSOPHERS; j++){
            printf(" ");
        }
        printf("%c", 65 + i);
        for(j = 0; j <= NUM_PHILOSOPHERS; j++){
            printf(" ");
        }
    }
    printf("|\n");

    for(i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|");
        for(j = 0; j < NUM_PHILOSOPHERS*2+3; j++){
            printf("=");
        }
    }
    printf("|\n");
    return;
}

/* void PrintEnding()
* Prints out the footer of the philosopher table.
*
* Takes no inputs, returns no value. */
void printEnding(){
    int i, j;
    for(i = 0; i < NUM_PHILOSOPHERS; i++){
        printf("|");
        for(j = 0; j < NUM_PHILOSOPHERS*2+3; j++){
            printf("=");
        }
    }
    printf("|\n");
    return;
}

/* void PrintStatus()
* Prints out the header of the philosopher table.
*
* Takes no inputs, returns no value. */
void PrintStatus(){
    int philId, currentFork, leftFork, rightFork;

    for(philId = 0; philId < NUM_PHILOSOPHERS; philId++){
        leftFork = philId;
        rightFork = (philId+1) % NUM_PHILOSOPHERS;
        printf("| ");
        for(currentFork = 0; 
        currentFork < NUM_PHILOSOPHERS; 
        currentFork++){
            if(philState[philId] == 2){
                if(currentFork == leftFork){
                    printf("%c", 48 + currentFork);
                } else if (currentFork == rightFork){
                    printf("%c", 48 + currentFork);
                } else {
                    printf("-");
                }
            } else if(philState[philId] == 1) {
               if(currentFork == leftFork && philId%2 == 0){
                    printf("%c", 48 + currentFork);
                } else if (currentFork == rightFork && 
                            philId%2 == 1){
                    printf("%c", 48 + currentFork);
                } else {
                    printf("-");
                }
            } else {
                printf("-");
            }
        }
        printf(" ");
        if(philState[philId] == 2){
            printf("Eat");
            for(currentFork = 0; 
                currentFork <= NUM_PHILOSOPHERS-3; 
                currentFork++){
                printf(" ");
            }
        } else if (philState[philId] == 3) {
            printf("Think");
            for(currentFork = 0; 
                currentFork <= NUM_PHILOSOPHERS-5; 
                currentFork++){
                printf(" ");
            }
        } else {
            for(currentFork = 0; 
                currentFork <= NUM_PHILOSOPHERS; 
                currentFork++){
                printf(" ");
            }
        }
    }
printf("|\n");
}

/* void eat(int fork1, int fork2, int philId)
* Instruction for a philosopher to eat. To eat, a philosopher
* will first want to eat, and will attempt to pick up fork1. 
* They will then attempt to pick up fork2. During each change,
* it will modify its state in a global array.
*
* fork1 is the fork that a philosopher will pick up first.
* fork2 is the fork that a philosopher will pick up second.
* philId is the philosopher's number that it will use to update 
* its state.
*
* This function does not return anything. */
void eat(int fork1, int fork2, int philId){
    sem_wait(&forks[fork1]);
    sem_wait(&printSem);
    philState[philId] = 1;
    PrintStatus();
    sem_post(&printSem);

    sem_wait(&forks[fork2]);
    sem_wait(&printSem);
    philState[philId] = 2;
    PrintStatus();
    sem_post(&printSem);

    dawdle();
    
    philState[philId] = 1;
    sem_post(&forks[fork2]);
    sem_wait(&printSem);
    PrintStatus();
    sem_post(&printSem);
    
    philState[philId] = 0;
    sem_post(&forks[fork1]);
    sem_wait(&printSem);
    PrintStatus();
    sem_post(&printSem);
}

/* void think(int philId)
* Instruction for a philosopher to think. While thinking, a
* philosopher will essentially do nothing and ponder the 
* questions that the world throws at them. The thinking 
* philosopher will also update their own state to note 
* that they are thinking.
*
* philId is the philosopher's number that it will use to 
* update its state.
*
* This function does not return anything. */
void think(int philId){
    philState[philId] = 3;
    sem_wait(&printSem);
    PrintStatus();
    sem_post(&printSem);

    dawdle();

    philState[philId] = 0;
    sem_wait(&printSem);
    PrintStatus();
    sem_post(&printSem);
}

/* void *philosopher(void *id)
* The overall instruction that a philosopher follows. 
* Depending on NUM_CYCLES, a philosopher will eat and think 
* that many times. Based on the philosopher's id, it 
* will set it to pick either its left or right fork first
* in order to prevent deadlock.
*
* id is the id of the philosopher, which will determine if they
* will pick up the left or right fork first.
*
* This function does not return anything.
*/
void *philosopher(void *id){
    int philId=*(int*)id;
    int cycles;
    dawdle();
    for(cycles = 0; cycles < NUM_CYCLES; cycles++){
        if(philId%2 == 0){
            eat(philId, (philId+1)%NUM_PHILOSOPHERS, philId);
        } else {
            eat((philId+1)%NUM_PHILOSOPHERS, philId, philId);
        }
        think(philId);
    }

    return NULL;
}

/* int main()
* Main program that is called upon dine being run. This creates the
* philosophers and prints out the foundations for the tables. It will
* exit once all philosophers complete all of their cycles.
*
* main() takes in no parameters.
*
* This function returns 0 upon success, but will exit and print
* an error message upon some error.
*/
int main(int argc, char *argv[]) {
    int philId;
    int id[NUM_PHILOSOPHERS];
    pthread_t philosopherid[NUM_PHILOSOPHERS];

    if (NUM_PHILOSOPHERS <= 1) {
        perror("ERROR: Cannot accept 1 or less philosophers");
        exit(-1);
    }

    if(argc >= 2){
        NUM_CYCLES = atoi(argv[1]);
    }
    if (NUM_CYCLES <= 0) {
        perror("ERROR: Invalid number of cycles to run");
        exit(-1);
    }

    sem_init(&printSem, 0, 1);
    for(philId=0;philId<NUM_PHILOSOPHERS;philId++){
        id[philId]=philId;
        sem_init(&forks[philId], 0, 1);
    }

    printHeader();

    for (philId=0;philId<NUM_PHILOSOPHERS;philId++) {
        int res;
        res = pthread_create(
            &philosopherid[philId],
            NULL,
            philosopher,
            (void*) (&id[philId])
        );
        if ( -1 == res ) {
            fprintf(stderr,"Child %i: %s\n",philId,strerror(res));
            exit(-1);
        }
        philState[philId] = 0;
    }

    for(philId = 0 ; philId < NUM_PHILOSOPHERS ; philId++ ) {
        pthread_join(philosopherid[philId],NULL);
    }

    printEnding();

    for(philId=0;philId<NUM_PHILOSOPHERS;philId++){
        sem_destroy(&forks[philId]);
    }
    sem_destroy(&printSem);

    return 0; /* Success */
}
