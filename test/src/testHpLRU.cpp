/*
 * testHpLRU.cpp
 *
 *  Created on: Jan 15, 2014
 *      Author: Nishith
 */

#include "HpLru.h"
#include "ThreadPool.h"
#include "Latch.h"
#include <glog/logging.h>
#include <iostream>
#include <chrono>

namespace
{
const char* logger = "testlogdir";
const char* module = "hplru";

uint64_t getTimeInMs()
{
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

} // unnamed

void
initilialize(int threads)
{
    // initialize threadpool
    threadpool::allocate(threads);
    hplru::allocate();
    // Google Logging initialize
    FLAGS_logtostderr = 0;
    FLAGS_v = 0;
    FLAGS_minloglevel = google::INFO;
    FLAGS_max_log_size = 1000;
    FLAGS_log_dir = logger;

    google::InitGoogleLogging(module);
}
void
teardown()
{
    hplru::teardown();
    threadpool::teardown();
    // shutdown logger
    google::ShutdownGoogleLogging();
}
void
insert(const char *row, int count, int start, latch::Latch &latch)
{
    LOG(INFO) << "Thread:" << start / count << std::endl;
    for(int i = start; i < count; ++i) {
        hplru::insertKey(row, i, i);
    }
    latch.countDown();
}

void
extract(const char *row, int count, int start, latch::Latch &latch)
{
    LOG(INFO) << "Thread:" << start / count << std::endl;
    for(int i = start; i < count; ++i) {
        PayloadPtr payloadPtr = hplru::lookup(row, i);
        if (!payloadPtr) {
            LOG(INFO) << "Miss";
        } else if ( ((unsigned int)i != payloadPtr.get()->value())) {
            LOG(ERROR) << "Failure for key:" << i << ":expected:"
                       << i << ":actual:" << payloadPtr.get()->value();
        }
        latch.countDown();
    }
}

void
testSanity(int size, int count, int buckets, int threads, int runs)
{
    const char *row = "sanity";
    hplru::addRow(row, size);

    uint64_t tStart = getTimeInMs();
    latch::Latch l(threads);
    for(int i = 0; i < threads; ++i) {
        threadpool::enqueue([row, buckets, i, &l] {
             insert(row, buckets, i * buckets, l);
          });
    }
    l.wait();
    LOG(INFO) << "Insertions done.";
    latch::Latch l1(runs);
    while(runs--) {
	for(int i = 0; i < threads; ++i) {
		threadpool::enqueue([row, buckets, i, &l1] {
			extract(row, buckets, i * buckets, l1);
		});
	}
    }
    l1.wait();
    uint64_t tStop = getTimeInMs();
    LOG(INFO) << "Sanity Time taken:" << (tStop - tStart);
}

void
testConcurrency(int size, int count, int buckets, int threads, int runs)
{
    const char *row = "concurrency";
    hplru::addRow(row, size);

    uint64_t tStart = getTimeInMs();
    latch::Latch l(threads);
    // let operation be done in parallel
    for(int i = 0; i < threads; ++i) {
        threadpool::enqueue([row, buckets, i, &l] {
            insert(row, buckets, i * buckets, l);
          });
    }
    latch::Latch l1(threads);
    for(int i = 0; i < threads; ++i) {
        threadpool::enqueue([row, buckets, i, &l1] {
           extract(row, buckets, i * buckets, l1);
          });
    }

    l.wait();
    l1.wait();
    uint64_t tStop = getTimeInMs();
    LOG(INFO) << "Concurrency Time taken:" << (tStop - tStart);
}

void
testEviction(uint64_t size)
{
    const char *row = "eviction";
    // create a list of 10 elements and trigger eviction
    hplru::addRow(row, size);
    for(uint64_t i = 0; i < 4*size; ++i)
        hplru::insertKey(row, i, i);
    // wait for eviction threads to finish
    uint64_t usage = hplru::usage(row);
    uint64_t capacity = hplru::size(row);
    LOG(INFO) << "Usage:" << usage << ":Capacity:" << capacity;
    if (size < usage)
        LOG(ERROR) << "Eviction Failure, usage:" << usage << ":capacity:" << capacity;
}

int
main(int argc, char *argv[])
{
    if(argc < 5) {
        std::cout << "Usage: " << argv[0]
                  << " <cache-size> <count> <threads> <runs>" << std::endl;
        return 0;
    }

    int size = atoi(argv[1]);
    int count = atoi(argv[2]);
    int threads = atoi(argv[3]);
    int runs = atoi(argv[4]);
    int buckets = count / threads;
    {
        initilialize(threads);

        testSanity(size, count, buckets, threads, runs);
        testConcurrency(size, count, buckets, threads, runs);
        testEviction(size);

        teardown();
    }
    return 0;
}

