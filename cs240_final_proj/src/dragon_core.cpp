//
//  dragon_core.cpp
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "dragonDB.h"
#include <functional>


using namespace std;

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

/* Constructs the dragon_core class, which takes in a filename
 * for the persistent db, or creates the file if it doesn't exist.
 * This function also creates a new dragon_segment owned by this core.
 */
dragon_core::dragon_core(string filename) {
    //TODO: IMPLEMENT THIS FN
}


/* First attempts to put it into our own segment if it hashes to us, 
 * otherwise, we package it and send it to another thread's mailbox */
bool dragon_core::put(string key, string value) {
    int correct_core = find_core(key);
    if (correct_core == this->core_id){ //if the segment is our own, yay! just write to segment
        dragon_segment* segment = db->get_segment(correct_core);
        return segment->put(key,value);
    } else { //create a package, and send it off to another core to be put in its mailbox
        pair<string,string> package_contents(key,value);
        package new_package;
        new_package.timestamp = time(0);
        new_package.contents = package_contents;
        dragon_core* dest_core = db->get_core(correct_core);
        dest_core->deliver_package(this->core_id,new_package);
    }
    //TODO: figure out flushing our own mailbox
    return true;
}


void dragon_core::deliver_package(int slot_num, package &package){
    mailbox[slot_num].packages.push(package);
}

string dragon_core::get(string key){
    return "";
}

void dragon_core::set_consistency(bool on) {
    consistent = on;
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



