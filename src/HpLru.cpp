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

#include "HpLru.h"
#include <unordered_map>

#include <cstdint>

// Cache defines
Cache::Cache(const std::string &row,
             uint64_t capacity)
    : row_(row)
    , capacity_(capacity)
{
    lru_.reset(new LRU(capacity_));
}

void
Cache::insert(key_type key, PayloadPtr value)
{
    lru_->insert(key, value);
}

PayloadPtr
Cache::lookup(key_type key)
{
    return lru_->lookup(key);
}

CachePtr
CacheMgr::lookupRow(const std::string &row)
{
    lock::ReadLock rlock(mutex_);
    auto it = rowMap_.find(row);
    return it->second;
}

void
CacheMgr::addRow(const std::string &row,
                 uint64_t capacity)
{
    // add Row if not already added.
    lock::ReadLock rlock(mutex_);
    auto it = rowMap_.find(row);
    // add if Row is missing
    if (it == rowMap_.end()) {
        // upgrade lock to exclusive type
        rlock.release();
        mutex_.unlock_shared();
        {
            lock::WriteLock _(mutex_);
            auto it = rowMap_.find(row);
            if (it != rowMap_.end()) return;
            CachePtr cache = std::make_shared<Cache>(row, capacity);
            rowMap_.insert(std::make_pair<>(row, cache));
        }
    } else {
        // row already added, nothing needs to be done.
    }
}

void
CacheMgr::deleteRow(const std::string &row)
{
    auto it = lookupRow(row);
    // free up all columns for this row
    if (it != nullptr) it.reset();
}

void
CacheMgr::insertKey(const std::string &row,
                    key_type key,
                    PayloadPtr value)
{
    auto it = lookupRow(row);
    if (it == nullptr) return;
    it->insert(key, value);
}

PayloadPtr
CacheMgr::lookupKey(const std::string &row, key_type key)
{
    auto it = lookupRow(row);
    if (it == nullptr) return nullptr;
    return it->lookup(key);
}

uint64_t
CacheMgr::size(const std::string &row)
{
    auto it = lookupRow(row);
    if (it == nullptr) return 0;
    return it->size();
}

uint64_t
CacheMgr::usage(const std::string &row)
{
    auto it = lookupRow(row);
    if (it == nullptr) return 0;
    return it->usage();
}

namespace
{
CacheMgrPtr _instance;
}

namespace hplru
{
void allocate()
{
    assert(!_instance);
    _instance = CacheMgr::allocate();
}
void teardown()
{
    assert(_instance);
    _instance.reset();
}
void addRow(const std::string &row,
            uint64_t capacity)
{
    assert(_instance);
    _instance->addRow(row, capacity);
}
void insertKey(const std::string &row,
               key_type key,
               uint64_t value)
{
    assert(_instance);
    PayloadPtr payload = std::make_shared<Payload>(value);
    _instance->insertKey(row, key, payload);
}
PayloadPtr lookup(const std::string &row, key_type key)
{
    assert(_instance);
    return _instance->lookupKey(row, key);
}
uint64_t size(const std::string &row)
{
    assert(_instance);
    return _instance->size(row);
}
uint64_t usage(const std::string &row)
{
    assert(_instance);
    return _instance->usage(row);
}

} //hplru
