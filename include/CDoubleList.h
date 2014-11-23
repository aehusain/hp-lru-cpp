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
#include <memory>

namespace cdouble_list
{

using namespace lock;

template<class T>
class Node
{
public:
    T data_;
    Node<T> *next_;
    Node<T> *prv_;

    Node(const T &data):
        data_(data),
        next_(nullptr),
        prv_(nullptr)
    { }
};

template<class T>
class CDoubleList
{
public:
    Node<T> *head_;
    Node<T> *tail_;
    SpinLock lock_;
public:
    CDoubleList():
        head_(nullptr),
        tail_(nullptr)
    { }

    Node<T>*
    insertFirst(const T &data)
    {
        Node<T> *node = new Node<T>(data);
        GuardSpinLock lock(lock_);
        if (!head_)
        {
            head_ = node;
            tail_ = node;
        } else if (head_->data_ == tail_->data_) {
            node->next_ = head_;
            tail_->prv_ = head_->prv_ = node;
            head_ = node;
        } else {
            node->next_ = head_;
            head_->prv_ = node;
            head_ = node;
        }
        return node;
    }
    void
    promote(Node <T> *node)
    {
        GuardSpinLock lock(lock_);
        if (node->data_ == head_->data_)
        {
            // do nothing
        }
        else if (node->data_ == tail_->data_)
        {
            head_->prv_ = tail_;
            tail_->next_ = head_;
            tail_ = tail_->prv_;
            head_ = head_->prv_;
            tail_->next_ = nullptr;
            head_->prv_ = nullptr;
        }
        else
        {
            node->prv_->next_ = node->next_;
            node->next_->prv_ = node->prv_;
            head_->prv_ = node;
            node->next_ = head_;
            node->prv_ = nullptr;
            head_ = node;
        }
    }
    Node<T>*
    repositionTail(int numElements)
    {
        if (numElements <= 0) {
            return nullptr;
        }
        GuardSpinLock lock(lock_);
        Node<T> *temp = tail_;
        for (int i = 0;
             i < numElements && temp != nullptr; ++i) {
            temp = temp->prv_;
        }
        tail_ = temp;
        temp = temp->next_;
        tail_->next_ = nullptr;
        temp->prv_= nullptr;
        return temp;
    }
};

} // cdouble_list
