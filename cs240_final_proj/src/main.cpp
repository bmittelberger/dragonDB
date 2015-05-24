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
    return 0;
}






int main(int argc, const char * argv[]) {
    // INITIALIZATION 
    int nc = 2;
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
    int cores_used[num_cores];
    pthread_attr_t attr;
    cpu_set_t cpus;
    pthread_attr_init(&attr);

    //While threads being initialized, their cores are set to busy
    for (int i = 0; i < num_cores; i++) {
        CPU_ZERO(&cpus);
        CPU_SET(i, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	cores_used[i] = 1;
        pthread_create(&threads[i], &attr, print_stuff, NULL);
    }

    for (int i = 0; i < num_cores; i++) {
	pthread_join(threads[i], NULL);
    }

    //Cores not in use any more
    for (int i = 0; i < num_cores; i++) {
	cores_used[i] = 0;
    }

    // START READING REQUESTS TO THE DB
    

    db = new dragon_db("no_file.txt",1);
    db->db_put("hello","goodbye");
    string s = db->db_get("hello");
    cout << "got " << s << " back as a result! " << endl;





    return 0;
}
    
