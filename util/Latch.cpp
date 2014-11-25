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

#include "Latch.h"
#include "Locks.h"

using namespace latch;

Latch::Latch(size_t count)
        : count_(count)
        , forceRelease_(false)
{ }

void
Latch::countDown()
{
    lock::GuardLock guard(lock_);
    if (forceRelease_) return;
    if (count_ == 0) {
        throw std::logic_error("Error count is 0\n");
    }
    if (--count_ == 0) {
        // wake up all the threads
        cnd_.notify_all();
    }
}

void
Latch::wait()
{
    lock::MutexLock lck(lock_);
    while (count_ > 0) cnd_.wait(lck);
}

bool
Latch::tryWait()
{
    lock::MutexLock lck(lock_);
    return count_ == 0;
}

void
Latch::forceRelease()
{
    lock::GuardLock guard(lock_);
    if (forceRelease_) return;
    else {
        forceRelease_ = true;
        count_ = 0;
        cnd_.notify_all();
    }
}

