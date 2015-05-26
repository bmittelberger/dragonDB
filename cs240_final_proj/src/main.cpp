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

dragon_db *db;

void* print_stuff(void* args) {
    printf("ID: %lu, CPU: %d\n", pthread_self(), sched_getcpu());
    
    
    for (int i = 0; i < 10001 ; i++) {
        string key(to_string(i));
        string value(to_string(i));
        db->db_put(key,value);
    }
    db->flush();

    for (int i = 0; i < 25 ; i++) {
        string key(to_string(i));
        string val = db->db_get(key);
        //cout << "CPU: " << sched_getcpu() ;
        //cout << " -- for string " << key << ", got value " << val << endl;
        if (key != val) {
            cout << "ERROR -- MISMATCH ON KEY/VAL PAIR" << endl;
            exit(1);
        }
    }
    
    //db->flush();


    return 0;
}



void init(int argc, const char *argv[]) {
    int nc = 3;
    if (argc > 1) {
	nc = atoi(argv[1]);
    } 
    const int num_cores = nc;    

    long max_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores > max_cores) {
	cout << "Too many cores specified. Run again with " << max_cores << "  cores\n";
    }
    
    //Create threads for each core
    pthread_t threads[num_cores];
    
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);


    for (int i = 0; i < num_cores; i++) {
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
        pthread_create(&threads[i], &attr, print_stuff, NULL);
    }

    for (int i = 0; i < num_cores; i++) {
	pthread_join(threads[i], NULL);
    }
}




int main(int argc, const char * argv[]) {
    // insert code here...
    db = new dragon_db("no_file.txt",3);
    init(argc, argv);

 





    return 0;
}
    
