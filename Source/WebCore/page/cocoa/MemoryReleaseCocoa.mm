/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
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

#include "config.h"
#include "MemoryRelease.h"

#import "GCController.h"
#import "IOSurfacePool.h"
#import "LayerPool.h"
#import <notify.h>

extern "C" void _sqlite3_purgeEligiblePagerCacheMemory(void);

namespace WebCore {

void platformReleaseMemory(Critical)
{
    {
        MemoryPressureHandler::ReliefLogger log("Purging SQLite caches");
        _sqlite3_purgeEligiblePagerCacheMemory();
    }

    {
        MemoryPressureHandler::ReliefLogger log("Drain LayerPools");
        for (auto& pool : LayerPool::allLayerPools())
            pool->drain();
    }
#if USE(IOSURFACE)
    {
        MemoryPressureHandler::ReliefLogger log("Drain IOSurfacePool");
        IOSurfacePool::sharedPool().discardAllSurfaces();
    }
#endif
}

void jettisonExpensiveObjectsOnTopLevelNavigation()
{
#if PLATFORM(IOS)
    // Protect against doing excessive jettisoning during repeated navigations.
    const auto minimumTimeSinceNavigation = 2s;

    static auto timeOfLastNavigation = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    bool shouldJettison = now - timeOfLastNavigation >= minimumTimeSinceNavigation;
    timeOfLastNavigation = now;

    if (!shouldJettison)
        return;

    // Throw away linked JS code. Linked code is tied to a global object and is not reusable.
    // The immediate memory savings outweigh the cost of recompilation in case we go back again.
    GCController::singleton().deleteAllLinkedCode();
#endif
}

void registerMemoryReleaseNotifyCallbacks()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [] {
        int dummy;
        notify_register_dispatch("com.apple.WebKit.fullGC", &dummy, dispatch_get_main_queue(), ^(int) {
            GCController::singleton().garbageCollectNow();
        });
        notify_register_dispatch("com.apple.WebKit.deleteAllCode", &dummy, dispatch_get_main_queue(), ^(int) {
            GCController::singleton().deleteAllCode();
            GCController::singleton().garbageCollectNow();
        });
    });
}

} // namespace WebCore
