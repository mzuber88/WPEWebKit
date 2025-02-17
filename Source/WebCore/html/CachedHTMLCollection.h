/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "CollectionTraversal.h"
#include "HTMLCollection.h"
#include "HTMLElement.h"

namespace WebCore {

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
class CachedHTMLCollection : public HTMLCollection {
public:
    CachedHTMLCollection(ContainerNode& base, CollectionType);

    virtual ~CachedHTMLCollection();

    unsigned length() const final { return m_indexCache.nodeCount(collection()); }
    Element* item(unsigned offset) const override { return m_indexCache.nodeAt(collection(), offset); }
    Element* namedItem(const AtomicString& name) const override;
    size_t memoryCost() const final { return m_indexCache.memoryCost() + HTMLCollection::memoryCost(); }

    // For CollectionIndexCache; do not use elsewhere.
    using CollectionTraversalIterator = typename CollectionTraversal<traversalType>::Iterator;
    CollectionTraversalIterator collectionBegin() const { return CollectionTraversal<traversalType>::begin(collection(), rootNode()); }
    CollectionTraversalIterator collectionLast() const { return CollectionTraversal<traversalType>::last(collection(), rootNode()); }
    CollectionTraversalIterator collectionEnd() const { return CollectionTraversal<traversalType>::end(rootNode()); }
    void collectionTraverseForward(CollectionTraversalIterator& current, unsigned count, unsigned& traversedCount) const { CollectionTraversal<traversalType>::traverseForward(collection(), current, count, traversedCount); }
    void collectionTraverseBackward(CollectionTraversalIterator& current, unsigned count) const { CollectionTraversal<traversalType>::traverseBackward(collection(), current, count); }
    bool collectionCanTraverseBackward() const { return traversalType != CollectionTraversalType::CustomForwardOnly; }
    void willValidateIndexCache() const { document().registerCollection(const_cast<CachedHTMLCollection<HTMLCollectionClass, traversalType>&>(*this)); }

    void invalidateCache(Document&) override;

    bool elementMatches(Element&) const;

private:
    HTMLCollectionClass& collection() { return static_cast<HTMLCollectionClass&>(*this); }
    const HTMLCollectionClass& collection() const { return static_cast<const HTMLCollectionClass&>(*this); }

    mutable CollectionIndexCache<HTMLCollectionClass, CollectionTraversalIterator> m_indexCache;
};

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
CachedHTMLCollection<HTMLCollectionClass, traversalType>::CachedHTMLCollection(ContainerNode& base, CollectionType collectionType)
    : HTMLCollection(base, collectionType)
    , m_indexCache(collection())
{ }

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
CachedHTMLCollection<HTMLCollectionClass, traversalType>::~CachedHTMLCollection()
{
    if (m_indexCache.hasValidCache(collection()))
        document().unregisterCollection(*this);
}

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
void CachedHTMLCollection<HTMLCollectionClass, traversalType>::invalidateCache(Document& document)
{
    HTMLCollection::invalidateCache(document);
    if (m_indexCache.hasValidCache(collection())) {
        document.unregisterCollection(*this);
        m_indexCache.invalidate(collection());
    }
}

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
bool CachedHTMLCollection<HTMLCollectionClass, traversalType>::elementMatches(Element&) const
{
    // We call the elementMatches() method directly on the subclass instead for performance.
    ASSERT_NOT_REACHED();
    return false;
}

static inline bool nameShouldBeVisibleInDocumentAll(HTMLElement& element)
{
    // The document.all collection returns only certain types of elements by name,
    // although it returns any type of element by id.
    return element.hasTagName(HTMLNames::appletTag)
        || element.hasTagName(HTMLNames::embedTag)
        || element.hasTagName(HTMLNames::formTag)
        || element.hasTagName(HTMLNames::imgTag)
        || element.hasTagName(HTMLNames::inputTag)
        || element.hasTagName(HTMLNames::objectTag)
        || element.hasTagName(HTMLNames::selectTag);
}

static inline bool nameShouldBeVisibleInDocumentAll(Element& element)
{
    return is<HTMLElement>(element) && nameShouldBeVisibleInDocumentAll(downcast<HTMLElement>(element));
}

template <typename HTMLCollectionClass, CollectionTraversalType traversalType>
Element* CachedHTMLCollection<HTMLCollectionClass, traversalType>::namedItem(const AtomicString& name) const
{
    // http://msdn.microsoft.com/workshop/author/dhtml/reference/methods/nameditem.asp
    // This method first searches for an object with a matching id
    // attribute. If a match is not found, the method then searches for an
    // object with a matching name attribute, but only on those elements
    // that are allowed a name attribute.

    if (name.isEmpty())
        return nullptr;

    ContainerNode& root = rootNode();
    if (traversalType != CollectionTraversalType::CustomForwardOnly && root.isInTreeScope()) {
        Element* candidate = nullptr;

        TreeScope& treeScope = root.treeScope();
        if (treeScope.hasElementWithId(*name.impl())) {
            if (!treeScope.containsMultipleElementsWithId(name))
                candidate = treeScope.getElementById(name);
        } else if (treeScope.hasElementWithName(*name.impl())) {
            if (!treeScope.containsMultipleElementsWithName(name)) {
                if ((candidate = treeScope.getElementByName(name))) {
                    if (!is<HTMLElement>(*candidate))
                        candidate = nullptr;
                    else if (type() == DocAll && !nameShouldBeVisibleInDocumentAll(*candidate))
                        candidate = nullptr;
                }
            }
        } else
            return nullptr;

        if (candidate && collection().elementMatches(*candidate)) {
            if (traversalType == CollectionTraversalType::ChildrenOnly ? candidate->parentNode() == &root : candidate->isDescendantOf(&root))
                return candidate;
        }
    }

    return namedItemSlow(name);
}

} // namespace WebCore
