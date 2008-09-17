//---------------------------------------------------------- -*- Mode: C++ -*-
// $Id$ 
//
// Created 2006/03/31
// Author: Sriram Rao (Kosmix Corp.) 
//
// Copyright 2006 Kosmix Corp.
//
// This file is part of Kosmos File System (KFS).
//
// Licensed under the Apache License, Version 2.0
// (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.
//
// 
//----------------------------------------------------------------------------

#include "EventManager.h"
#include "Globals.h"

using std::list;
using namespace KFS;
using namespace KFS::libkfsio;

EventManager::EventManager()
{
    mCurrentSlot = 0;
    mEventManagerTimeoutImpl = new EventManagerTimeoutImpl(this);
}

EventManager::~EventManager()
{
    for (int i = 0; i < MAX_EVENT_SLOTS; i++)
        mSlots[i].clear();

    mLongtermEvents.clear();
}

void
EventManager::Init()
{
    // The event manager schedules events at 10ms granularity.
    mEventManagerTimeoutImpl->SetTimeoutInterval(EVENT_GRANULARITY_MS);
    globals().netManager.RegisterTimeoutHandler(mEventManagerTimeoutImpl);
}

void EventManager::Schedule(EventPtr &event, int afterMs)
{
    int slot;

    assert(afterMs >= 0);

    event->SetStatus(EVENT_SCHEDULED);

    if (afterMs <= 0)
        slot = (mCurrentSlot + 1) % MAX_EVENT_SLOTS;
    else
        slot = afterMs / EVENT_GRANULARITY_MS;

    if (slot > MAX_EVENT_SLOTS) {
        event->SetLongtermWait(afterMs);
        mLongtermEvents.push_back(event);
        return;
    }
    slot = (MAX_EVENT_SLOTS + slot - mCurrentSlot) % MAX_EVENT_SLOTS;
    mSlots[slot].push_back(event);

}

void EventManager::Timeout()
{
    list <EventPtr>::iterator iter, eltToRemove;
    EventPtr event;
    int waitMs;
    int msElapsed = mEventManagerTimeoutImpl->GetTimeElapsed();

    for (iter = mSlots[mCurrentSlot].begin(); 
         iter != mSlots[mCurrentSlot].end(); ++iter) {
        event = *iter;
        event->EventOccurred();
        // Reschedule it if it is a periodic event
        if (event->IsPeriodic())
            Schedule(event, event->GetTimeout());
        
    }
    mSlots[mCurrentSlot].clear();
    mCurrentSlot++;
    if (mCurrentSlot == MAX_EVENT_SLOTS)
        mCurrentSlot = 0;

    if ((mLongtermEvents.size() > 0) &&
        (msElapsed - EVENT_GRANULARITY_MS >= 3 * EVENT_GRANULARITY_MS)) {
        KFS_LOG_VA_DEBUG("Elapsed ms = %d", msElapsed);
    }
    // Now, pull all the long-term events
    iter = mLongtermEvents.begin();
    while (iter != mLongtermEvents.end()) {
        event = *iter;
        // count down for each ms that ticks by
        waitMs = event->DecLongtermWait(msElapsed);
        if (waitMs >= MAX_EVENT_SLOTS) {
            ++iter;
            continue;
        }
        // we have counted down to "short-term" amount
        Schedule(event, waitMs);
        eltToRemove = iter;
        ++iter;
        mLongtermEvents.erase(eltToRemove);
    }

}