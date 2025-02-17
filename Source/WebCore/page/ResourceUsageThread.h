/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(RESOURCE_USAGE)

#include "ResourceUsageData.h"
#include <array>
#include <functional>
#include <wtf/Condition.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/NeverDestroyed.h>
#include <wtf/Noncopyable.h>

namespace JSC {
class VM;
}

namespace WebCore {

class ResourceUsageThread {
    WTF_MAKE_NONCOPYABLE(ResourceUsageThread);

public:
    static void addObserver(void* key, std::function<void (const ResourceUsageData&)>);
    static void removeObserver(void* key);

private:
    friend NeverDestroyed<ResourceUsageThread>;
    ResourceUsageThread();
    static ResourceUsageThread& singleton();

    void waitUntilObservers();
    void notifyObservers(ResourceUsageData&&);

    void createThreadIfNeeded();
    static void threadCallback(void* scrollingThread);
    void threadBody();
    void platformThreadBody(JSC::VM*, ResourceUsageData&);

    ThreadIdentifier m_threadIdentifier { 0 };
    Lock m_lock;
    Condition m_condition;
    HashMap<void*, std::function<void (const ResourceUsageData&)>> m_observers;

    // Platforms may need to access some data from the common VM.
    // They should ensure their use of the VM is thread safe.
    JSC::VM* m_vm { nullptr };
};

#if PLATFORM(COCOA)
struct TagInfo {
    TagInfo() { }
    size_t dirty { 0 };
    size_t reclaimable { 0 };
};

const char* displayNameForVMTag(unsigned);
std::array<TagInfo, 256> pagesPerVMTag();
void logFootprintComparison(const std::array<TagInfo, 256>&, const std::array<TagInfo, 256>&);
#endif

} // namespace WebCore

#endif // ENABLE(RESOURCE_USAGE)
