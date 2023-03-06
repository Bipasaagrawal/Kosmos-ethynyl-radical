/*
 * kosmos-mcv.c (mutexes & condition variables)
 *
 * For UVic CSC 360, Summer 2022
 *
 * Here is some code from which to start.
 *
 * PLEASE FOLLOW THE INSTRUCTIONS REGARDING WHERE YOU ARE PERMITTED
 * TO ADD OR CHANGE THIS CODE. Read from line 133 onwards for
 * this information.
 */

#include <assert.h>
#include <pthread.h>
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
* many atoms may be blocked on some condition variable of
* our own devising. How do we ensure the program ends when
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


int hy=0,cr=0,a_num=0;
int q[100]; //carbon queue to store id 
int h[100]; //hydrogen queue to store ids
pthread_mutex_t mutexH =PTHREAD_MUTEX_INITIALIZER; //static initializtion of mutex 
pthread_cond_t condC=PTHREAD_COND_INITIALIZER; //static initilization of Carbon cond var 
pthread_cond_t condH=PTHREAD_COND_INITIALIZER; //static initilization of Hydrogen cond var 

void kosmos_init() {
    //mutex & cond_var already initialized statically
}

//make_radical takes in id of 2C and H along with name of the newest atom
//calls kosmos_log_add_entry and passes ids, and name to to logged 
void make_radical(int c1, int c2, int c3, char *n)
{
    kosmos_log_add_entry(a_num,c1,c2,c3, n);
}

//Function h_ready takes in the id of the hydrogen atom created
//It uses mutex and condiitonal var to determine the hydrogen queue 
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
    
    pthread_mutex_lock(&mutexH); //critical section starts - mutex locked
    h[hy]=id; 
    hy++;
    pthread_cond_signal(&condH); //signals that a hydrogen atom is ready
    if(cr>=2)
     {
        cr-=2; hy-=1;  a_num++; 
        make_radical(q[cr],q[cr+1],h[hy],name);   //When 2C and 1H is found, a radical is formed and their no.s reset
     }
    else {
        pthread_cond_wait(&condC,&mutexH); //If 1H is found, waiting for C..
            }
    pthread_mutex_unlock(&mutexH); //critical section ends - mutex unlocked
  
	return NULL;
}

//Function c_ready takes in the id of the carbon atom created
//It uses mutex and condiitonal var to determine the carbon queue 
//and calls make_radical when ready to form  Kosmos-ethynyl-radical
void *c_ready( void *arg )
{
	int id = *((int *)arg);
    char name[MAX_ATOM_NAME_LEN];

    sprintf(name, "c%03d", id);

#ifdef VERBOSE
	printf("%s now exists\n", name);
#endif

    pthread_mutex_lock(&mutexH); //critical section starts - mutex locked
    q[cr]=id;
    cr++;
    pthread_cond_signal(&condC); //signals that a carbon atom is ready
    if(cr>=2)
        {
            if(hy>=1)
            {
                hy-=1; cr=cr-2;  a_num++;
                make_radical(q[cr],q[cr+1],h[hy],name); //When 2C and 1H is found, a radical is formed and their no.s reset
            }
            else
            {
              pthread_cond_wait(&condH,&mutexH); //If 2C is found, waiting for H..
            }
        }
    pthread_mutex_unlock(&mutexH);   //critical section starts - mutex unlocked
    
	return NULL;
}


void wait_to_terminate(int expected_num_radicals) {
    /* A rather lazy way of doing it, but good enough for this assignment. */
    sleep(MAX_KOSMOS_SECONDS);
    kosmos_log_dump();

    exit(0);
}
