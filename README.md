HpLRU
=====
HpLRU stands for HighPerformance LRU.

One of the core component in developing a performant
systems software is caching sub-component.
Few challenges involved in design caching sub-system are:
1. Thread-safe design to allow multi-threaded access.
2. Low-latency operations including insert/lookup.
3. Eviction strategy to better utilize memory sub-system (RAM, SSD or HDD).

Implementation:
==============

Architecture:
=============

	HashMap					Double Linked List
        ------					-----------------

	 _______________	 cell-1		   cell-2	cell-n
	|key-1 | cell-1 |		
	|---------------|	 ---------
	|key-2 | cell-2 |	| key-1   |
	|---------------|	|---------|-------> ...		...
	|key-3 | cell-3 |       | payload |<------
	|---------------|        ----------
	|               |
	|key-n | cell-n |
	|               |
	---------------

Cache Interface:
===============

Below is the architecture of Cache Interface:

	 --------------------------------------
	|       | key-1  | key-2  |	 key-n|
	| Row-1 |------- |--------|-----------|
	|	|	 |	  |	      |
	|	|payload | payload|	      |
	 -------------------------------------

Interface framework design is inspired by NoSQL database. The data is layout in to two major components:
1. Row
2. Columns
 
Each row is identified by a unique key, each row is a bucket of <key, value> pair.
Each bucket holds 'n' pairs defined by user. The keys within a bucket respects a LRU eviction policy.
Interface framework design is inspired by NoSQL database. The data is layout in to two major components:
1. Row
2. Columns

Each row is identified by a unique key, each row is a bucket of <key, value> pair.
Each bucket holds 'n' pairs defined by user. The keys within a bucket respects a LRU eviction policy.

Eviction
========

For a high performant LRU the eviction strategy plays a very significant role. Basic operarions performed during an eviction involves: updating backend-list candidate removal as well as update to front-end hashmap. In-order to shield active I/O performance impact due to eviction, HpLRU designed a novel mechanism. It uses back-ground task (leveraging configurable threadpool threads) to perform eviction based on user-defined "water highmarks". On an insert if the highmark are breached systems schedules eviction thread, which attempts to clean up least recently evicted keys and bring space consumption down below specified highmarks.

Build
====

Library dependancies:
1. boost (tested on boost-1.53)
	2. gcc/g++ 4.8.1
	3. Google logger - 0.3.3.

	$ ./configure
	$ make

	Contribitors
1. Ata E Husain Bohra (ata.husain@hotmail.com)
