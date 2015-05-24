//
//  dragon_core.cpp
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "../include/dragonDB.h"
#include <functional>


using namespace std;

/* Default variable values. */
const bool CONSISTENCY_MODEL = false;		/* true -> strong, false -> eventual */
const int MAILBOX_RATE = 500;						/* Mailbox checking frequency, in ms */

/* Constructor: dragon_core
 * 
 * Constructs the dragon_core class, which takes in a filename
 * for the persistent db, or creates the file if it doesn't exist.
 * This function also creates a new dragon_segment owned by this core.
 * 
 * @param filename name of the backup store on disk
 * @param num_cores total number of cores
 */
dragon_core::dragon_core(string filename, int num_cores, int core_id, dragon_db *db) {
		
    /* Instantiate private variables. */
    num_entries = 0;
    disk_flush_last_done = time(0);
    mailbox_last_checked = time(0);
    this->num_cores = num_cores;
    this->core_id = core_id;
    this->db = db;
    //construct mailbox

    /* Set default values for rate, consistency model. */
    consistent = CONSISTENCY_MODEL;
		mailbox_rate = MAILBOX_RATE;
    
    /* Initialize the mailboxes. */
    for(int i = 0; i < num_cores; i++ ) {
        slot s; // TODO: should this be declared on the heap?
        s.packages = new queue<package>;
        pthread_mutex_init(&s.mailbox_lock,NULL);
        mailbox.push_back(s);
    }
}

/* First attempts to put it into our own segment if it hashes to us, 
 * otherwise, we package it and send it to another thread's mailbox 
 *
 * @param key as the string key for the k/v
 * @param value as the string value for the k/v
 */
void dragon_core::put(string key, string value) {
    int dest_core_id = find_core(key);
    package p;
    p.contents = pair<string,string>(key,value);
    p.timestamp = time(0);
    
    if (dest_core_id == this->core_id) { //if the segment is our own, yay! just write to segment
        
        dragon_segment* segment = db->get_segment(dest_core_id);
        if (!segment) cout << "segment is null" << endl;
        segment->put(p);
    } else { //create a package, and send it off to another core to be put in its mailbox
        dragon_core* dest_core = db->get_core(dest_core_id);
        if (!dest_core) cout << "core is null" << endl;
        dest_core->deliver_package(this->core_id,p);
    }
    
    if (time(0) - mailbox_last_checked > mailbox_rate) {
        //flush_mailbox();
    }
}


void dragon_core::flush_mailbox() {
    dragon_segment* segment = db->get_segment(core_id);
    for (int i = 0; i < mailbox.size(); i++) {
        if (i == core_id) {
            continue;
        }
        pthread_mutex_t lock = mailbox[i].mailbox_lock;
        pthread_mutex_lock(&lock);
        queue<package> *packages = mailbox[i].packages;
        while (packages->size() > 0) {
            package p = packages->front();
            packages->pop();
            segment->put(p);
        }
        pthread_mutex_unlock(&lock);
    }
    mailbox_last_checked = time(0);
}

/* Go to another core and deliver the package to its mailbox. Must 
 * synchronize access with a mutex.
 *
 * @param slot_num the number for the slot to deliver to
 * @param pacakge the package itself to add
 */
void dragon_core::deliver_package(int slot_num, package &package) {
    pthread_mutex_t lock = mailbox[slot_num].mailbox_lock;
    pthread_mutex_lock(&lock);
    mailbox[slot_num].packages->push(package);
    pthread_mutex_unlock(&lock);
}

string dragon_core::get(string key) {
    int dest_core = find_core(key);
    dragon_segment* segment = db->get_segment(dest_core);
    segment_entry* entry = segment->get(key);
    if (!entry) {
        return "";
    } else {
        return entry->value;
    }
}

void dragon_core::set_flush_rate(uint64_t rate) {
    mailbox_rate = rate;
}

void dragon_core::set_consistency(bool is_strong) {
    consistent = is_strong;
}

/* Utilizes the c++ hash type to hash the string and find the correct
 * core for this inputed key.
 *
 * @param key for finding the correct core
*/
int dragon_core::find_core(string key) {
    return hasher(key)%num_cores;
    //TODO assuming thread_ids range from 0 to n-1 -- must make sure
}



