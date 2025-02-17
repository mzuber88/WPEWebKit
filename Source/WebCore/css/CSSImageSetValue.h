/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include "CSSValueList.h"
#include "CachedImageClient.h"
#include "CachedResourceHandle.h"

namespace WebCore {

class CachedResourceLoader;
class Document;
struct ResourceLoaderOptions;

class CSSImageSetValue final : public CSSValueList {
public:
    static Ref<CSSImageSetValue> create()
    {
        return adoptRef(*new CSSImageSetValue());
    }
    ~CSSImageSetValue();

    std::pair<CachedImage*, float>  loadBestFitImage(CachedResourceLoader&, const ResourceLoaderOptions&);
    CachedImage* cachedImage() const { return m_cachedImage.get(); }

    String customCSSText() const;

    struct ImageWithScale {
        String imageURL;
        float scaleFactor;
    };

    bool traverseSubresources(const std::function<bool (const CachedResource&)>& handler) const;

    Ref<CSSImageSetValue> cloneForCSSOM() const;

    void updateDeviceScaleFactor(const Document&);

protected:
    ImageWithScale bestImageForScaleFactor();

private:
    CSSImageSetValue();
    CSSImageSetValue(const CSSImageSetValue& cloneFrom);

    void fillImageSet();
    static inline bool compareByScaleFactor(ImageWithScale first, ImageWithScale second) { return first.scaleFactor < second.scaleFactor; }

    CachedResourceHandle<CachedImage> m_cachedImage;
    bool m_accessedBestFitImage { false };
    float m_bestFitImageScaleFactor { 1 };
    float m_deviceScaleFactor { 1 };

    Vector<ImageWithScale> m_imagesInSet;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_CSS_VALUE(CSSImageSetValue, isImageSetValue())
