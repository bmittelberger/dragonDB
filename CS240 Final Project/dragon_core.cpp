//
//  dragon_core.cpp
//  CS240 Final Project
//
//  Created by Ben Mittelberger on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "dragonDB.h"

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
dragon_core::dragon_core(string filename){
    
}
