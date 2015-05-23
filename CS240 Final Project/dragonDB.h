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
#include <queue>
#include <string>
#include <iostream>

namespace std{
    
#ifndef CS240_Final_Project_dragonDB_h
#define CS240_Final_Project_dragonDB_h
    
class dragon_segment {
private:
    map<string, string> segment;
    int version_number;
    mutex segment_lock; //used if consistency is required
    
public:
    int put(string key, string value);
    string get(string key);
    int flush_to_disk();
    void lock_segment();
    void unlock_segment();
};

//the data put into another core's queue
//if not hashed to own segment
struct package {
    pair<string,string> key_val; //actual k/v data
    uint64_t timestamp;
};

class dragon_core {
private:
    map<int, dragon_segment*>  map_segments; //hashing out to other segments on diff cores
    int core_id;
    int num_cores;
    int num_entries;
    uint64_t disk_flush_last_done; //indicates when we last flushed to disk
    uint64_t mailbox_last_checked; //indicates when we last checked mailboxes
    uint64_t mailbox_rate; //interval between mailbox checking
    bool consistent;
    vector< queue<transfer_data> > mailboxes;
    
    int find_core(string key); //hashes incoming key to find which segment it should go to
    
public:
    dragon_core(string filename);
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
    bool db_put(string key, string value);
    string db_get(string key);
    void close();
    
    void set_consistency(bool on);
};
    
    
} //end of std namespace

#endif
