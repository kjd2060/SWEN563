#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <queue>

#define MIN_ARRIVAL (60)                                    // 1 minute, in seconds
#define MAX_ARRIVAL (240)                                   // 4 minutes, in seconds
#define MIN_TRANSACTION (30)                                // 30 seconds
#define MAX_TRANSACTION (480)                               // 8 minutes, in seconds
#define SECONDS_OPEN (25200)                                // 7 hours (9am to 4pm)
#define MAX_AMOUNT_OF_CUSTOMERS (SECONDS_OPEN/MIN_ARRIVAL)  // (25200/60) = 420 customers - this assumes that customers come every 60 seconds which is the quickest arrival time for consecutive customers
#define BILLION (1000000000L)								// used for timing conversions
#define MILLION (1000000L)

pthread_mutex_t lock;					// need to lock here to ensure things are declared properly


int current_queue_depth = 0;

double current_teller1_customer_wait_time;
double current_teller2_customer_wait_time;
double current_teller3_customer_wait_time;

double time_teller1_waited_for_customer;
double time_teller2_waited_for_customer;
double time_teller3_waited_for_customer;

// variables used for metrics later
int total_customers = 0;
int max_queue_depth = 0;
int customers_served_by_teller1 = 0;
int customers_served_by_teller2 = 0;
int customers_served_by_teller3 = 0;
int current_teller1_customer = 0;
int current_teller2_customer = 0;
int current_teller3_customer = 0;
int time_teller1_worked = 0;
int time_teller2_worked = 0;
int time_teller3_worked = 0;
int tellers_total_work_time = 0;
int teller1_longest_transaction = 0;
int teller2_longest_transaction = 0;
int teller3_longest_transaction = 0;

double time_customers_wait_for_teller1 = 0.0;
double time_customers_wait_for_teller2 = 0.0;
double time_customers_wait_for_teller3 = 0.0;

// timespec struct holds an interval broken into seconds and nanoseconds.  Used to tell 
struct timespec ts_customer_starts_waiting_in_queue;
struct timespec ts_customer_leaves_queue_to_teller;

double time_waited_by_teller1_for_customer = 0.0;
struct timespec ts_teller1_starts_to_wait_for_customer;
struct timespec ts_teller1_receives_customer;

double time_waited_by_teller2_for_customer = 0.0;
struct timespec ts_teller2_starts_to_wait_for_customer;
struct timespec ts_teller2_receives_customer;

double time_waited_by_teller3_for_customer = 0.0;

struct timespec ts_teller3_starts_to_wait_for_customer;
struct timespec ts_teller3_receives_customer;

double max_time_waited_by_teller1_customer = 0.0;
double max_time_waited_by_teller2_customer = 0.0;
double max_time_waited_by_teller3_customer = 0.0;

double max_time_waited_by_teller1_for_customer = 0.0;
double max_time_waited_by_teller2_for_customer = 0.0;
double max_time_waited_by_teller3_for_customer = 0.0;

double total_time_customers_waited_for_all_tellers = 0;
double total_time_waited_by_all_tellers_for_customer = 0;

bool bankOpen = false;
std::queue<int> *Q = new std::queue<int>;

/* This will sleep for a number milliseconds equal to the function input. 
 * CONVERT TO SIM TIME BEFORE CALLING THIS!!!!!!!!!!!!!!!!!!!!!!!!!!! (100ms = 60seconds).
 * Example from http://timmurphy.org/2009/09/29/nanosleep-in-c-c/
 */
void msSleep(int milliseconds)
{
    int ms = milliseconds; // length of time to sleep, in miliseconds
    struct timespec req = {0};
    req.tv_sec = 0;
    req.tv_nsec = ms * MILLION;
    nanosleep(&req, (struct timespec *)NULL);
}

/* Convert the world time in seconds to simulation time (60seconds => 100ms)*/
int convertToSimulationTime(int seconds)
{
    double converted_time_in_ms = 0;
    converted_time_in_ms = ((seconds)/600.0)*1000; // sim time = (1/600) * ms, eg. (60/600)*1000 = 100ms
    return (int)converted_time_in_ms;
}

/* This converts a millisecond input (real world time) to simultated time.
Input eg. 100. This 100ms is converted to (60) seconds of simulated time.*/
double msRealToSim(double ms) 
{
	return (ms / 100.0)*60.0;
}

