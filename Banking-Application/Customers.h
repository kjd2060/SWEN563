/*
 * Customers.h
 *
 *  Created on: Oct 18, 2017
 *      Author: kjd2060
 */

#ifndef CUSTOMERS_H_
#define CUSTOMERS_H_

#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

pthread_t Generate_Customer();

void Queue_Customer();


#endif /* CUSTOMERS_H_ */
