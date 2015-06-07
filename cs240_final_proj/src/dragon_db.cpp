//
//  dragon_db.cc
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "../include/dragonDB.h"

using namespace std;


const bool CONSISTENCY_MODEL = false;       /* true -> strong, false -> eventual */
const uint64_t DISK_FLUSH_RATE = 1000;
    
/* Opens up a new instantiation of the dragonDB main structures.
 * The filename passed in is in reference to a persisted db that
 * stored somewhere on disk. If it doesn't already exist on disk,
 * we initialize a new store with that name. 
 *
 * @param filename name for the persistence file
 */
dragon_db::dragon_db(string filename,int num_cores) {
    
    consistent = CONSISTENCY_MODEL; //default
    this->num_cores = num_cores;

    for(int i = 0; i < this->num_cores; i++) {
        dragon_core *core = new dragon_core(filename, num_cores, i, this);
        dragon_segment *segment = new dragon_segment(filename, i);
        segment->load_from_disk();
        map_cores[i] = core;
        map_segments[i] = segment;

    }
    disk_flush_rate = DISK_FLUSH_RATE;

}


dragon_db::~dragon_db() {
    for (int i = 0; i < this->num_cores; i++) {
        delete map_cores[i];
        delete map_segments[i];
    }
}


/* Abstracted put function for the database, each application will interact
 * with this exposed function, adding its own key and value. Returns true
 * on success and false on failure
 *
 * @param key string as key for the k/v store
 * @param value string as value for the k/v store
 */
void dragon_db::db_put(string key, string value) {
    if (value == ""  || key == "") {
        return;
    }
    dragon_core *core = map_cores[sched_getcpu()];
    if (core) {
        core->put(key,value);
    } else {
        cout << "Core " << sched_getcpu() << " was null in put" << endl;
    }
}

/* Abstracted put function for the database, each application will interact
 * with this exposed function, adding its own key and value. Returns the value
 * if it is in the store, or the empty string if it is not found 
 *
 * @param key string as key for the k/v store
 */
string dragon_db::db_get(string key) {
    dragon_core *core = map_cores[sched_getcpu()];
    if (core){
        string ret = core->get(key);
        return ret;
    } else {
        cout << "Core " << sched_getcpu() << " was null in get" << endl;
    }
}

void dragon_db::flush() {
    dragon_core *core = map_cores[sched_getcpu()];
    dragon_segment *segment = map_segments[sched_getcpu()];
    core->flush_mailbox();
    segment->flush_to_disk();
}


dragon_segment* dragon_db::get_segment(int core_id) {
    return map_segments[core_id];
}
    
dragon_core* dragon_db::get_core(int core_id) {
    return map_cores[core_id];
}


/* Closes the dragon db, and flushes all buffered stores to disk. Also
 * frees all memory used by the library. Called when user no longer 
 * needs to use the db. 
 */
void dragon_db::close() {
    this->flush();
    delete this;
}

uint64_t dragon_db::get_time(){
    return std::chrono::system_clock::now().time_since_epoch() / 
            std::chrono::milliseconds(1);
}

/* Sets the consistency value for the database. If true, then all puts
 * and gets on any core will be immediately available to any other thread
 * that also has access to the db. If false, then we guarantee eventual
 * consistency, but not immediate. False will scale better because it doesn't
 * require serialized consistency.
 */
void dragon_db::set_consistency(bool is_strong) {
    consistent = is_strong;
}

/* Returns the consistency value for the database - true if strongly
 * consistent, and false if eventual consistency is allowed.
 */
bool dragon_db::is_strongly_consistent() {
    return consistent;
}

