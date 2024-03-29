#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <queue>

#define MIN_ARRIVAL (60)
// 1 minute, in seconds
#define MAX_ARRIVAL (240)
// 4 minutes, in seconds
#define MIN_TRANSACTION (30)
// 30 seconds
#define MAX_TRANSACTION (480)
// 8 minutes, in seconds
#define SECONDS_OPEN (25200)
// 7 hours (9am to 4pm)
#define MAX_AMOUNT_OF_CUSTOMERS (SECONDS_OPEN/MIN_ARRIVAL)
// (25200/60) = 420 customers - this assumes that customers come every 60 seconds which is the quickest arrival time for consecutive customers
#define BILLION (1000000000L)
// used for timing conversions
#define MILLION (1000000L)
#define NUM_TELLERS (3)

pthread_mutex_t lock;					// need to lock here to ensure things are declared properly

int current_queue_depth = 0;
int arg;
unsigned int i;

double time_teller_waited_for_customer[NUM_TELLERS];
double current_teller_customer_wait_time[NUM_TELLERS];

// variables used for metrics later
int total_customers = 0;
int max_queue_depth = 0;
int average_transaction_time = 0;
int customers_served_by_tellers[NUM_TELLERS] = { 0 };

double current_teller_customer[NUM_TELLERS] = { 0 };
double time_teller_worked[NUM_TELLERS] = { 0 };
int teller_longest_transaction[NUM_TELLERS] = { 0 };

double tellers_total_work_time = 0;
int max_transaction_time = 0;

double time_customers_wait_for_teller =  0;
double average_queue_time = 0;

// timespec struct holds an interval broken into seconds and nanoseconds.  Used to tell 
struct timespec ts_customer_starts_waiting_in_queue;
struct timespec ts_customer_leaves_queue_to_teller;

struct timespec ts_teller_starts_to_wait_for_customer[NUM_TELLERS];
struct timespec ts_teller_receives_customer[NUM_TELLERS];


double time_waited_by_teller_for_customer[NUM_TELLERS] = { 0 };
double average_teller_wait = 0;
double max_time_customer_waited_for_teller = 0;
double max_time_waited_by_teller_for_customer = 0;

double total_time_customers_waited_for_all_tellers;
double total_time_waited_by_all_tellers_for_customer;

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

/* Convert the world time in seconds to simulation time (.1s real time => 60 seconds sim time, 1s = 1000ms real time...1000ms real time = 600s sim time => 600000ms sim time  )*/
double convertToSimulationTime(int seconds)
{
    double converted_time_in_ms = 0;
    converted_time_in_ms = ((seconds)/600.0)*1000; // sim time = (1/600) * ms, eg. (60/600)*1000 = 100ms
    return converted_time_in_ms;
}

/* This converts a millisecond input (real world time) to simultated time.
Input eg. 100. This 100ms is converted to (60) seconds of simulated time.
100ms rt => 60s st, 1ms rt = 60/100s rt => 1ms rt = .6s st => 1ms rt = 600ms st */
double msRealToSim(double ms) 
{
	return (ms * 600);
}

/* This generates a random number within range of the passed parameters inclusively, while overall
   producing a uniform distribution of generated values.
 */
int getRandomWithRange(int lower, int upper)
{
	double myRand = std::rand()/(1.0 + RAND_MAX);
	int range = upper - lower + 1;
	int randScaled = (myRand * range) + lower;
	if ( randScaled > max_transaction_time ){
		max_transaction_time = randScaled;
	}
	return randScaled;
}


