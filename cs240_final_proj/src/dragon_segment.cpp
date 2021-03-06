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
#include <cstdlib>
#include <sys/stat.h>

using namespace std;
    

dragon_segment::dragon_segment(string filename, int core_id) {
    version_number = 0;
    this->core_id = core_id;
    pthread_mutex_init(&segment_lock,NULL);
    this->filename = filename;
};

dragon_segment::~dragon_segment() {
    pthread_mutex_destroy(&segment_lock);
}


/* Receives a package to put into the segment store. If it exists,
 * first checks timestamp to make sure we want to replace it with
 * the inputted value. If we want to replace (meaning the inputted timestamp
 * is more recent (greater)), we insert the new value and free the 
 * memory of the old entry
 *
 * @param p package struct that contains info for timestamp and key/value
 */
void dragon_segment::put(package& p) {

    pthread_mutex_lock(&segment_lock);

    segment_entry *old_entry = get(p.contents.first);

    if (old_entry && old_entry->timestamp > p.timestamp) {
        pthread_mutex_unlock(&segment_lock);
        return;
    }
    
    segment_entry* entry = new segment_entry;
    entry->value = p.contents.second;
    entry->timestamp = p.timestamp;

    store[p.contents.first] = entry;
    if (old_entry) {
        delete old_entry;
    }

    pthread_mutex_unlock(&segment_lock);
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
    struct stat sb;

    //If a directory doesn't already exist for the dragon store, make one
    /*if (stat(filename.c_str(), &sb) != 0 && !S_ISDIR(sb.st_mode)) {
        string command = "mkdir " + filename;
        system(command.c_str());
    }*/
    string outfile = filename + "-" + to_string(core_id) + ".drg";

    pthread_mutex_lock(&segment_lock);

    ofstream fs(outfile.c_str(), ofstream::app);
    write_segment(fs);
    int fd = open (outfile.c_str(),O_WRONLY);
    fsync(fd);

    pthread_mutex_unlock(&segment_lock);
};


/* Takes in the file stream to write to and writes out the entirety
 * of the core segment's memory into a segment on the disk. It creates
 * a segment size line as a pointer to the next segment in the file, and
 * it checksums the data to ensure durability 
 *
 * NOTE: this function assumes that segment_lock has already been obtained,
 * as this entire function sits in the critical section.
 *
 * @param fs filestream pointing to the file where we want to write the segment
 */
void dragon_segment::write_segment(ofstream &fs) {
    vector<string> out;
    map<string, segment_entry*>::iterator it;
    string output;

    size_t cksum = 0;
    int segment_size = 0;

    for (it = store.begin(); it != store.end(); it++){
        output = it->first + "," + it->second->value + "," + to_string(it->second->timestamp);
        segment_size += it->first.length();
        segment_size += it->second->value.length();
        segment_size += (to_string(it->second->timestamp)).length();
        segment_size += 3; // for commas and endl
        out.push_back(output + "\n");
        cksum ^= hash_str(output);
    }
    
    segment_size += to_string(cksum).length() + 1; //+1 is for newline
    fs << segment_size;
    fs << "\n" ;

    // Write checksum here
    fs << to_string(cksum);
    fs << "\n";

    for (int i = 0; i < out.size(); i++){
        fs << out[i];
    }
    fs.flush();
}




/* Iterate through all of the segements until we find the most
 * recent complete one. Complete means that the checksum matches
 * a newly computed checksum of the data in the file. If it doesn't
 * match, we roll back a segment and check the validity. We continue
 * to roll back until we find a complete segment that we can load.
 */
int dragon_segment::load_from_disk() {
    string infile = filename + "-" + to_string(core_id) + ".drg";

    ifstream is (infile, ios::in);
    cout << "reading from file " << infile << "..." << endl;
    
    //if file doesn't exist, return
    if (!is.good()){
        //cout << "nothing to read" << endl;
        return -1;
    }

    //get the size of the file in its entirety
    is.seekg(0,iostream::end);
    int filesize = is.tellg();
    is.seekg(0,iostream::beg);
    if (filesize == 0) {
        cout << "nothing to read" << endl;
        return -1;
    }

    int prev_segment_offset = 0;
    string line;
    int cur_offset = 0;
    while (true) {
        getline(is,line);
        int segment_size = stoi(line);
        cur_offset = is.tellg();
        if (cur_offset + segment_size >= filesize) break;
        is.seekg(segment_size, iostream::cur);
        prev_segment_offset = cur_offset;
    }

    /* Build segment in memory and compute checksum. */
    while (true) {
        is.clear();
        is.seekg(cur_offset, iostream::beg);

        string cksum_line;
        getline(is, cksum_line);

        size_t disk_cksum = stoull(cksum_line);
        size_t cksum = 0;

        //iterate through all lines of the segment
        while (getline(is,line)) {

            string workline = line;
            //tokenize string by commas
            auto find = workline.find(",");
            if (find == string::npos) {
                break;
            }
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
            store[key] = entry;
            cksum ^= hash_str(line);
        }

        // Ensure that the current segment hasn't been corrupted
        if (disk_cksum != cksum){
            if (cur_offset == prev_segment_offset || prev_segment_offset == 0) {
                cout << "ERROR: FILE [" << core_id << "] IS CORRUPTED" << endl;
                exit(1);
            } else {
                store.clear();  
                cur_offset = prev_segment_offset;
            }
        } else {
            break;
        }


    }
    clean_file(infile);

    cout << "Done loading " << infile << endl;
    /* TODO: figure out what to do if cksum != disk_cksum. */

    /* TODO: why aren't we returning an int here? */
};


/* This function takes the most recent full segment
 * and writes it out to a fresh file without the rest 
 * of the dead entries in the previous log */
void dragon_segment::clean_file(string filename) {

    pthread_mutex_lock(&segment_lock);

    string tmpfile = filename + ".tmp";
    ofstream fs(tmpfile.c_str(), ofstream::app);
    write_segment(fs);
    string command = "mv " + filename + " " + filename + ".tmp2";
    system(command.c_str());
    command = "mv " + tmpfile + " " + filename;
    system(command.c_str());
    command = "rm -rf " + filename + ".tmp2";
    system(command.c_str());

    pthread_mutex_unlock(&segment_lock);
}

size_t dragon_segment::hash_str(string input) {
    /* Start with some primes. */
    size_t hash = 937;
    size_t p1 = 37;
    size_t p2 = 79;
    for (int i = 0; i < input.size(); i++) {
        hash = hash * p1 ^ ((int) input[i] * p2);
    }
    return hash;
}

void lock_segment() {
    
};

void unlock_segment() {
    
};

    
    
