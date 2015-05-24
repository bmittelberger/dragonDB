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
#include <pthread.h>
#include <sched.h>

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
    
//the data put into another core's queue
//if not hashed to own segment
struct package {
    pair<string,string> contents; //actual k/v data
    uint64_t timestamp;
};
    
class dragon_segment {
private:
    map<string, segment_entry*> store;
    int version_number;
    pthread_mutex_t segment_lock; //used if consistency is required
    string filename; //The file that the segment will write to
    
public:
    dragon_segment() {
        version_number = 0;
        //Can only flush segments that correspond to our core
        //int cpu = sched_getcpu();
    }
    
    void put(package& p);
    segment_entry* get(string key);
    int flush_to_disk();
    void lock_segment();
    void unlock_segment();
};



// struct for each core to place info in
// another core's mailbox -- lock is only
// used if strict consistency is turned on
struct slot {
    queue< package > *packages;
    pthread_mutex_t mailbox_lock;
};

/* Class: dragon_core
 * 
 * Represents a logical in-memory core for our key-value store, which
 * will, at the user-facing level, handle "put" and "get" calls for
 * a given thread.
 *
 * Each of <n> dragon_cores owns a dragon_segment object, which is a
 * partitioning of our hashmap into <n> on-memory buffered segments. There
 * is a one-to-one correspondence between segments and cores.
 *
 * Each core also has its own "mailbox" consisting of  <n> "slots". Each 
 * slot is a queue of "packages" (i.e. KV pairs instantiated by a "put"
 * request) destined for the segment belonging to the owner (determined by
 * the to-core hash). The slots are indexed by core id. Put requests on 
 * other cores that get hashed to dragon_segment will then be queued on
 * the slot corresponding to the core sending the request. The owner core
 * will then periodically check the mailbox to handle all queued requests.
 * This effectively allows a core to queue data packages destined for
 * segments belonging to another core without compromising scalability.
 *
 * Note: the above queueing only applies if the user specifies that
 * eventual consistency is acceptable for our key-value store. If strong
 * consistency is mandated, then incoming "put" requests will simply
 * be processed immediately by the core on which they were called.
 */
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
    void flush_mailbox();
    
public:
    dragon_core(string filename, int num_cores);
    void put(string key, string value);
    string get(string key);
    void set_consistency(bool on);
    void set_flush_rate(uint64_t rate);
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
    void db_put(string key, string value);
    string db_get(string key);
    void close();
    
    void set_consistency(bool on);
};
    

#endif
