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

#include "Locks.h"
#include "ThreadPool.h"
#include "CDoubleList.h"
#include "glog/logging.h"

#include <atomic>

namespace
{
const int EVICTION_THRESHOLD = 90; // 90%
}

namespace clru
{

using namespace cdouble_list;
using namespace lock;

template<typename key, typename value, template<typename ...> class MAP>
class Lru
{
public :
    using LIST_ELEMENT_ = std::pair<value, key>;
    using DoubleListNodePtr = Node <LIST_ELEMENT_>*;
    using VALUE_TRACKER_ = std::pair<value, DoubleListNodePtr >;
    using MAP_ELE_ = std::pair<key, VALUE_TRACKER_ >;
    using MAP_ = MAP<key, VALUE_TRACKER_ >;

    Lru(int capacity):
        capacity_(capacity),
        current_(0),
        evictionInProgress_(false)
    { }

    void resize(int size)
    {
	capacity_ = size;
	checkAndTriggerEviction();
    }
    void
    checkAndTriggerEviction()
    {
        if (current_ < ((capacity_*EVICTION_THRESHOLD)/100)) {
            return;
        }
        if ( !evictionInProgress_ ) {
            bool oldValue = evictionInProgress_.exchange(true);
            if (oldValue == true) {
                return;
            }
        } else {
            return ;
        }
        VLOG(0) << "Eviction triggered:capacity:" << capacity_ << ":current:" << current_;
        // reposition double list tail node
        DoubleListNodePtr ptr = list_.repositionTail(current_*.1);
        if (ptr == nullptr) {
            return ;
        }
        current_ = current_*.9;
        threadpool::enqueue(
          [ptr, this] () {
          this->evictNodes(ptr);
        });
    }
    void
    evictNodes(DoubleListNodePtr ptr)
    {
        while (ptr != nullptr) {
            LIST_ELEMENT_ l_element = ptr->data_;
            map_.erase(l_element.second);
            DoubleListNodePtr temp = ptr->next_;
            delete(ptr);
            ptr = temp;
        }
        evictionInProgress_.exchange(false);
    }

    void
    insert(key k, value v)
    {
        typename MAP_::iterator itr = map_.find(k);
        // check if element exists
        if (itr != map_.end())
        {
            VLOG(0) << "update:key:" << k;
            // update value
            VALUE_TRACKER_ &tracker = itr->second;
            tracker.first = v;
            list_.promote(tracker.second);
        }
        else
        {
            VLOG(0) << "insert:key:" << k;
            // update the list with new member
            LIST_ELEMENT_ l_element = std::make_pair(v, k);
            DoubleListNodePtr node = list_.insertFirst(l_element);
            VALUE_TRACKER_ tracker = std::make_pair(v, node);
            //map_.insert(k, tracker);
            ++current_;
        }
        checkAndTriggerEviction();
    }
    // lookup function
    value
    lookup(const key &k)
    {
        // does the element exist in the map
        typename MAP_::iterator itr = map_.find(k);
        if (itr == map_.end())
        {
            VLOG(0) << "lookup missed:key:" << k;
            return nullptr;
        }

        // reposition it to the top of the list
        VALUE_TRACKER_ &tracker = itr->second;
        list_.promote(tracker.second);
	VLOG(0) << "lookup success:key:" << k;
        return tracker.first;
    }

    /* Interfaces for testing/debugging purposes ONLY*/
    uint64_t
    usage() const
    {
        return current_;
    }
    uint64_t
    size() const
    {
        return capacity_;
    }

private :
    // avoid copy and assignement operation
    Lru(const Lru &) = delete;
    Lru& operator=(const Lru &) = delete;

    int capacity_;
    int current_;
    CDoubleList< LIST_ELEMENT_ > list_;
    MAP_ map_;
    std::atomic_bool evictionInProgress_;
};

} // clru
