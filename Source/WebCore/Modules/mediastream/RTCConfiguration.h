/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
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

#pragma once

#if ENABLE(WEB_RTC)

#include "ExceptionOr.h"
#include "PeerConnectionStates.h"
#include "RTCIceServer.h"
#include <wtf/Vector.h>

namespace WebCore {

class Dictionary;

// FIXME: Why reference count this?
class RTCConfiguration : public RefCounted<RTCConfiguration> {
public:
    static ExceptionOr<RefPtr<RTCConfiguration>> create(const Dictionary& configuration);

    using IceTransportPolicy = PeerConnectionStates::IceTransportPolicy;
    IceTransportPolicy iceTransportPolicy() const { return m_iceTransportPolicy; }

    using BundlePolicy = PeerConnectionStates::BundlePolicy;
    BundlePolicy bundlePolicy() const { return m_bundlePolicy; }

    Vector<RefPtr<RTCIceServer>> iceServers() const { return m_iceServers; }

private:
    RTCConfiguration();

    ExceptionOr<void> initialize(const Dictionary& configuration);

    Vector<RefPtr<RTCIceServer>> m_iceServers;
    IceTransportPolicy m_iceTransportPolicy { IceTransportPolicy::All };
    BundlePolicy m_bundlePolicy { BundlePolicy::Balanced };
};

} // namespace WebCore

#endif // ENABLE(WEB_RTC)
