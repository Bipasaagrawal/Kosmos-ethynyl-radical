/*
 * kosmos-sem.c (semaphores)
 *
 * For UVic CSC 360, Summer 2022
 *
 * Here is some code from which to start.
 *
 * PLEASE FOLLOW THE INSTRUCTIONS REGARDING WHERE YOU ARE PERMITTED
 * TO ADD OR CHANGE THIS CODE. Read from line 136 onwards for
 * this information.
 */

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "logging.h"


/* Random # below threshold indicates H; otherwise C. */
#define ATOM_THRESHOLD 0.55
#define DEFAULT_NUM_ATOMS 40

#define MAX_ATOM_NAME_LEN 10
#define MAX_KOSMOS_SECONDS 5

/* Global / shared variables */
int  cNum = 0, hNum = 0;
long numAtoms;


/* Function prototypes */
void kosmos_init(void);
void *c_ready(void *);
void *h_ready(void *);
void make_radical(int, int, int, char *);
void wait_to_terminate(int);


/* Needed to pass legit copy of an integer argument to a pthread */
int *dupInt( int i )
{
	int *pi = (int *)malloc(sizeof(int));
	assert( pi != NULL);
	*pi = i;
	return pi;
}


int main(int argc, char *argv[])
{
	long seed;
	numAtoms = DEFAULT_NUM_ATOMS;
	pthread_t **atom;
	int i;
	int status;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: %s <seed> [<num atoms>]\n", argv[0]);
		exit(1);
	}

	if ( argc >= 2) {
		seed = atoi(argv[1]);
	}

	if (argc == 3) {
		numAtoms = atoi(argv[2]);
		if (numAtoms < 0) {
			fprintf(stderr, "%ld is not a valid number of atoms\n",
				numAtoms);
			exit(1);
		}
	}

    kosmos_log_init();
	kosmos_init();

	srand(seed);
	atom = (pthread_t **)malloc(numAtoms * sizeof(pthread_t *));
	assert (atom != NULL);
	for (i = 0; i < numAtoms; i++) {
		atom[i] = (pthread_t *)malloc(sizeof(pthread_t));
		if ( (double)rand()/(double)RAND_MAX < ATOM_THRESHOLD ) {
			hNum++;
			status = pthread_create (
					atom[i], NULL, h_ready,
					(void *)dupInt(hNum)
				);
		} else {
			cNum++;
			status = pthread_create (
					atom[i], NULL, c_ready,
					(void *)dupInt(cNum)
				);
		}
		if (status != 0) {
			fprintf(stderr, "Error creating atom thread\n");
			exit(1);
		}
	}

    /* Determining the maximum number of ethynyl radicals is fairly
     * straightforward -- it will be the minimum of the number of
     * hNum and cNum/2.
     */

    int max_radicals = (hNum < cNum/2 ? hNum : (int)(cNum/2));
#ifdef VERBOSE
    printf("Maximum # of radicals expected: %d\n", max_radicals);
#endif

    wait_to_terminate(max_radicals);
}

/*
* Now the tricky bit begins....  All the atoms are allowed
* to go their own way, but how does the Kosmos ethynyl-radical
* problem terminate? There is a non-zero probability that
* some atoms will not become part of a radical; that is,
* many atoms may be blocked on some semaphore of our own
* devising. How do we ensure the program ends when
* (a) all possible radicals have been created and (b) all
* remaining atoms are blocked (i.e., not on the ready queue)?
*/


/*
 * ^^^^^^^
 * DO NOT MODIFY CODE ABOVE THIS POINT.
 *
 *************************************
 *************************************
 *
 * ALL STUDENT WORK MUST APPEAR BELOW.
 * vvvvvvvv
 */

int radicals; //total no. of radicals made
sem_t mutex; //used for the critical section 
sem_t wait_c;//used for Carbon 
sem_t wait_h;//used for hydrogen 
 
int hy=0,cr=0,radicals=0;
int q[100]; 
int h[100];

void kosmos_init() {
    sem_init(&wait_c,0,0); // initializing semaphores
    sem_init(&wait_h,0,0);
    sem_init(&mutex,0,1); 
}

//Function h_ready takes in the id of the hydrogen atom created
//It uses semaphores to determine the hydrogen queue 
//and calls make_radical when ready to combine with 2 carbon atoms and
//form  Kosmos-ethynyl-radical
void *h_ready( void *arg )
{
	int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];
    sprintf(name, "h%03d", id);

#ifdef VERBOSE
	printf("%s now exists\n", name);
#endif
    sem_wait(&mutex); //critical section starts 
    h[0]=id;  //stores id for radical making
    hy++;
    sem_post(&wait_h); //indicates H is ready  
    
    if(cr>=2)
        {
            sem_wait(&wait_h);
            sem_wait(&wait_c);      
            sem_wait(&wait_c); //Since we have 1H and 2C, the values of semaphores (wait_h and wait_c) is decreased
            cr=cr-2;hy=hy-1;
            radicals++;
            make_radical(q[cr],q[cr+1],h[0],name); //To make new radical

        }
    sem_post(&mutex); //critical section starts
    return NULL;
    
}

//Function c_ready takes in the id of the carbon atom created
//It uses semaphores to determine the carbon queue 
//and calls make_radical when ready to form  Kosmos-ethynyl-radical
void *c_ready( void *arg )
{
	int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "c%03d", id);

#ifdef VERBOSE
	printf("%s now exists\n", name);
#endif
    sem_wait(&mutex); //critical section starts
    q[cr]=id;  
    cr++; //stores carbon id for radical making 
    sem_post(&wait_c); //indicated carbon is ready 
    if (hy>=1 && cr>=2)
            {
            sem_wait(&wait_h); 
            sem_wait(&wait_c);
            sem_wait(&wait_c); //Since we have 2C and 1H, the values of semaphores (wait_h and wait_c) is decreased
            cr=cr-2;
            hy=hy-1;
            radicals++;
            make_radical(q[cr],q[cr+1],h[0],name); //to make new radical
            }
    sem_post(&mutex); //critical section ends 
	return NULL;
}

//make_radical takes in id of 2C and H along with name of the newest atom
//calls kosmos_log_add_entry and passes ids, and name to to logged 
void make_radical(int c1, int c2, int h, char *maker)
{
#ifdef VERBOSE
	//fprintf(stdout, "A ethynyl radical was made: c%03d  c%03d  h%03d\n",
		//c1, c2, h);
#endif
    kosmos_log_add_entry(radicals, c1, c2, h, maker);
}

void wait_to_terminate(int expected_num_radicals) {
    /* A rather lazy way of doing it, for now. */
    sleep(MAX_KOSMOS_SECONDS);
    kosmos_log_dump();
    sem_destroy(&wait_c);
    sem_destroy(&wait_h);
    sem_destroy(&mutex);
    exit(0);
}