/* This generates a random number within range of the passed parameters inclusively, while
 * maintaining a uniform distribution.  Algorithm from some research regarding generating
 * uniform int distributions
 */
int getRandomWithRange(int lower, int upper)
{
	double myRand = std::rand()/(1.0 + RAND_MAX);
	int range = upper - lower + 1;
	int randScaled = (myRand * range) + lower;
	return randScaled;
}


/*
 * Teller threads to follow (teller1, teller2, teller3).
 * These check to see if bank operating, dequeue the next customer from the queue and services
 * them for their generated time.  During that time, the thread sleeps to simulate business happening.  Metric calculation is
 * done internally to each thread and tally'd overall.  Mutex used to lock/unlock vars so shared tallys aren't fudged up.
*/
void* tellerThread1(void *arg)
{
    while(1)
	{																						    // always checking
        if (bankOpen)
		{																					// Bank is open
            clock_gettime( CLOCK_REALTIME, &ts_customer_leaves_queue_to_teller);                        // Fetch current time - customer leaving queue to teller1

            pthread_mutex_lock( &lock );																// lock to service variables

            if (Q->size() > 0)
			{                                                                               // if there are customers
                current_teller1_customer = Q->front();                                                    // Current customer for teller1 is the next one in the queue

				/*
				 * if the time the customer left the queue is after when he entered it and the teller has served a customer
				*/
                if (ts_customer_leaves_queue_to_teller.tv_sec + ts_customer_leaves_queue_to_teller.tv_nsec > // exact time customer left queue
					ts_customer_starts_waiting_in_queue.tv_sec + ts_customer_starts_waiting_in_queue.tv_nsec && // exact time customer started waiting in queue
					customers_served_by_teller1 > 0)
				{	
					/* teller1 serving a customer
					 * structure is used repeatedly.  the wait time is set to the difference between when the customer left the queue to when he entered the queue converted to ns
					 */
                    current_teller1_customer_wait_time = (( ts_customer_leaves_queue_to_teller.tv_sec - ts_customer_starts_waiting_in_queue.tv_sec ) + 
						(double)( ts_customer_leaves_queue_to_teller.tv_nsec - ts_customer_starts_waiting_in_queue.tv_nsec ) / (double)BILLION); 
                    time_customers_wait_for_teller1 += current_teller1_customer_wait_time;              // Update time waited by customers to be seen by teller1
                }

                if (current_teller1_customer_wait_time > max_time_waited_by_teller1_customer)
				{
                    max_time_waited_by_teller1_customer = current_teller1_customer_wait_time;           // Update max time waited by customer
                }

                clock_gettime( CLOCK_REALTIME, &ts_teller1_receives_customer);                       // End wait for customer - take from queue

                if (ts_teller1_receives_customer.tv_sec + ts_teller1_receives_customer.tv_nsec > ts_teller1_starts_to_wait_for_customer.tv_sec + 
					ts_teller1_starts_to_wait_for_customer.tv_nsec && customers_served_by_teller1 > 0)
				{
                    time_teller1_waited_for_customer = (( ts_teller1_receives_customer.tv_sec - ts_teller1_starts_to_wait_for_customer.tv_sec ) + 
						(double)( ts_teller1_receives_customer.tv_nsec - ts_teller1_starts_to_wait_for_customer.tv_nsec ) / (double)BILLION);
                    time_waited_by_teller1_for_customer += time_teller1_waited_for_customer;            // Update time waited by teller1 for customer
                }

                if (time_teller1_waited_for_customer > max_time_waited_by_teller1_for_customer)
				{
                    max_time_waited_by_teller1_for_customer = time_teller1_waited_for_customer;         // Update max time of teller1 waiting for customer if needed
                }

                pthread_mutex_unlock( &lock );															// no longer using shared resources, unlock them

                Q->pop(); // grab next customer

                printf("Teller1 is taking a customer        (%d)...\n",current_teller1_customer);

                msSleep(convertToSimulationTime(current_teller1_customer));                             // Sleep for current customer's transaction time to simulate business

                clock_gettime( CLOCK_REALTIME, &ts_teller1_starts_to_wait_for_customer);             // Start to wait for next customer
                
                printf("Teller1 is done with their customer (%d)...\n",current_teller1_customer);

                if (current_teller1_customer > teller1_longest_transaction)
				{
                    teller1_longest_transaction = current_teller1_customer;                             // Update the max transaction time 
                }

                time_teller1_worked += current_teller1_customer;                                        // Increment the time worked by teller1
                customers_served_by_teller1 += 1;                                                       // Increment the number of customers teller1 has processed
            }
            else                          // Bank is open but no customers!
			{                                                                                       
                pthread_mutex_unlock( &lock );
            }
        }
        else
		{                                                                                           // Bank closed
            clock_gettime( CLOCK_REALTIME, &ts_customer_leaves_queue_to_teller);                     // Fetch current time - customer leaving queue to teller1
            pthread_mutex_lock( &lock );
            if (Q->size() > 0)
			{                                                                               // still have customers, service them
                current_teller1_customer = Q->front();                                                    // Current customer for teller1 is the next one in the queue
                
                if (ts_customer_leaves_queue_to_teller.tv_sec + ts_customer_leaves_queue_to_teller.tv_nsec > 
					ts_customer_starts_waiting_in_queue.tv_sec + ts_customer_starts_waiting_in_queue.tv_nsec && 
					customers_served_by_teller1 > 0)
				{
                    current_teller1_customer_wait_time = (( ts_customer_leaves_queue_to_teller.tv_sec - ts_customer_starts_waiting_in_queue.tv_sec ) + 
						(double)( ts_customer_leaves_queue_to_teller.tv_nsec - ts_customer_starts_waiting_in_queue.tv_nsec ) / (double)BILLION);
                    time_customers_wait_for_teller1 += current_teller1_customer_wait_time;              // Update time waited by customers to be seen by teller1
                }

                if (current_teller1_customer_wait_time > max_time_waited_by_teller1_customer)
				{
                    max_time_waited_by_teller1_customer = current_teller1_customer_wait_time;           // Update max time waited by customer to be seen by teller1 if needed
                }

                clock_gettime( CLOCK_REALTIME, &ts_teller1_receives_customer);                       // End wait for customer - take from queue
                if (ts_teller1_receives_customer.tv_sec + ts_teller1_receives_customer.tv_nsec > 
					ts_teller1_starts_to_wait_for_customer.tv_sec + ts_teller1_starts_to_wait_for_customer.tv_nsec && 
					customers_served_by_teller1 > 0)
				{
                    time_teller1_waited_for_customer = (( ts_teller1_receives_customer.tv_sec - ts_teller1_starts_to_wait_for_customer.tv_sec ) + 
						(double)( ts_teller1_receives_customer.tv_nsec - ts_teller1_starts_to_wait_for_customer.tv_nsec ) / (double)BILLION);
                    time_waited_by_teller1_for_customer += time_teller1_waited_for_customer;            // Update time waited by teller1 for customer
                }

                if (time_teller1_waited_for_customer > max_time_waited_by_teller1_for_customer)
				{
                    max_time_waited_by_teller1_for_customer = time_teller1_waited_for_customer;         // Update max time of teller1 waiting for customer if needed
                }

                pthread_mutex_unlock( &lock );

				Q->pop();                                                                            // Next customer in queue taken to conduct business

                printf("Teller1 is taking a customer        (%d)...\n",current_teller1_customer);

                msSleep(convertToSimulationTime(current_teller1_customer));                             // Sleep for the current customer's transaction time to simulate business

                clock_gettime( CLOCK_REALTIME, &ts_teller1_starts_to_wait_for_customer);             // Start to wait for next customer
                
                printf("Teller1 is done with their customer (%d)...\n",current_teller1_customer);

                if (current_teller1_customer > teller1_longest_transaction)
				{
                    teller1_longest_transaction = current_teller1_customer;                             // Update the max transaction time
                }

                time_teller1_worked += current_teller1_customer;                                        // Increment the time worked by teller1
                customers_served_by_teller1 += 1;                                                       // Increment the number of customers teller1 has processed
			}
            else{                                                                                       // Bank closed and no customers!
                pthread_mutex_unlock( &lock );
                return NULL;
            }
        }
    }
    return NULL;
}

