//
//  main.cpp
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <unistd.h>

#include "../include/dragonDB.h"
#include <stdio.h>
#include <unistd.h>


using namespace std;

int specify_how_many_cores() {
    long nc = sysconf(_SC_NPROCESSORS_ONLN);
    
    cout << "You have " << nc << " cores. How many cores would you like to use?\n";
    cin >> nc;
    return nc;
}


void* print_stuff(void* args) {
    printf("ID: %lu, CPU: %d\n", pthread_self(), sched_getcpu());
    return 0;
}




int main(int argc, const char * argv[]) {
    // insert code here...
    const int num_cores = 2; //specify_how_many_cores();
    cout << num_cores << "\n";
    
    //Create threads for each core
    pthread_t threads[num_cores];
    cpu_set_t cpus;

    for (int i = 0; i < num_cores; i++) {
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
	//pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[i], NULL, print_stuff, NULL);
    }

    for (int i = 0; i < num_cores; i++) {
	pthread_join(threads[i], NULL);
    }

    return 0;
}
    
