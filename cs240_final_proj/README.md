DragonDB
=========

Making a scalable, eventually-consistent, durable, commutative key-value store

Project done with:
Ben Mittelberger (https://github.com/bmittelberger)
Evan Sheih (https://github.com/eshieh)
Claudia Roberts (https://github.com/chicacvr)

## Setup ##
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

	```
  vagrant up #this could take 10-15 minutes
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
   ```
   cd lab2
   git clone https://github.com/bmittelberger/dragonDB
   cd dragonDB/cs240_final_proj
   make
   ```

   This will generate the dragonDB executable.

4. To run the tests/benchmarks:
	**Large writes:**
	If there are no files in the tests/ directory, navigate to the benchmarks directory and 
	run `bash make_big_puts.sh`. Then, navigate back to the cs240_final_proj directory and 
	run `bash test_speed_from_file.sh`. This test will demonstrate the scalability and 
	speed of our key-value store. 

	**Large reads and writes:**
	Navigate to the `cs_240_final_proj` directory. In src/main.cpp, around line 315, 
	there are lines: 

	
    ```
    test(threads, cores_used, num_cores, MIXED);
    test(threads, cores_used, num_cores, R_ONLY);
    ```

    Comment out the second R_ONLY line if you'd like to see the performance of 
    how the database handles a mixed read-and-write workload.

	**Large reads:**
	Navigate to the `cs_240_final_proj` directory, and in src/main.cpp, make
  sure the line test(threads, cores_used, num_cores, R_ONLY) is commented
  in.

5. To interact with the key-value store and dynamically create/manipulate a 
   key-value store on the fly, from the cs240_final_proj directory, run:
   `./dragonDB NUMTHREADS` (where) NUMTHREADS is the number of threads you'd like
   to run. 

   A command prompt will appear asking you to open an already existing data 
   store or to open a new data store. The first command should be 
   `open DATASTORE` (where datastore is the name of your key value store).
    
   The other commands that our dragonDB can understand from the shell are:
	* puts key val
	* get key <-- will return value or an error message indicating
	  that the key does not exist.  
