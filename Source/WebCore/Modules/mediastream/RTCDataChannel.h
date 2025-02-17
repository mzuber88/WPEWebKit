/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(WEB_RTC)

#include "Event.h"
#include "EventTarget.h"
#include "ExceptionOr.h"
#include "RTCDataChannelHandler.h"
#include "RTCDataChannelHandlerClient.h"
#include "ScriptWrappable.h"
#include "Timer.h"

namespace JSC {
    class ArrayBuffer;
    class ArrayBufferView;
}

namespace WebCore {

class Blob;
class Dictionary;
class RTCPeerConnectionHandler;

class RTCDataChannel final : public RefCounted<RTCDataChannel>, public EventTargetWithInlineData, public RTCDataChannelHandlerClient {
public:
    static Ref<RTCDataChannel> create(ScriptExecutionContext&, std::unique_ptr<RTCDataChannelHandler>&&, String&&, RTCDataChannelInit&&);

    bool ordered() const { return m_options.ordered; }
    unsigned short maxRetransmitTime() const { return m_options.maxRetransmitTime; }
    unsigned short maxRetransmits() const { return m_options.maxRetransmits; }
    String protocol() const { return m_options.protocol; }
    bool negotiated() const { return m_options.negotiated; };
    unsigned short id() const { return m_options.id; };

    String label() const { return m_label; }
    const AtomicString& readyState() const;
    unsigned bufferedAmount() const;

    const AtomicString& binaryType() const;
    ExceptionOr<void> setBinaryType(const AtomicString&);

    ExceptionOr<void> send(const String&);
    ExceptionOr<void> send(JSC::ArrayBuffer&);
    ExceptionOr<void> send(JSC::ArrayBufferView&);
    ExceptionOr<void> send(Blob&);

    void close();

    void stop();

    using RefCounted::ref;
    using RefCounted::deref;

private:
    RTCDataChannel(ScriptExecutionContext&, std::unique_ptr<RTCDataChannelHandler>&&, String&&, RTCDataChannelInit&&);

    void scheduleDispatchEvent(Ref<Event>&&);
    void scheduledEventTimerFired();

    EventTargetInterface eventTargetInterface() const final { return RTCDataChannelEventTargetInterfaceType; }
    ScriptExecutionContext* scriptExecutionContext() const final { return m_scriptExecutionContext; }

    void refEventTarget() final { ref(); }
    void derefEventTarget() final { deref(); }

    ScriptExecutionContext* m_scriptExecutionContext;

    void didChangeReadyState(ReadyState) final;
    void didReceiveStringData(const String&) final;
    void didReceiveRawData(const char*, size_t) final;
    void didDetectError() final;
    void protect() final { ref(); }
    void unprotect() final { deref(); }

    std::unique_ptr<RTCDataChannelHandler> m_handler;

    bool m_stopped { false };

    ReadyState m_readyState { ReadyStateConnecting };

    enum class BinaryType { Blob, ArrayBuffer };
    BinaryType m_binaryType { BinaryType::ArrayBuffer };

    Timer m_scheduledEventTimer;
    Vector<Ref<Event>> m_scheduledEvents;

    String m_label;
    RTCDataChannelInit m_options;
};

} // namespace WebCore

#endif // ENABLE(WEB_RTC)
