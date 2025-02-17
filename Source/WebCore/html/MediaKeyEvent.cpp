/*
 * Copyright (C) 2012 Google Inc.  All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"

#if ENABLE(LEGACY_ENCRYPTED_MEDIA_V1)

#include "MediaKeyEvent.h"

#include "EventNames.h"

namespace WebCore {

MediaKeyEvent::MediaKeyEvent(const AtomicString& type, const String& keySystem, const String& sessionId, RefPtr<Uint8Array>&& initData, RefPtr<Uint8Array>&& message, const String& defaultURL, RefPtr<WebKitMediaKeyError>&& errorCode, uint32_t systemCode)
    : Event(type, false, false)
    , m_keySystem(keySystem)
    , m_sessionId(sessionId)
    , m_initData(initData)
    , m_message(message)
    , m_defaultURL(defaultURL)
    , m_errorCode(errorCode)
    , m_systemCode(systemCode)
{
}

MediaKeyEvent::MediaKeyEvent(const AtomicString& type, const MediaKeyEventInit& initializer)
    : Event(type, initializer, IsTrusted::No)
    , m_keySystem(initializer.keySystem)
    , m_sessionId(initializer.sessionId)
    , m_initData(initializer.initData)
    , m_message(initializer.message)
    , m_defaultURL(initializer.defaultURL)
    , m_errorCode(initializer.errorCode)
    , m_systemCode(initializer.systemCode)
{
}

MediaKeyEvent::~MediaKeyEvent()
{
}

EventInterface MediaKeyEvent::eventInterface() const
{
    return MediaKeyEventInterfaceType;
}

} // namespace WebCore

#endif
