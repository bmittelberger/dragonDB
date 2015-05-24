//
//  dragon_segment.cpp
//  CS240 Final Project
//
//  Created by Suzanne Stathatos on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "../include/dragonDB.h"

using namespace std;
    
/* Receives a package to put into the segment store. If it exists,
 * first checks timestamp to make sure we want to replace it with
 * the inputted value. If we want to replace (meaning the inputted timestamp
 * is more recent (greater)), we insert the new value and free the 
 * memory of the old entry
 *
 * @param p package struct that contains info for timestamp and key/value
 */
void dragon_segment::put(package& p) {
    segment_entry *old_entry = get(p.contents.first);
    if (old_entry && old_entry->timestamp > p.timestamp) {
        return;
    }
    
    segment_entry* entry = new segment_entry;
    entry->value = p.contents.second;
    entry->timestamp = p.timestamp;
    store[p.contents.first] = entry;
    
    if (old_entry) {
        delete old_entry;
    }
};

/* Gets a segment entry out of the segment's store. Uses no locks,
 * but instead checks the timestamp on the entry twice to see if
 * anything has chagned between accesses.
 *
 * @param key string for the value
 */
segment_entry* dragon_segment::get(string key) {
    if (store.count(key) == 0) {
        return NULL;
    }
    segment_entry *entry1 = store[key];
    while (true) {
        segment_entry *entry2 = store[key];
        if (entry1->timestamp == entry2->timestamp) {
            break; //nothing changed between two accesses -- we're good
        }
        entry1 = entry2; //store entry2 so we can make the check again
    }
    return entry1;
};

//Each core flushes to a unique file corresponding to
//that core/segment
int flush_to_disk() {
    
    return 0;
};

int load_from_disk() {
    return 0;
};

void lock_segment() {
    
};

void unlock_segment() {
    
};

    
    
