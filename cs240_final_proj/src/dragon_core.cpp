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
const int MAILBOX_RATE = 500;               /* Mailbox checking frequency, in ms */

/* Constructor: dragon_core
 * 
 * Constructs the dragon_core class, which takes in a filename
 * for the persistent db, or creates the file if it doesn't exist.
 * This function also creates a new dragon_segment owned by this core.
 * 
 * @param filename name of the backup store on disk
 * @param num_cores total number of cores
 * @param core_id the core_id identifying the core
 * @param db a reference to the database object
 */
dragon_core::dragon_core(string filename, int num_cores, int core_id, dragon_db *db) {

    /* Instantiate private variables. */
    num_entries = 0;
    disk_flush_last_done = db->get_time();
    mailbox_last_checked = db->get_time();
    this->num_cores = num_cores;
    this->core_id = core_id;
    this->db = db;

    /* Set default value for mailbox rate. */
    mailbox_rate = MAILBOX_RATE;
    
    /* Initialize the mailboxes. */
    for (int i = 0; i < num_cores; i++ ) {
        slot s; // TODO: should this be declared on the heap?
        s.packages = new queue<package>;
        pthread_mutex_init(&s.mailbox_lock,NULL);
        mailbox.push_back(s);
    }
}

/* Destructor: ~dragon_core
 * 
 * Frees resources associated with the core. 
 */
dragon_core::~dragon_core() {
    for (int i = 0; i < num_cores; i++) {
        delete mailbox[i].packages;
    }
}

/* Function: dragon_core::put
 *
 * A thread-specific "put" handler called by dragon_db on a particular
 * dragon_core. This function first hashes the key to determine if the
 * K/V pair can be processed on this core. If so, the put is performed
 * immediately - otherwise, this function will package the K/V pair
 * and send it to another dragon_core to be processed. In the latter
 * case, the pair will then be processed immediately if and only if
 * strong consistency is specified as the model.
 *
 * @param key as the string key for the K/V
 * @param value as the string value for the K/V
 */
void dragon_core::put(string key, string value) {

    /* Create a package out of the K/V pair. */
    package p;
    p.contents = pair<string,string>(key,value);
    p.timestamp = db->get_time();

    /* Determine which core's segment the key belongs on. */
    int dest_core_id = map_to_core(key);
    // cout << "Current thread " << sched_getcpu() << " Dest_core: " << dest_core_id << endl;

    /* If the segment is owned locally, perform the put. */
    if (dest_core_id == this->core_id) { 

        dragon_segment* segment = db->get_segment(dest_core_id);
        if (!segment) cout << "segment is null" << endl;
        segment->put(p);

    /* If the segment is owned by another core, queue the package. */
    } else { 

        dragon_core* dest_core = db->get_core(dest_core_id);
        if (!dest_core) cout << "core is null" << endl;

        dest_core->deliver_package(this->core_id, p);
    }
    
    /* Flush the local core's mailbox periodically. */
    if (db->get_time() - mailbox_last_checked > mailbox_rate) {
        flush_mailbox();
    }
    if (db->get_time() - disk_flush_last_done > db->disk_flush_rate){
        dragon_segment * segment = db->get_segment(core_id);
        segment->flush_to_disk();
    }
}

/* Function: dragon_core::get
 *
 * A thread-specific "get" handler called by dragon_db on a particular
 * dragon_core. This function locates the segment that the key belongs
 * on, and retrieves the corresponding value. The empty string is
 * returned if the entry has not yet been inserted into the store.
 *
 * @param key as the string key for the K/V
 */
string dragon_core::get(string key) {

    /* Determine which core's segment the key belongs on. */
    int dest_core = map_to_core(key);

    // cout << "Current thread " << sched_getcpu() << " Dest_core: " << dest_core << endl;

    /* Check to see if an entry exists for this key. */
    dragon_segment* segment = db->get_segment(dest_core);
    segment_entry* entry = segment->get(key);

    /* Return entry value if exists, or "" otherwise. */
    if (!entry) {
        return "";
    } else {
        return entry->value;
    }
}

/* Function: dragon_core::flush_mailbox
 *
 * Flushes the local core's mailbox (which contains packages inserted
 * by other cores) and processes each package as a put request. This
 * implementation assumes that dragon_segment->put keeps the integrity
 * of the package ordering, as specified as the package's timestamp.
 * That is, if package "A" and "B" both refer to the same key, then
 * the store will persist the value for the package created at a later
 * timestamp, regardless of the order in which A and B are processed.
 */
void dragon_core::flush_mailbox() {
    dragon_segment* segment = db->get_segment(core_id);

    /* Loop through the slots (except our own, which should be empty). */

    for (int i = 0; i < mailbox.size(); i++) {
        if (i == core_id)
            continue;

        pthread_mutex_t lock = mailbox[i].mailbox_lock;
        pthread_mutex_lock(&lock);



        /* Process each package in each slot. */
        queue<package> *packages = mailbox[i].packages;
        while (packages->size() > 0) {
            package p = packages->front();

            packages->pop();
            segment->put(p);
        }

        pthread_mutex_unlock(&lock);
    }

    /* Update the last time at which the mailbox was checked. */
    mailbox_last_checked = db->get_time();
}

/* Function: dragon_core::deliver_package
 * 
 * Delivers a package to another core's mailbox, stored in the slot
 * set aside for the local core. Note that this function is called
 * on the core of the receiving core. 
 *
 * @param slot_num the number for the slot to deliver to
 * @param pacakge the package to add
 */
void dragon_core::deliver_package(int slot_num, package &package) {
    pthread_mutex_t lock = mailbox[slot_num].mailbox_lock;
    pthread_mutex_lock(&lock);
    mailbox[slot_num].packages->push(package);
    pthread_mutex_unlock(&lock);
}

void dragon_core::set_flush_rate(uint64_t rate) {
    mailbox_rate = rate;
}

/* Utilizes the c++ hash type to hash the string and find the correct
 * core for this inputed key. This function assumes that the core
 * ID's are unique, 0-indexed integers that increase incrementally. 
 *
 * @param key for finding the correct core
*/
int dragon_core::map_to_core(string key) {
    return hasher(key) % num_cores;
}
