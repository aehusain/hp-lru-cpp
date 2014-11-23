HpLRU
=====

HpLRU stands for HighPerformance LRU.

One of the core component in developing a performant systems software is caching sub-component. Few challenges involved in design caching sub-system are:
1. Thread-safe design to allow multi-threaded access.
2. Low-latency operations including insert/lookup.
3. Eviction strategy to better utilize memory sub-system (RAM, SSD or HDD).

Implementation:

Architecture:

      HashMap                           Doubly Linked List
   _______________              
  | key-1 | cell-1|               cell-1             cell-2       cell-j        cell-n
  |_______________|              _________
  | key-2 | cell-2|             | key-1   |
  |_______________|             |_________|-------->  ...         ...             
  | ...   | ...   |             | payload |<--------  
  |_______________|             |_________|
  | ...   | ....  |
  |_______________|
  | key-n | cell-n|
  |_______________|

Cache Interface:

	        |<---- Row-Bucket-------------->|
	 __________________________________________
  |       | key-1   |               | key-n  |
  | Row-1 |_________|               |________|
  |       | payload |               |payload |
  |_______|_________|_______________|________|

Interface framework design is inspired by NoSQL database. The data is layout in to two major components:
1. Row
2. Columns
 
Each row is identified by a unique key, each row is a bucket of <key, value> pair. Each bucket holds 'n' pairs defined by user. The keys within a bucket respects a LRU eviction policy.

Build

Library dependancies:
1. boost (tested on boost-1.53)
2. gcc/g++ 4.8.1
3. Google logger - 0.3.3.

$ ./configure
$ make

Contribitors
1. Ata E Husain Bohra (ata.husain@hotmail.com)
