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
 * Represents a logical in-memory core for our key-value store. Each
 * of "n" cores owns a dragon_segment object, which is a partitioning
 * our hashmap into n
 */

/* Constructs the dragon_core class, which takes in a filename
 * for the persistent db, or creates the file if it doesn't exist.
 * Upon initialization, it creates a new dragon_segment that it owns,
 * there is a 1-1 correspondence between segments and cores. Each core
 * also has it's own set of "mailboxes" where the data destined for this
 * core as per the global hash will be queued up. There are n mailboxes, where
 * n is the number of cores. Each mailbox is a queue of packages (i.e. key-value
 * pairs) that corresponds to a single segment. This mailbox structure, in effect,
 * allows a core to queue data packages destinated for segments belonging to
 * another core.
 */
dragon_core::dragon_core(string filename){
    
}