#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#include "Bank.h"
#include "Teller.h"
#include "Customers.h"
#include "Queue.cc"

/*
 * Some theories here:
 * We basically have three overall classes:
 * 		1. Customers
 * 		2. Tellers
 * 		3. The bank
 * The bank is our main; their should only ever exist one bank
 * The tellers are seperate threads that manage and handle customers
 */

void Test_Queue(){
	Queue<int> queue;
	    int choice, data;
	    while(1){
	        std::cout<<"\n1. Enqueue\n2. Dequeue\n3. Size\n4. Display all elements\n5. Quit";
	        std::cout<<"\nEnter your choice: ";
	        std::cin>>choice;
	        switch(choice){
	            case 1:
	            if(!queue.isFull()){
	            	std::cout<<"\nEnter data: ";
	            	std::cin>>data;
	                if(std::is_same<data, queue>::value)
	                	queue.enqueue(data);
	            }else{
	            	std::cout<<"Queue is Full"<<std::endl;
	            }
	            break;
	            case 2:
	            if(!queue.isEmpty()){
	            	std::cout<<"The data dequeued is :"<<queue.dequeue();
	            }else{
	            	std::cout<<"Queue is Emtpy"<<std::endl;
	            }
	                break;
	            case 3:
	            	std::cout<<"Size of Queue is "<<queue.size();
	                break;
	            case 4:
	                queue.display();
	                break;
	            case 5:
	                exit(0);
	                break;
	        }
	    }
}
int main(int argc, char *argv[]) {
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;

	Test_Queue();



	return EXIT_SUCCESS;
}
