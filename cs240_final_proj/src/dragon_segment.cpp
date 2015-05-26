//
//  dragon_segment.cpp
//  CS240 Final Project
//
//  Created by Suzanne Stathatos on 5/23/15.
//  Copyright (c) 2015 cs240. All rights reserved.
//

#include <stdio.h>
#include "../include/dragonDB.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

using namespace std;
    

dragon_segment::dragon_segment(string filename, int core_id) {
    version_number = 0;
    this->core_id = core_id;
    pthread_mutex_init(&segment_lock,NULL);
    this->filename = filename;
};

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
int dragon_segment::flush_to_disk() {
    string outfile = filename + "-" + to_string(core_id) + ".drg";
    uint64_t segment_size = 0; //start at 1 for newline after size write

    cout << "flushing to file for core: " << sched_getcpu() << endl;
    ofstream fs(outfile.c_str(), ofstream::app);


    //THIS IS REALLY SHITTY -- FIND A WAY TO DO IT IN A SINGLE PASS
    vector<string> out;
    map<string, segment_entry*>::iterator it;
    string output;
    for (it = store.begin(); it != store.end(); it++){
        output = it->first + "," + it->second->value + "," + to_string(it->second->timestamp) + "\n";
        segment_size += it->first.length();
        segment_size += it->second->value.length();
        segment_size += (to_string(it->second->timestamp)).length();
        segment_size += 3; // for commas and endl
        out.push_back(output);
    }
    fs << segment_size;
    fs << "\n" ;
    
    //Write checksum here

    for (int i = 0; i < out.size(); i++){
        fs << out[i];
    }
    version_number ++;
};





/* Iterate through all of the segements until we find the most
 * recent complete one. Complete means that the checksum matches
 * a newly computed checksum of the data in the file. If it doesn't
 * match, we roll back a segment and attempt to checksum there
 */
int dragon_segment::load_from_disk() {
    string infile = filename + "-" + to_string(core_id) + ".drg";
    ifstream is (infile, ios::in);
    cout << "reading from file " << infile << endl;
    
    //if file doesn't exist, return
    if (!is.good()){
        return -1;
    }

    //get the size of the file in its entirety
    is.seekg(0,iostream::end);
    int filesize = is.tellg();
    is.seekg(0,iostream::beg);
    if (filesize == 0) return -1;


    int prev_segment_offset = 0;
    string line;
    while (true) {
        getline(is,line);
        cout << line << endl;
        int segment_size = stoi(line);
        int cur_offset = is.tellg();
        if (cur_offset + segment_size >= filesize) break;
        is.seekg(segment_size, iostream::cur);
        prev_segment_offset = cur_offset;
    }

    //iterate through all lines of the segment
    while (getline(is,line)) {

        string workline = line;
        //tokenize string by commas
        auto find = workline.find(",");
        string key = workline.substr(0,find);
        workline = workline.substr(find+1,line.length());
        find = workline.find(",");
        string value = workline.substr(0,find);
        string timestamp_str = workline.substr(find+1,line.length());
        uint64_t timestamp = stoull(timestamp_str);

        //Build the segment        
        segment_entry *entry = new segment_entry;
        entry->value = value;
        entry->timestamp = timestamp;

        // cout << "key:" << key ;
        // cout << " value: " << value ;
        // cout << " timestamp: " << timestamp << endl;

        store[key] = entry;

    }


};



void lock_segment() {
    
};

void unlock_segment() {
    
};

    
    
