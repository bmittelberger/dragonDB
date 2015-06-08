DragonDB
=========

Making a scalable, eventually-consistent, durable, commutative key-value store

Project done with:
  * Ben Mittelberger (https://github.com/bmittelberger)
  * Suzanne Stathatos (https://github.com/suzanne-stathatos)
  * Evan Shieh (https://github.com/eshieh)
  * Claudia Roberts (https://github.com/chicacvr)

##Setup##

Disclaimer: Much of this is from the Lab2 setup instructions. If you've already set up
lab2, you can skip the first 2 steps.

1. Vagrant and the Virtual Machine: 
  All of our database is run in a virtual machine. We needed to operate on a Linux platform to 
  allow client applications to pin threads to CPUs. To start the virtual machine, your machine
  must meet the following requirements:
  * Run a supported OS: Windows, OS X, Linux
  * Run a 64-bit variant of the OS
  * Have a CPU with at least 4 logical cores
  * Have at least 4GB memory

  To run the virtual machine, you will need to install [Virtualbox]((https://www.virtualbox.org/wiki/Downloads)
  for running the VM and [Vagrant](https://www.vagrantup.com/downloads.html) for simple VM configuration. 

2. Set up the VM and get the source code on your development machine. You'll need to operate 
	and issue all your commands from the ** lab2 ** directory. 

	`git clone https://web.stanford.edu/class/cs240/lab2-skeleton.git lab2`

	Then, initialize the virtual machine by issuing the following commands in the lab2 directory:

    ```shell
    vagrant up    #this could take 10-15 minutes
    vagrant ssh
    ```

	To put the VM to sleep, you can run `vagrant suspend`. `vagrant up` brings the machine back up. 
	When/if you're done with the datastore, you can reclaim your resources by issuing `vagrant destroy`. 
	You must be in the lab2 directory when you run these commands.

	There's a lab2 directory in the VM's home directory, which mirrors the lab2 directory on your local 
	machine. Any changes you make to a file in either the VM or your local machine will be reflected
	to the other machine. 

3. Clone the dragonDB repository into the lab2 directory on your virtual machine. After ssh'ing into
   your virtual machine, issue the following commands:
     ```shell
     cd lab2
     git clone https://github.com/bmittelberger/dragonDB
     cd dragonDB/cs240_final_proj
     make
     ```

   This will generate the dragonDB executable.

##Tests/benchmarks:##

  1.) Scalability Tests

	* Large writes:

	If there are no files in the tests/ directory, navigate to the benchmarks directory and 
	run `bash make_big_puts.sh`. Then, navigate back to the cs240_final_proj directory and 
	run `bash test_speed_from_file.sh`. This test will demonstrate the scalability and 
	speed of our key-value store. 

	* Large reads and writes:

	Navigate to the `cs_240_final_proj` directory. In src/main.cpp, in the main 
  function around line 350, there are lines: 

    ```C
    test(threads, cores_used, num_cores, MIXED);
    test(threads, cores_used, num_cores, R_ONLY);
    test(threads, cores_used, num_cores, STRONG);
    ```

    Comment out the second R_ONLY line if you'd like to see the performance of 
    how the database handles a mixed read-and-write workload.

	* Large reads:

	Navigate to the `cs_240_final_proj` directory, and in src/main.cpp, make
  sure the line test(threads, cores_used, num_cores, R_ONLY) is commented
  in.

  2.) Durability Tests

  DragonDB is able to withstand corruptions given a power failure. As each segment
  writes to the database, it writes with a checksum on the data. When loading the most
  recently modified segment back into memory, dragonDB first recomputes the 
  checksum on the data. If the checksums match, the data has not been corrupted, 
  and dragonDB loads the segment back into memory. If they do not match, 
  dragonDB iterates through previous checksummed segments to determine which
  segment is not corrupted. It loads back the most recent segment with a valid 
  checksum. To test this, we did the following:
    1.) Run a dragonDB call to completion to populate a datastore.
    2.) Immitate a power failure/hardware corruption by deleting part 
        of the last segment written.
    3.) Run the same dragonDB call. It will load in the most recent
        non-corrupted segment.

    An example to see/try this out:
      ```shell
      ./dragonDB NUMCORES
      vim no_file-0.drg
      (delete the last line of the file, save and close)
      ./dragonDB NUMCORES
      ```

    The load indicate that there had previously been a corruption of the data, 
    but the segment will roll back to the most recent uncorrupted segment.


  3.) Consistency Tests:

  Our key-value store by default promises eventual consistency. With this design, 
  sometimes the caller may get back old data, but eventually, the most up-to-date 
  data will be flushed and reflected on disk.

  We understand, though, that some clients may prefer hard consistency to 
  scalability. Therefore, we designed our API to allow the user to specify 
  the level of consistency he/she would like. Currently, the "level" of 
  consistency is a binary--either the client would like strong consistency
  (at the cost of scalability) or the client would prefer weak
  consistency with high performance/scalability.

  With strong-consistency dragonDB, the database is flushed (with
  fsync so that it persists to disk rather than to the disk cache) after 
  every push.

  To compare the performance of strong-consistency to weak-consistency, 
  run the script test_consistency_perf.sh from the cs240_final_proj
  directory. This will output the time it takes 1-4 cores to do 
  10K gets and and puts requests for each model. 

    `./test_consistency_perf.sh`


##Embeddability## 
  Client-side application using the API:
  ---------------------------------------
  There are multiple ways to interact with dragonDB. We created a client-side application
  (main.cpp) that uses the dragonDB API directly. We've designed the application to 
  be user-friendly and take in command-line arguments. The client-side is responsible for 
  pinning threads to cores to handle the puts/gets/open/close processes. In default
  client-side application (when ./dragonDB is run with no arguments), the client spawns
  two threads, each of which put kev,value pairs and then after saving them to 
  the database, reads back in the values (the key and value are consecutively 
  increasing integers, i.e. puts 1 1; puts 2 2. Keys and values are stored as 
  strings. Thus, they are of variable size. We chose to do this to allow the client 
  maximum flexibility). 

  After interacting with this database, the client-side application opens a shell. This
  shell tells the not-so-tech-savy user how to interact with the key-value store. 
  The user can either open a new dragonDB kv-store, or (s)he can open the same kv-store
  that the program manipulated. 


  Interacting with the Shell
  ---------------------------------------
  To interact with the key-value store and dynamically create/manipulate a 
   key-value store on the fly, from the cs240_final_proj directory, run:
   `./dragonDB NUMTHREADS` (where) NUMTHREADS is the number of threads you'd like
   to run. 

   A command prompt will appear asking you to open an already existing data 
   store or to open a new data store. The first command should be 
   `open DATASTORE` (where datastore is the name of your key value store).
    
   The other commands that our dragonDB can understand from the shell are:
  	* puts key val
  	* get key    # will return value or an error message indicating that the key does not exist.  


  *** Storing mass puts/gets from a file ***
  The client-side application can read in a list of open/puts/get/close commands that 
  the user has already written if the user does not want to type all his commands in shell.
  To see how this works, you can write a command file with the following format:
    ```shell
    open DRAGON_STORE
    puts key val
    close
    ```
  For example, you can write a file called store_commands.tst, 
    ```shell
    open dragon_store
    puts armor 8
    puts space 10
    puts origami 2
    puts magic_dragon 9
    puts dragon_ball_z 10
    get armor
    get space
    get origami
    get magic_dragon
    get dragon_ball_z
    close
    ```

  This list of commands will save all of the key-value pairs in dragon_store-*.drg files. 
  It will also output in stdout, 
    ```shell
    armor : 8
    space : 10
    origami : 2
    magic_dragon : 9
    dragon_ball_z : 10
    ```

    However, by nature of our design, the order in which these outputs appear will 
    not be guaranteed.

  Commutativity
  ---------------------------------------
    Key-value pairs are stored across cores. Threads from other cores must be able to 
    interact with them. Each thread receives a process (puts/open/get/close) to execute 
    based on the load and scheduling of the other cores. It is highly unlikely that the 
    same thread will be assigned to handle the same process each time. In addition, 
    one thread may not see a key-value pair stored in another thread's segment
    until the other thread completes its get/put operation. As a result, one threads'
    get/put cannot depend on another thread's get/put. Therefore, every operation's
    memory accesses are conflict-free.


##Repository Contents##

Header file:
  * include/dragonDB.h: Main interface to the DB: Start here

C++ files:
  * src/main.cpp: The client-side application that calls dragonDB API functions.
                  The client program is in charge of spawning off however many
                  threads it wishes to work with and 
  * src/dragon_segment.cpp: 

