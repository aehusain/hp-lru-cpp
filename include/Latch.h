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
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <memory>

namespace latch
{
    /* Latch
     *
     * Synchronization barrier; supported operations are:
     * 1. countdown()
     * 2. wait()
     */
    class Latch
    {
        private:
            size_t                      count_;
            bool                        forceRelease_;
            std::condition_variable     cnd_;
            lock::Mutex                 lock_;

        public:
            explicit Latch(size_t count);
            // Decrement the count. When count reaches ZERO, all waiting
            // threads are released.
            //
            // throws std::logic_error if called on count == 0
            void countDown();
            // Wait till count reaches ZERO
            void wait();
            // Will caller thread wait?
            bool tryWait();
            // force relase the latch
            void forceRelease();
        private:
            // avoid copy and assignment operation
            Latch(const Latch&) = delete;
            Latch& operator=(const Latch&) = delete;

    }; // Latch
    using LatchPtr = std::shared_ptr<Latch>;

} // latch
