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
#include <fstream>
#include <cstring>
#include <sstream>
#include <cstdlib>

#define GET 1
#define PUT 2
#define CLOSE 3

using namespace std;

string store_name;
dragon_db *db;

struct keyval {
    string key;
    string value;
};


void* print_stuff(void* args) {    
    

    int max = 100;
    for (int i = 0; i < max ; i++) {
        string key(to_string(i));
        string value(to_string(i));
        db->db_put(key,value);
    }

    db->flush();

    for (int i = 0; i < max ; i++) {
        string key(to_string(i));
        string val = db->db_get(key);
        // cout << "CPU: " << sched_getcpu() ;
        // cout << " -- for string " << key << ", got value " << val << endl;
        if (key != val) {
            cout << "ERROR -- MISMATCH ON KEY/VAL PAIR" << endl;
            exit(1);
        }
    }

}

void* put(void* args) {
    keyval *kv;
    kv = (keyval *)args;
    db->db_put(kv->key, kv->value);
    //db->flush();
}


void *get(void* args) {
    keyval *kv;
    kv = (keyval *)args;
    string val = db->db_get(kv->key);
    string malformed("");
    if (val.compare(malformed) == 0) {
        cout << "KEY DOES NOT EXIST\n";
    } else {
        cout << kv->key << ": " << val << endl;
    }
}


void *close(void *args) {
    db->close();
}


int set_thread(string key, string value, pthread_t threads[], 
    int cores_used[], const int num_cores, int flag) {

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    cpu_set_t cpus;

    CPU_ZERO(&cpus);
    //pick random CPU until you hit one that's not busy
    int rand_cpu = rand() % num_cores;
    while (cores_used[rand_cpu]) {
        rand_cpu = rand() % num_cores;
    }
    CPU_SET(rand_cpu, &cpus);
    cores_used[rand_cpu] = 1;
    
    keyval *kv = new keyval();
    kv->key = key;
    kv->value = value;

    pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
    if (flag == GET) {
        pthread_create(&threads[rand_cpu], &attr, get,  (void *)kv);
    } else if (flag == PUT) {
        pthread_create(&threads[rand_cpu], &attr, put,  (void *)kv);
    } else if (flag == CLOSE) {
        pthread_create(&threads[rand_cpu], &attr, close, NULL);
    }

    return rand_cpu;
}


void process_lines(pthread_t threads[], 
    int cores_used[], const int num_cores, string line) {

    string put ("puts");
    string get ("get");
    string close ("close");

    string command, key, value;
    
    istringstream iss(line);
    iss >> command;
    int tu;

    if (command.compare(put) == 0) {
        iss >> key; 
        iss >> value;
        tu = set_thread(key, value, threads, cores_used, num_cores, PUT);
    } else if (command.compare(get) == 0) {
        iss >> key;
        value = "";
        tu = set_thread(key, value, threads, cores_used, num_cores, GET);
    } else if (command.compare(close) == 0) {
        tu = set_thread(key, value, threads, cores_used, num_cores, CLOSE);
    } else {
        cout << "Invalid command\n";
        tu = -1;
    }

    if (tu != -1 ) {
        cores_used[tu] = 0;
    }
}

void read_file(string line, pthread_t threads[], 
    int cores_used[], const int num_cores) {
    string open("open");
    string command;
    istringstream iss(line);
    iss >> command;
    int tu;

    if (command.compare(open) == 0) {
        iss >> command;
        store_name = command;
        db = new dragon_db(command.c_str(), num_cores);
    } else {
        process_lines(threads, cores_used, num_cores, line);
    }
}


void read_commands(string filename, pthread_t threads[], 
    int cores_used[], const int num_cores) {
    //If not file was specified, read from stdin
    string line; 

    if (filename == "") {
        cout << "\nYour first command to interact with any key-value store will \n" <<
                "need to be an open of a specifically-named store, i.e. open \n" <<
                "new-store\n";
        while (getline(cin, line)) {
            read_file(line, threads, cores_used, num_cores);
        }

        for (int i=0; i< num_cores; i++) {
           pthread_join(threads[i], NULL);
        }

    } else {
        ifstream myfile(filename);
        if (myfile) {
            while (getline( myfile, line )) {
                read_file(line, threads, cores_used, num_cores);
             }
            
            for (int i=0; i< num_cores; i++) {
               pthread_join(threads[i], NULL);
            }

            myfile.close();
        } else {
            cout << "You need to specify a correct dragonDB key-value store\n";
        }

    }
}


int main(int argc, const char * argv[]) {
    // INITIALIZATION 
    int nc = 4;
    string filename = "";

    if (argc > 1) {
	   nc = atoi(argv[1]);
    } 
    if (argc > 2) {
        filename = argv[2];
    }

    const int num_cores = nc;    
    
    long max_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_cores > max_cores) {
    	cout << "Too many cores specified. Run again with <= " << max_cores << " cores\n";
    	exit(0);
    }
    
    db = new dragon_db("no_file", num_cores);
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

    //Read commands
    //read_commands(filename, threads, cores_used, num_cores);

    return 0;
}
    