/*
 * Teller threads to follow (teller1, teller2, teller3).
 * These check to see if bank operating, dequeue the next customer from the queue and services
 * them for their generated time.  During that time, the thread sleeps to simulate business happening.  Metric calculation is
 * done internally to each thread and tally'd overall.  Mutex used to lock/unlock vars so shared tallys aren't fudged up.
*/
void* tellerThread( void *args )
{
	int arg = *((int *) args);
	//int teller_number = arg+1;
    while(1)
	{
    	// always checking
        if (bankOpen)
		{
        	// Bank is open
            clock_gettime( CLOCK_REALTIME, &ts_customer_leaves_queue_to_teller);
            // Fetch current time - customer leaving queue to teller1

            pthread_mutex_lock( &lock );
            // lock to service variables

            if (Q->size() > 0)
			{
            	// if there are customers
                current_teller_customer[arg] = Q->front();
                // Current customer for teller1 is the next one in the queue

				/*
				 * if the time the customer left the queue is after when he entered it and the teller has served a customer
				*/
                if (ts_customer_leaves_queue_to_teller.tv_sec + ts_customer_leaves_queue_to_teller.tv_nsec > // exact time customer left queue
					ts_customer_starts_waiting_in_queue.tv_sec + ts_customer_starts_waiting_in_queue.tv_nsec && // exact time customer started waiting in queue
					customers_served_by_tellers[arg] > 0)
				{	
					/* teller serving a customer
					 * structure is used repeatedly.  the wait time is set to the difference between when the customer left the queue to when he entered the queue converted to ns
					 */
                    current_teller_customer_wait_time[arg] = (( ts_customer_leaves_queue_to_teller.tv_sec - ts_customer_starts_waiting_in_queue.tv_sec ) +
						(double)( ts_customer_leaves_queue_to_teller.tv_nsec - ts_customer_starts_waiting_in_queue.tv_nsec ) / (double)BILLION); 
                    time_customers_wait_for_teller += current_teller_customer_wait_time[arg];
                    // Update time waited by customers to be seen by teller1
                }

                if (current_teller_customer_wait_time[arg] > max_time_customer_waited_for_teller)
				{
                    max_time_customer_waited_for_teller = current_teller_customer_wait_time[arg];
                    //printf("current max queue time is %f", max_time_customer_waited_for_teller);
                    // Update max time waited by customer
                }

                clock_gettime( CLOCK_REALTIME, &ts_teller_receives_customer[arg]);
                // End wait for customer - take from queue

                if (ts_teller_receives_customer[arg].tv_sec + ts_teller_receives_customer[arg].tv_nsec > ts_teller_starts_to_wait_for_customer[arg].tv_sec +
					ts_teller_starts_to_wait_for_customer[arg].tv_nsec && customers_served_by_tellers[arg] > 0)
				{
                    time_teller_waited_for_customer[arg] = (( ts_teller_receives_customer[arg].tv_sec - ts_teller_starts_to_wait_for_customer[arg].tv_sec ) +
						(double)( ts_teller_receives_customer[arg].tv_nsec - ts_teller_starts_to_wait_for_customer[arg].tv_nsec ) / (double)BILLION);
                    time_waited_by_teller_for_customer[arg] += time_teller_waited_for_customer[arg];
                    // Update time waited by teller1 for customer
                }

                if (time_teller_waited_for_customer[arg] > max_time_waited_by_teller_for_customer)
				{
                    max_time_waited_by_teller_for_customer = time_teller_waited_for_customer[arg];
                    // Update max time of teller1 waiting for customer if needed
                }

                pthread_mutex_unlock( &lock );
                // no longer using shared resources, unlock them

                Q->pop();
                // grab next customer

                //printf("Teller%d is taking a customer        (%ds)...\n",teller_number,current_teller_customer[arg]);

                msSleep(convertToSimulationTime(current_teller_customer[arg]));
                // Sleep for current customer's transaction time to simulate business

                clock_gettime( CLOCK_REALTIME, &ts_teller_starts_to_wait_for_customer[arg]);
                // Start to wait for next customer
                
                //printf("Teller%d is done with their customer (%ds)...\n",teller_number,current_teller_customer[arg]);

                if (current_teller_customer[arg] > teller_longest_transaction[arg])
				{
                    teller_longest_transaction[arg] = current_teller_customer[arg];
                    // Update the max transaction time
                }

                time_teller_worked[arg] += current_teller_customer[arg];
                // Increment the time worked by teller1
                customers_served_by_tellers[arg] += 1;
                // Increment the number of customers teller1 has processed
            }
            else
            	// Bank is open but no customers!
			{                                                                                       
                pthread_mutex_unlock( &lock );
            }
        }
        else
		{

        	// Bank closed
            clock_gettime( CLOCK_REALTIME, &ts_customer_leaves_queue_to_teller);
            // Fetch current time - customer leaving queue to teller1
            pthread_mutex_lock( &lock );
            if (Q->size() > 0)
			{
            	// still have customers, service them
                current_teller_customer[arg] = Q->front();
                // Current customer for teller1 is the next one in the queue
                
                if (ts_customer_leaves_queue_to_teller.tv_sec + ts_customer_leaves_queue_to_teller.tv_nsec > 
					ts_customer_starts_waiting_in_queue.tv_sec + ts_customer_starts_waiting_in_queue.tv_nsec && 
					customers_served_by_tellers[arg] > 0)
				{
                    current_teller_customer_wait_time[arg] = (( ts_customer_leaves_queue_to_teller.tv_sec -
                    		ts_customer_starts_waiting_in_queue.tv_sec ) +
                    		(double)( ts_customer_leaves_queue_to_teller.tv_nsec -
                    				ts_customer_starts_waiting_in_queue.tv_nsec ) / (double)BILLION);

                    time_customers_wait_for_teller += current_teller_customer_wait_time[arg];
                    // Update time waited by customers to be seen by teller1
                }

                if (current_teller_customer_wait_time[arg] > max_time_customer_waited_for_teller)
				{
                    max_time_customer_waited_for_teller = current_teller_customer_wait_time[arg];
                    // Update max time waited by customer to be seen by teller if needed
                }

                clock_gettime( CLOCK_REALTIME, &ts_teller_receives_customer[arg]);
                // End wait for customer - take from queue

                if (ts_teller_receives_customer[arg].tv_sec + ts_teller_receives_customer[arg].tv_nsec >
					ts_teller_starts_to_wait_for_customer[arg].tv_sec + ts_teller_starts_to_wait_for_customer[arg].tv_nsec &&
					customers_served_by_tellers[arg] > 0)
				{
                    time_teller_waited_for_customer[arg] = (( ts_teller_receives_customer[arg].tv_sec -
                    		ts_teller_starts_to_wait_for_customer[arg].tv_sec ) +
                    		(double)( ts_teller_receives_customer[arg].tv_nsec -
                    				ts_teller_starts_to_wait_for_customer[arg].tv_nsec ) / (double)BILLION);

                    time_waited_by_teller_for_customer[arg] += time_teller_waited_for_customer[arg];
                    // Update time waited by teller for customer
                }

                if (time_teller_waited_for_customer[arg] > max_time_waited_by_teller_for_customer)
				{
                    max_time_waited_by_teller_for_customer = time_teller_waited_for_customer[arg];
                    // Update max time of teller1 waiting for customer if needed
                }

                pthread_mutex_unlock( &lock );

				Q->pop();
				// Next customer in queue taken to conduct business

                //printf("Teller%d is taking a customer        (%ds)...\n",teller_number,current_teller_customer[arg]);

                msSleep(convertToSimulationTime(current_teller_customer[arg]));
                // Sleep for the current customer's transaction time to simulate business

                clock_gettime( CLOCK_REALTIME, &ts_teller_starts_to_wait_for_customer[arg]);
                // Start to wait for next customer
                
                //printf("Teller%d is done with their customer (%ds)...\n",teller_number,current_teller_customer[arg]);

                if (current_teller_customer[arg] > teller_longest_transaction[arg])
				{
                    teller_longest_transaction[arg] = current_teller_customer[arg];
                    // Update the max transaction time
                }

                time_teller_worked[arg] += current_teller_customer[arg];
                // Increment the time worked by teller
                customers_served_by_tellers[arg] += 1;
                // Increment the number of customers teller has processed
            }
            else{
            	// Bank closed and no customers!
                pthread_mutex_unlock( &lock );
                break;
            }
        }
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
        if (bankOpen){
        	// Check if bank open
            arrivalTime = getRandomWithRange(MIN_ARRIVAL, MAX_ARRIVAL);
            // Generate random arrival time of customer
            transactionTime = getRandomWithRange(MIN_TRANSACTION, MAX_TRANSACTION);
            // Generate random transaction time of customer
            msSleep(convertToSimulationTime(arrivalTime));
            // wait for customer to arrive before queueing
			Q->push(transactionTime);
			// push transaction time of the customer to the queue
            clock_gettime( CLOCK_REALTIME, &ts_customer_starts_waiting_in_queue);
            // Fetch current time - used to calculate when the customer was queued and how long they'll wait to be seen
            //printf("Size of line: %d\n",Q->size());
            current_queue_depth = Q->size();
            // Check size of queue
            if (current_queue_depth > max_queue_depth){
            	// Update max queue size with current if needed
                max_queue_depth = current_queue_depth;
            }
        }
        else{
        	// bank is closed
            break;
        }
    }
    return NULL;
}