void* tellerThread2(void *arg)
{
    while(1)
	{
        if (bankOpen)
		{                                                                             // Bank is open
            
        }
        break;
    }
    return NULL;
}

void* tellerThread3(void *arg)
{
    while(1)
	{
        if (bankOpen)
		{                                                                             // Bank is open
            
        }
        break;
    }
    return NULL;
}

/* Queue thread.  Stores as ints representing the time
 * Generates two random numbers for arrival and transaction time.
 * sleep for the arrival time generated and then enqueues the transaction time.
 * transaction time also acts as customer id 
*/
void* queueThread(void *arg){
    int arrivalTime = 0;
    int transactionTime = 0;
    while(1){
        if (bankOpen){                                                             // Check if bank open
            arrivalTime = getRandomWithRange(MIN_ARRIVAL, MAX_ARRIVAL);                 // Generate random arrival time of customer
            transactionTime = getRandomWithRange(MIN_TRANSACTION, MAX_TRANSACTION);     // Generate random transaction time of customer
            msSleep(convertToSimulationTime(arrivalTime));                              // wait for customer to arrive before queueing
			Q->push(transactionTime);                                                // push transaction time of the customer to the queue
            clock_gettime( CLOCK_REALTIME, &ts_customer_starts_waiting_in_queue);    // Fetch current time - used to calculate when the customer was queued and how long they'll wait to be seen
            printf("Size of line: %d\n",Q->size());
            current_queue_depth = Q->size();                                              // Check size of queue
            if (current_queue_depth > max_queue_depth){                                 // Update max queue size with current if needed
                max_queue_depth = current_queue_depth;
            }
        }
        else{                                                                           // bank is closed
            break;
        }
    }
    return NULL;
}

