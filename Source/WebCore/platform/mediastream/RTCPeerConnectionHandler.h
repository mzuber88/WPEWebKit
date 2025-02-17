/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Google Inc. nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RTCPeerConnectionHandler_h
#define RTCPeerConnectionHandler_h

#if ENABLE(WEB_RTC)

#include "MediaStreamPrivate.h"
#include <wtf/PassRefPtr.h>

namespace WebCore {

class RealtimeMediaSource;
class RTCConfigurationPrivate;
class RTCDTMFSenderHandler;
class RTCDataChannelHandler;
class RTCIceCandidateDescriptor;
class RTCOfferOptionsPrivate;
class RTCOfferAnswerOptionsPrivate;
class RTCPeerConnectionHandler;
class RTCPeerConnectionHandlerClient;
class RTCSessionDescriptionDescriptor;
class RTCSessionDescriptionRequest;
class RTCStatsRequest;
class RTCVoidRequest;

struct RTCDataChannelInit;

typedef std::unique_ptr<RTCPeerConnectionHandler> (*CreatePeerConnectionHandler)(RTCPeerConnectionHandlerClient*);

class RTCPeerConnectionHandler {
public:
    WEBCORE_EXPORT static CreatePeerConnectionHandler create;
    virtual ~RTCPeerConnectionHandler() { }

    static const AtomicString& incompatibleConstraintsErrorName();
    static const AtomicString& invalidSessionDescriptionErrorName();
    static const AtomicString& incompatibleSessionDescriptionErrorName();
    static const AtomicString& internalErrorName();

    virtual bool initialize(PassRefPtr<RTCConfigurationPrivate>) = 0;

    virtual void createOffer(PassRefPtr<RTCSessionDescriptionRequest>, PassRefPtr<RTCOfferOptionsPrivate>) = 0;
    virtual void createAnswer(PassRefPtr<RTCSessionDescriptionRequest>, PassRefPtr<RTCOfferAnswerOptionsPrivate>) = 0;
    virtual void setLocalDescription(PassRefPtr<RTCVoidRequest>, PassRefPtr<RTCSessionDescriptionDescriptor>) = 0;
    virtual void setRemoteDescription(PassRefPtr<RTCVoidRequest>, PassRefPtr<RTCSessionDescriptionDescriptor>) = 0;
    virtual PassRefPtr<RTCSessionDescriptionDescriptor> localDescription() = 0;
    virtual PassRefPtr<RTCSessionDescriptionDescriptor> remoteDescription() = 0;
    virtual bool updateIce(PassRefPtr<RTCConfigurationPrivate>) = 0;
    virtual bool addIceCandidate(PassRefPtr<RTCVoidRequest>, PassRefPtr<RTCIceCandidateDescriptor>) = 0;
    virtual bool addStream(PassRefPtr<MediaStreamPrivate>) = 0;
    virtual void removeStream(PassRefPtr<MediaStreamPrivate>) = 0;
    virtual void getStats(PassRefPtr<RTCStatsRequest>) = 0;
    virtual std::unique_ptr<RTCDataChannelHandler> createDataChannel(const String& label, const RTCDataChannelInit&) = 0;
    virtual std::unique_ptr<RTCDTMFSenderHandler> createDTMFSender(PassRefPtr<RealtimeMediaSource>) = 0;
    virtual void stop() = 0;
};

} // namespace WebCore

#endif // ENABLE(WEB_RTC)

#endif // RTCPeerConnectionHandler_h
