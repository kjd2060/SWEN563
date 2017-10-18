/*
 * Queue.cc
 *
 *  Created on: Oct 18, 2017
 *      Author: kjd2060
 */

#include<iostream>
#include <cstdlib>

#define MAX_SIZE 100

template <class T>
class Queue{
    private:
        int item[MAX_SIZE];
        int rear;
        int front;
    public:
        Queue();
        void enqueue(T);
        T dequeue();
        int size();
        void display();
        bool isEmpty();
        bool isFull();
};

template <class T>
Queue<T>::Queue(){
    rear = -1;
    front = 0;
}

template <class T>
void Queue<T>::enqueue(T data){
        item[++rear] = data;
}

template <class T>
T Queue<T>::dequeue(){
        return item[front++];
}

template <class T>
void Queue<T>::display(){
    if(!this->isEmpty()){
        for(int i=front; i<=rear; i++)
            std::cout<<item[i]<<std::endl;
    }else{
        std::cout<<"Queue Underflow"<<std::endl;
    }
}

template <class T>
int Queue<T>::size(){
    return (rear - front + 1);
}

template <class T>
bool Queue<T>::isEmpty(){
    if(front>rear){
        return true;
    }else{
        return false;
    }
}

template <class T>
bool Queue<T>::isFull(){
    if(this->size()>=MAX_SIZE){
        return true;
    }
    else{
        return false;
    }
}
