//
//  main.cpp
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <unistd.h>
#include <pthread.h>
#include "dragonDB.h"

using namespace std;

int specify_how_many_cores() {
    long nc = sysconf(_SC_NPROCESSORS_ONLN);
    
    cout << "You have " << nc << " cores. How many cores would you like to use?\n";
    cin >> nc;
    return nc;
}




int main(int argc, const char * argv[]) {
    // insert code here...
    const int num_cores = 2; //specify_how_many_cores();
    cout << num_cores << "\n";
    
    //Create threads for each core
    pthread_t threads[num_cores];
    
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);
    
    for (int i = 0; i < num_cores; i++) {
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
        int res = pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[i], &attr, DoWork, NULL);
        cout << "here\n";
    }
    return 0;
}
