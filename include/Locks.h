#pragma once

/*
 * HpLRU
 *
 * Copyright (C) <year>  <name of author>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Ata E Husain Bohra(ata.husain@hotmail.com)
 */

#include <boost/thread/shared_mutex.hpp>
#include <pthread.h>
#include <mutex>

namespace lock
{

typedef boost::shared_mutex ReadWriteMutex;
typedef boost::shared_lock<ReadWriteMutex> ReadLock;
typedef boost::unique_lock<ReadWriteMutex> WriteLock;

typedef std::mutex Mutex;
typedef std::lock_guard<std::mutex> GuardLock;

class SpinLock
{
public:
    SpinLock()
    {
        pthread_spin_init(&mutex_, PTHREAD_PROCESS_PRIVATE);
    }
    ~SpinLock()
    {
        pthread_spin_destroy(&mutex_);
    }
    void lock()
    {
        pthread_spin_lock(&mutex_);
    }
    void tryLock()
    {
        pthread_spin_trylock(&mutex_);
    }
    void unlock()
    {
        pthread_spin_unlock(&mutex_);
    }
private:
    pthread_spinlock_t mutex_;
}; // SpinLock

class PthreadWriteAutoLock
{
public:
    PthreadWriteAutoLock(pthread_rwlock_t &lock) : lock_(lock)
    { 
        pthread_rwlock_wrlock(&lock_);
    }
    ~PthreadWriteAutoLock()
    {
        pthread_rwlock_unlock(&lock_);
    }
private:
    pthread_rwlock_t &lock_;
};
class PthreadReadAutoLock
{
public:
    PthreadReadAutoLock(pthread_rwlock_t &lock) : lock_(lock)
    { 
        pthread_rwlock_rdlock(&lock_);
    }
    ~PthreadReadAutoLock()
    {
        pthread_rwlock_unlock(&lock_);
    }
private:
    pthread_rwlock_t &lock_;
};

typedef std::lock_guard<SpinLock> GuardSpinLock;

} // lock
