/*
 * Copyright (C) 2007, 2008, 2009, 2011 Apple Inc. All rights reserved.
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

#ifndef ComplexTextController_h
#define ComplexTextController_h

#include "GlyphBuffer.h"
#include <wtf/HashSet.h>
#include <wtf/RefCounted.h>
#include <wtf/RetainPtr.h>
#include <wtf/Vector.h>
#include <wtf/text/WTFString.h>

#define USE_LAYOUT_SPECIFIC_ADVANCES ((PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED >= 101100) || (PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000))

typedef unsigned short CGGlyph;

typedef const struct __CTRun * CTRunRef;
typedef const struct __CTLine * CTLineRef;

namespace WTF {
template<> struct VectorTraits<CGPoint> : SimpleClassVectorTraits { };
}

namespace WebCore {

class FontCascade;
class Font;
class TextRun;

enum GlyphIterationStyle { IncludePartialGlyphs, ByWholeGlyphs };

// ComplexTextController is responsible for rendering and measuring glyphs for
// complex scripts on macOS and iOS.
class ComplexTextController {
    WTF_MAKE_FAST_ALLOCATED;
public:
    ComplexTextController(const FontCascade&, const TextRun&, bool mayUseNaturalWritingDirection = false, HashSet<const Font*>* fallbackFonts = 0, bool forTextEmphasis = false);

    // Advance and emit glyphs up to the specified character.
    void advance(unsigned to, GlyphBuffer* = nullptr, GlyphIterationStyle = IncludePartialGlyphs, HashSet<const Font*>* fallbackFonts = nullptr);

    // Compute the character offset for a given x coordinate.
    unsigned offsetForPosition(float x, bool includePartialGlyphs);

    // Returns the width of everything we've consumed so far.
    float runWidthSoFar() const { return m_runWidthSoFar; }

    float totalWidth() const { return m_totalWidth; }

    float minGlyphBoundingBoxX() const { return m_minGlyphBoundingBoxX; }
    float maxGlyphBoundingBoxX() const { return m_maxGlyphBoundingBoxX; }
    float minGlyphBoundingBoxY() const { return m_minGlyphBoundingBoxY; }
    float maxGlyphBoundingBoxY() const { return m_maxGlyphBoundingBoxY; }

    float leadingExpansion() const { return m_leadingExpansion; }
    
private:
    class ComplexTextRun : public RefCounted<ComplexTextRun> {
    public:
        static Ref<ComplexTextRun> create(CTRunRef ctRun, const Font& font, const UChar* characters, unsigned stringLocation, size_t stringLength, CFRange runRange)
        {
            return adoptRef(*new ComplexTextRun(ctRun, font, characters, stringLocation, stringLength, runRange));
        }

        static Ref<ComplexTextRun> create(const Font& font, const UChar* characters, unsigned stringLocation, size_t stringLength, bool ltr)
        {
            return adoptRef(*new ComplexTextRun(font, characters, stringLocation, stringLength, ltr));
        }

        unsigned glyphCount() const { return m_glyphCount; }
        const Font& font() const { return m_font; }
        const UChar* characters() const { return m_characters; }
        unsigned stringLocation() const { return m_stringLocation; }
        size_t stringLength() const { return m_stringLength; }
        ALWAYS_INLINE CFIndex indexAt(size_t i) const;
        CFIndex indexBegin() const { return m_indexBegin; }
        CFIndex indexEnd() const { return m_indexEnd; }
        CFIndex endOffsetAt(size_t i) const { ASSERT(!m_isMonotonic); return m_glyphEndOffsets[i]; }
        const CGGlyph* glyphs() const { return m_glyphs; }

        /*
         *                                              X (Paint glyph position)   X (Paint glyph position)   X (Paint glyph position)
         *                                             7                          7                          7
         *                                            /                          /                          /
         *                                           / (Glyph origin)           / (Glyph origin)           / (Glyph origin)
         *                                          /                          /                          /
         *                                         /                          /                          /
         *                X-----------------------X--------------------------X--------------------------X---->...
         * (text position ^)  (initial advance)          (base advance)             (base advance)
         */
        CGSize initialAdvance() const { return m_initialAdvance; }
        const CGSize* baseAdvances() const { return m_baseAdvances; }
        const CGPoint* glyphOrigins() const { return m_glyphOrigins.size() == glyphCount() ? m_glyphOrigins.data() : nullptr; }
        bool isLTR() const { return m_isLTR; }
        bool isMonotonic() const { return m_isMonotonic; }
        void setIsNonMonotonic();

    private:
        ComplexTextRun(CTRunRef, const Font&, const UChar* characters, unsigned stringLocation, size_t stringLength, CFRange runRange);
        ComplexTextRun(const Font&, const UChar* characters, unsigned stringLocation, size_t stringLength, bool ltr);

        Vector<CGSize, 64> m_baseAdvancesVector;
        Vector<CGPoint, 64> m_glyphOrigins;
        Vector<CGGlyph, 64> m_glyphsVector;
        Vector<CFIndex, 64> m_glyphEndOffsets;
        Vector<CFIndex, 64> m_coreTextIndicesVector;
        CGSize m_initialAdvance;
        const Font& m_font;
        const UChar* m_characters;
        size_t m_stringLength;
        const CFIndex* m_coreTextIndices;
        const CGGlyph* m_glyphs;
        const CGSize* m_baseAdvances;
        CFIndex m_indexBegin;
        CFIndex m_indexEnd;
        unsigned m_glyphCount;
        unsigned m_stringLocation;
        bool m_isLTR;
        bool m_isMonotonic;
    };
    
    static unsigned stringBegin(const ComplexTextRun& run) { return run.stringLocation() + run.indexBegin(); }
    static unsigned stringEnd(const ComplexTextRun& run) { return run.stringLocation() + run.indexEnd(); }

    void collectComplexTextRuns();

    void collectComplexTextRunsForCharacters(const UChar*, unsigned length, unsigned stringLocation, const Font*);
    void adjustGlyphsAndAdvances();

    unsigned indexOfCurrentRun(unsigned& leftmostGlyph);
    unsigned incrementCurrentRun(unsigned& leftmostGlyph);

    float runWidthSoFarFraction(unsigned glyphStartOffset, unsigned glyphEndOffset, unsigned oldCharacterInCurrentGlyph, GlyphIterationStyle) const;

    CGPoint glyphOrigin(unsigned index) const { return index < m_glyphOrigins.size() ? m_glyphOrigins[index] : CGPointZero; }

    Vector<CGSize, 256> m_adjustedBaseAdvances;
    Vector<CGPoint, 256> m_glyphOrigins;
    Vector<CGGlyph, 256> m_adjustedGlyphs;

    Vector<UChar, 256> m_smallCapsBuffer;

    // There is a 3-level hierarchy here. At the top, we are interested in m_run.string(). We partition that string
    // into Lines, each of which is a sequence of characters which should use the same Font. Core Text then partitions
    // the Line into ComplexTextRuns.
    // ComplexTextRun::stringLocation() and ComplexTextRun::stringLength() refer to the offset and length of the Line
    // relative to m_run.string(). ComplexTextRun::indexAt() returns to the offset of a codepoint relative to
    // its Line. ComplexTextRun::glyphs() and ComplexTextRun::advances() refer to glyphs relative to the ComplexTextRun.
    // The length of the entire TextRun is m_run.length()
    Vector<RefPtr<ComplexTextRun>, 16> m_complexTextRuns;

    // The initial capacity of these vectors was selected as being the smallest power of two greater than
    // the average (3.5) plus one standard deviation (7.5) of nonzero sizes used on Arabic Wikipedia.
    Vector<unsigned, 16> m_runIndices;
    Vector<unsigned, 16> m_glyphCountFromStartToIndex;

    Vector<RetainPtr<CTLineRef>> m_coreTextLines;

    Vector<String> m_stringsFor8BitRuns;

    HashSet<const Font*>* m_fallbackFonts { nullptr };

    const FontCascade& m_font;
    const TextRun& m_run;
 
    unsigned m_currentCharacter { 0 };
    unsigned m_end { 0 };

    float m_totalWidth { 0 };
    float m_runWidthSoFar { 0 };
    unsigned m_numGlyphsSoFar { 0 };
    unsigned m_currentRun { 0 };
    unsigned m_glyphInCurrentRun { 0 };
    unsigned m_characterInCurrentGlyph { 0 };
    float m_expansion { 0 };
    float m_expansionPerOpportunity { 0 };
    float m_leadingExpansion { 0 };

    float m_minGlyphBoundingBoxX { std::numeric_limits<float>::max() };
    float m_maxGlyphBoundingBoxX { std::numeric_limits<float>::min() };
    float m_minGlyphBoundingBoxY { std::numeric_limits<float>::max() };
    float m_maxGlyphBoundingBoxY { std::numeric_limits<float>::min() };

    bool m_isLTROnly { true };
    bool m_mayUseNaturalWritingDirection { false };
    bool m_forTextEmphasis { false };
};

} // namespace WebCore

#endif // ComplexTextController_h
