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

const uint64_t DISK_FLUSH_RATE = 1000;
    
/* Opens up a new instantiation of the dragonDB main structures.
 * The filename passed in is in reference to a persisted db that
 * stored somewhere on disk. If it doesn't already exist on disk,
 * we initialize a new store with that name. 
 *
 * @param filename name for the persistence file
 */
dragon_db::dragon_db(string filename,int num_cores) {
    
    consistent = false; //default
    this->num_cores = num_cores;

    for(int i = 0; i < this->num_cores; i++) {
        dragon_core *core = new dragon_core(filename,num_cores,core_id);
        dragon_segment *segment = new dragon_segment(core_id);
        map_cores.insert(i,core);
        map_segments.insert(i,segment);
    }
    disk_flush_rate = DISK_FLUSH_RATE;



    //TODO: IMPLEMENT THIS FN
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
    //TODO: IMPLEMENT THIS FN
}

/* Abstracted put function for the database, each application will interact
 * with this exposed function, adding its own key and value. Returns the value
 * if it is in the store, or the empty string if it is not found 
 *
 * @param key string as key for the k/v store
 */
string dragon_db::db_get(string key) {
    
    //TODO: IMPLEMENT THIS FN
    return "";
}


/* Closes the dragon db, and flushes all buffered stores to disk. Also
 * frees all memory used by the library. Called when user no longer 
 * needs to use the db. 
 */
void dragon_db::close() {
    
    //TODO: IMPLEMENT THIS FN
}

/* Sets the consistency value for the database. If true, then all puts
 * and gets on any core will be immediately available to any other thread
 * that also has access to the db. If false, then we guarantee eventual
 * consistency, but not immediate. False will scale better because it doesn't
 * require serialized consistency.
 */
void set_consistency(bool on) {
    //TODO: IMPLEMENT THIS FN
}