/* Entry point
 * Seed the randomizer, create the thread IDs, initialize the mutex and open the bank for business
 * Spin up threads for queue and tellers, then sleep for the time necessary
 * ((7 hours business day *60 minutes/hr) = 420 minutes*.1 second = 42
 * Also does metrics calculations
*/
int main(void) {
    int *args = &arg;
	srand(time(NULL));                          // Seed the randomizer

    // Thread Ids...3 teller threads, 1 queue thread
    pthread_t threadQ;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    if (pthread_mutex_init(&lock, NULL) != 0){
    	// Check mutex initialization
        printf("Mutex init failed\n");
        return 1;
    }

    printf("\nBank is now open!\n\n");
    bankOpen = true;
    // Bank is now Open

    // Creating threads
	pthread_create(&threadQ, NULL, queueThread, NULL);
	arg = 0;
    pthread_create(&thread1, NULL, tellerThread, args);
	arg = 1;
    pthread_create(&thread2, NULL, tellerThread, args);
	arg = 2;
    pthread_create(&thread3, NULL, tellerThread, args);

    sleep(42);
    // Simulate 9AM-4PM business day per assignment
    bankOpen = false;
    // Bank is now Closed
    
    printf("Bank is now closed!\n\n");
    printf("People in queue still: %d\n",Q->size());

    pthread_join(threadQ, NULL);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    //calculating total customers served
    total_customers = (customers_served_by_tellers[0] + customers_served_by_tellers[1] + customers_served_by_tellers[2]);

    //calculating average queue time
    average_queue_time = time_customers_wait_for_teller/total_customers;
    average_queue_time = msRealToSim(average_queue_time);
    average_queue_time /= 1000.0;

    //calculating average transaction time
    tellers_total_work_time = (time_teller_worked[0] + time_teller_worked[1] + time_teller_worked[2]);
    average_transaction_time = tellers_total_work_time/total_customers;

    //calculating average teller wait
    for(i = 0; i < sizeof(time_teller_waited_for_customer); i++){
    	average_teller_wait += time_teller_waited_for_customer[i];
    }
    average_teller_wait /= total_customers;
    average_teller_wait = msRealToSim(average_teller_wait);
    average_teller_wait /= 1000.0;

    //calculating max time customer waited for teller
    max_time_customer_waited_for_teller = msRealToSim(max_time_customer_waited_for_teller);
    max_time_customer_waited_for_teller /= 1000.0;

    //calculating max time teller waited for customer
    max_time_waited_by_teller_for_customer = msRealToSim(max_time_waited_by_teller_for_customer);


    printf("1. Total customers served: (%d People)\n", total_customers);
    printf("2. Average Queue Time: (%fs)\n", average_queue_time);
    printf("3. Average Time Customer Spends with Teller: (%ds)\n", average_transaction_time);
    printf("4. Average Time Teller Waited for Customer: (%fs)\n", average_teller_wait);
    printf("5. Maximum Time Customer Waited for Teller: (%fs)\n", max_time_customer_waited_for_teller);
    printf("6. Maximum Time Teller Waited for Customer: (%fs)\n", max_time_waited_by_teller_for_customer);
    printf("7. Maximum Transaction Time: (%ds)\n", max_transaction_time);
    printf("8. Maximum Queue Depth: (%d People) \n", max_queue_depth);


	// destroy the mutex lock
    pthread_mutex_destroy(&lock);
    return 0;
}