/* Entry point
 * Seed the randomizer, create the thread IDs, initialize the mutex and open the bank for business
 * Spin up threads for queue and tellers, then sleep for the time necessary ((7 hours business day *60 minutes/hr) = 420 minutes*.1 second = 42
 * Also does metrics calculations
*/
int main(void) {
    srand(time(NULL));                          // Seed the randomizer

    // Thread Ids...3 teller threads, 1 queue thread
    pthread_t threadQ;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    if (pthread_mutex_init(&lock, NULL) != 0){  // Check mutex initialization
        printf("Mutex init failed\n");
        return 1;
    }

    printf("\nBank is now open!\n\n");
    bankOpen = true;                               // Bank is now Open

    // Creating threads

	pthread_create(&threadQ, NULL, queueThread, NULL);
    pthread_create(&thread1, NULL, tellerThread1, NULL);
    pthread_create(&thread2, NULL, tellerThread2, NULL);
    pthread_create(&thread3, NULL, tellerThread3, NULL);

    sleep(10);                                  // Simulate 9AM-4PM business day per assignment - 42 = full day
    bankOpen = false;                               // Bank is now Closed
    
    printf("Bank is now closed!\n\n");
    printf("People in queue still: %d\n",Q->size());

    pthread_join(threadQ, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);


    total_customers = (customers_served_by_teller1 + customers_served_by_teller2 + customers_served_by_teller3);
    tellers_total_work_time = (time_teller1_worked + time_teller2_worked + time_teller3_worked);

    printf("Total customers served: (%d)\nTeller work time: (%d)\n", total_customers, tellers_total_work_time);

    max_time_waited_by_teller1_customer = msRealToSim(max_time_waited_by_teller1_customer*1000);
	max_time_waited_by_teller1_for_customer = msRealToSim(max_time_waited_by_teller1_for_customer * 1000);

	printf("Maximum time customer waited for teller1: %f \nMaximum time teller1 waited for customer %f\n", max_time_waited_by_teller1_customer, max_time_waited_by_teller1_for_customer);


    max_time_waited_by_teller2_customer = msRealToSim(max_time_waited_by_teller2_customer*1000);
	max_time_waited_by_teller2_for_customer = msRealToSim(max_time_waited_by_teller2_for_customer * 1000);

    max_time_waited_by_teller3_customer = msRealToSim(max_time_waited_by_teller3_customer*1000);
    max_time_waited_by_teller3_for_customer = msRealToSim(max_time_waited_by_teller3_for_customer*1000);

    total_time_customers_waited_for_all_tellers = msRealToSim((time_customers_wait_for_teller1 + time_customers_wait_for_teller2 + time_customers_wait_for_teller3)*1000);
    total_time_waited_by_all_tellers_for_customer = msRealToSim((time_waited_by_teller1_for_customer + time_waited_by_teller2_for_customer + time_waited_by_teller3_for_customer)*1000);
    
	printf("Total time customer waited for tellers: %f \Total time tellers waited for customers %f\n",
			total_time_customers_waited_for_all_tellers, total_time_waited_by_all_tellers_for_customer);

    // need to figure out how to report all the variables
    
	// destroy the mutex lock
    pthread_mutex_destroy(&lock);
    return 0;
}
