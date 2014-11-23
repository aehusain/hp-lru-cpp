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

#pragma once

/*
 * Cache Layout:
 *
 * Cache layout is inspired by NoSQL database. The system is designed
 * to hold multiple Rows; an abstraction t hold multiple unique
 * <key, value> pairs. Each row is created with a definite capacity,
 * the keys within a row respects Least-Recently-Policy i.e. the key
 * which is least visited is a candidate to be evicted if row real-estate
 * becomes sparse.
 *
 *        |<---------LRU(capacity)---------->|
 *  _________________________________________
 * |      |key1   | key2 |           | key-n |
 * | ROW-1|_______|______|___________|_______|
 * |      | Value1|Value2|           |Value-n|
 * |______|_______|______|___________|_______|
 * |      |                                  |
 * | ROW-2|                                  |
 * |______|__________________________________|
 *
 */

#include "CLru.h"
#include "Locks.h"
#include <memory>
#include <unordered_map>

using key_type = uint64_t;
typedef struct PaylodTag
{
    uint64_t value_;
    PaylodTag(uint64_t value) : value_(value)
    { }
} Payload;
using PayloadPtr = std::shared_ptr<Payload>;

using LRU = clru::Lru<key_type,
                      PayloadPtr,
                      std::unordered_map>;
using LruPtr = std::shared_ptr<LRU>;

/*
 * Cache
 */
class Cache
{
    public:
        Cache(const std::string &row, uint64_t capacity);
        ~Cache()
        { lru_.reset(); }
        void insert(key_type key, PayloadPtr value);
        PayloadPtr lookup(key_type key);
    private:
        std::string row_;
        uint64_t capacity_;
        LruPtr lru_;
}; // Cache

using CachePtr = std::shared_ptr<Cache>;

/*
 * CacheMgr
 *
 * Singleton abstraction, maintains access to underlyinf LRU
 * per Key instance.
 */
class CacheMgr;
using CacheMgrPtr = std::shared_ptr<CacheMgr>;

class CacheMgr
{
    public:
        ~CacheMgr() = default;
        static CacheMgrPtr allocate()
        {
            CacheMgrPtr ptr(new CacheMgr());
            return ptr;
        }
        // row operations
        void addRow(const std::string &row, const uint64_t capacity);
        void deleteRow(const std::string &row);
        // column operations
        void insertKey(const std::string &row,
                       key_type key,
                       PayloadPtr value);
        PayloadPtr lookupKey(const std::string &row,
                             key_type key);
    private:
        CacheMgr() = default;
        CachePtr lookupRow(const std::string &row);

        std::unordered_map<std::string, CachePtr> rowMap_;
        lock::ReadWriteMutex mutex_;
}; // CacheMgr

namespace hplru
{
void allocate();
void teardown();
void addRow(const std::string &row, uint64_t capacity);
void insertKey(const std::string &row,
               key_type key,
               uint64_t value);
PayloadPtr lookup(const std::string &row, key_type key);
} // hplru
