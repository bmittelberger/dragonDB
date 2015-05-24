//
//  dragonDB.h
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//


#include <map>
#include <vector>
#define _GNU_SOURCE
#include <sched.h>

#include "mach/thread_policy.h"

#include <thread>

#include <stdlib.h>
#include <queue>
#include <string>
#include <iostream>
#include <stdio.h>


using namespace std;
    
#ifndef CS240_Final_Project_dragonDB_h
#define CS240_Final_Project_dragonDB_h
    
class dragon_segment {
private:
    map<string, string> segment;
    int version_number;
    mutex segment_lock; //used if consistency is required
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
struct transfer_data {
    pair<string,string> key_val; //actual k/v data
    uint64_t timestamp;
};

class dragon_core {
private:
    map<int, dragon_segment*>  map_segments; //hashing out to other segments on diff cores
    int core_id;
    int num_cores;
    int num_entries;
    uint64_t disk_flush_timer; //indicates when we flush to disk
    uint64_t mailbox_timer; //indicates when we should check mailboxes
    uint64_t mailbox_rate; //interval between mailbox checking
    bool consistent;
    vector<queue<transfer_data> >data_qs;
    
    int find_core(string key); //hashes incoming key to find which segment it should go to
    
public:
    dragon_core* init_core();
    dragon_core* load(string filename);
    int put(string key, string value);
    string get(string key);
    void set_consistency(bool on);
    
};


class dragon_db {
private:
    vector<dragon_core* > cores;
    int num_cores;
            bool consistent;
    
public:
    
    uint64_t disk_flush_rate;
    
    //load persisted key/val store from disk
    //if filename is empty, init a new k/v store
    dragon_db* db_open(string filename);
    int db_put(string key, string value);
    string db_get(string key);
    void close();
    
    void set_consistency(bool on);
};
    

#endif
