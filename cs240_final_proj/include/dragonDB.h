//
//  dragonDB.h
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//


#include <map>
#include <vector>
#include <sched.h>
#include <pthread.h>
#include <thread>

#include <stdlib.h>
#include <queue>
#include <string>
#include <iostream>
#include <stdio.h>

#include <time.h>

using namespace std;
    
#ifndef CS240_Final_Project_dragonDB_h
#define CS240_Final_Project_dragonDB_h


class dragon_db;
class dragon_core;
class dragon_segment;
    
struct segment_entry{
    string value;
    uint64_t timestamp;
};
    
class dragon_segment {
private:
    map<string, segment_entry> segment;
    int version_number;
    pthread_mutex_t segment_lock; //used if consistency is required
    string filename; //The file that the segment will write to
    
public:
    dragon_segment() {
        version_number = 0;
    
        //Can only flush segments that correspond to our core
        //int cpu = sched_getcpu();
        
        
    }
    
    int put(string key, string value); //For HM
    string get(string key); //For HM
    int flush_to_disk(); //Flushing seg to FS
    int load_from_disk(); //For loading from FS
    void lock_segment();
    void unlock_segment();
};

//the data put into another core's queue
//if not hashed to own segment
struct package {
    pair<string,string> contents; //actual k/v data
    uint64_t timestamp;
};

// struct for each core to place info in
// another core's mailbox -- lock is only
// used if strict consistency is turned on
struct slot {
    queue< package > packages;
    pthread_mutex_t mailbox_lock;
};

class dragon_core {
private:
    
    dragon_db* db;
    int core_id;
    int num_cores;
    int num_entries;
    
    uint64_t disk_flush_last_done; //indicates when we last flushed to disk
    uint64_t mailbox_last_checked; //indicates when we last checked mailboxes
    uint64_t mailbox_rate; //interval between mailbox checking
    bool consistent;
    vector<slot> mailbox;
    
    hash<string> hasher;
    int find_core(string key); //hashes incoming key to find which segment it should go to
    
public:
    dragon_core(string filename);
    bool put(string key, string value);
    string get(string key);
    void set_consistency(bool on);
    void send_package(package p);
    void deliver_package(int slot_num, package& package);
    
};


class dragon_db {
private:
    vector<dragon_core* > cores;
    int num_cores;
    bool consistent;
    map<int, dragon_segment*>  map_segments; //hashing out to other segments on diff cores
    map<int, dragon_core*> map_cores;
    
public:
    
    uint64_t disk_flush_rate;
    
    dragon_segment* get_segment(int core_id);
    dragon_core* get_core(int core_id);
    
    //load persisted key/val store from disk
    //if filename is empty, init a new k/v store
    dragon_db* db_open(string filename);
    bool db_put(string key, string value);
    string db_get(string key);
    void close();
    
    void set_consistency(bool on);
};
    

#endif
