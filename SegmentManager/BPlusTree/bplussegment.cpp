#ifndef BPLUSSEGMENT_C
#define BPLUSSEGMENT_C

#include "bplussegment.h"

#include <iostream>
#include <set>

template<typename K, typename V>
BPlus_iterator<K, V>::BPlus_iterator(BufferManager &bm, uint64_t segID, PageID pageID, uint64_t offset)
    : bm(bm),
      segID(segID),
      pageID(pageID),
      offset(offset) {}

template<typename K, typename V>
BPlus_iterator<K, V>::BPlus_iterator(const BPlus_iterator& other)
    : bm(other.bm),
      segID(other.segID),
      pageID(other.pageID),
      offset(other.offset) {}

template<typename K, typename V>
const V& BPlus_iterator<K, V>::operator*() const {
    if (pageID == 0) {
        throw NotFoundException();
    }
    BufferFrame& frame = bm.fixPage(segID, pageID, false);
    BPlusPage<K, PageID> page(frame.getData(), PAGESIZE, nullptr);
    V retVal = page.getValue(offset);
    bm.unfixPage(frame, false);
    return retVal;
}

template<typename K, typename V>
BPlus_iterator<K, V>& BPlus_iterator<K, V>::operator++() {
    offset++;
    BufferFrame& frame = bm.fixPage(segID, pageID, false);
    BPlusPage<K, PageID> page(frame.getData(), PAGESIZE, nullptr);
    if (offset >= page.header->count) {
        pageID = page.header->next;
        offset = 0;
    }
    bm.unfixPage(frame, false);
    return *this;
}

template<typename K, typename V>
BPlus_iterator<K, V> BPlus_iterator<K, V>::operator++(int) {
    BPlus_iterator tmp(*this);
    operator ++();
    return tmp;
}

template<typename K, typename V>
bool BPlus_iterator<K, V>::operator==(const BPlus_iterator& rhs) const {
    return pageID == rhs.pageID && offset == rhs.offset;
}

template<typename K, typename V>
bool BPlus_iterator<K, V>::operator!=(const BPlus_iterator& rhs) const {
    return pageID != rhs.pageID || offset != rhs.offset;
}

template<typename K, typename V>
BPlusSegment<K,V>::BPlusSegment(bool (*comparator)(const K&, const K&),
                                BufferManager& bufman,
                                SchemaManager& schemaManager,
                                uint64_t segId,
                                uint64_t pgcount,
                                uint32_t ps,
                                uint64_t rootPage)
    : Segment(schemaManager, segId, pgcount, ps),
      cmp(comparator),
      root{rootPage},
      bm(bufman)
{
    initialize();
}

template<typename K, typename V>
void BPlusSegment<K,V>::insert(const K &key, const V &value)
{
    //TODO: (optional) locate without X lock
    //locate page
    try{
        BufferFrame& targetPage = *fixLeafFor(key, true);

        BPlusPage<K,V> bpp(targetPage.getData(), pageSize, cmp);

        //check if insert possible
        if (bpp.hasAdditionalSpace()){
            bpp.insert(key, value);
            bm.unfixPage(targetPage, true);
            return;
        }
        bm.unfixPage(targetPage, false);

    }catch(NotFoundException e){
        //couldn't locate page because all keys are bigger than given key and upper
        //doesn't exist, thus we continue with insertion case
    }

    insertAndSplit(key, value);
}



template<typename K, typename V>
void BPlusSegment<K,V>::erase(const K &key) {
    BufferFrame& targetFrame = *fixLeafFor(key, true);
    BPlusPage<K,V> bpp(targetFrame.getData(), pageSize, cmp);
    bool removed = bpp.remove(key);
    bm.unfixPage(targetFrame, removed);
}

template<typename K, typename V>
V BPlusSegment<K,V>::lookup(const K &key) const
{
    BufferFrame& targetPage = *fixLeafFor(key, false);

    BPlusPage<K,V> bpp(targetPage.getData(), pageSize, cmp);
    V value = bpp.lookup(key);
    bm.unfixPage(targetPage, false);

    return value;
}

template<typename K, typename V>
void BPlusSegment<K,V>::insertAndSplit(const K& key, const V& value){
    SplitResult<K> result = insertAndSplit(key, value, root);

    //update root if root has split
    if (result.hasSplit){
        //TODO: handle upper
        PageID newRootId = pageCount;
        BufferFrame& newRootFrame = bm.fixPage(segmentId, newRootId, true);
        pageCount++;
        BPlusPage<K, PageID> newRoot(newRootFrame.getData(), pageSize, cmp);      
        newRoot.initialize();
        newRoot.setLeaf(false);

        newRoot.insert(result.pageHighestKey, root);
        newRoot.insert(result.siblingHighestKey, result.siblingPageID);
        root = newRootId;
        bm.unfixPage(newRootFrame, true);
    }


    bm.unfixPage(result.pageFrame, true);
    scm.segmentResized(*this);
}


template<typename K, typename V>
SplitResult<K> BPlusSegment<K,V>::insertAndSplit(const K& key, const V& value, const PageID pageID){

    BufferFrame& frame = bm.fixPage(segmentId, pageID, true);

    if (isLeaf(frame.getData())){
        BPlusPage<K, V> page(frame.getData(), pageSize, cmp);

        if (page.hasAdditionalSpace()){
            //Page still has space, this is the case when we used this method in order to propagate
            //growing parent link

            page.insert(key, value);

            return SplitResult<K>(false, frame);

        }else{

            //split the leaf
            SplitResult<K> result = splitPage(frame, false);

            //insert key
            BufferFrame* lowerSiblingFrame;
            bool needsUnfix = false;
            if (cmp(key, result.pageHighestKey)){
                lowerSiblingFrame = &frame;
            }else{
                lowerSiblingFrame = &bm.fixPage(segmentId, result.siblingPageID, true);
                needsUnfix = true;
            }

            BPlusPage<K, V> page(lowerSiblingFrame->getData(), pageSize, cmp);
            page.insert(key, value);

            if (needsUnfix){
                bm.unfixPage(*lowerSiblingFrame, true);
            }

            return result;
        }

    //or locate the page we need to go to
    }else{
        bool highestKeyNeedsUpdate = false;
        bool traversingUpper = false;
        BPlusPage<K, PageID> page(frame.getData(), pageSize, cmp);
        PageID nextPage;
        K usedKey;

        try{
            LookupResult<K, PageID> res = page.lookupSmallestGreaterThan(key);
            nextPage = res.value;
            usedKey = res.key;
        }catch(NotFoundException e){
            if(page.getUpperExists()){
                //insert in upper if it exists
                nextPage = page.getUpper();
                traversingUpper = true;
            }else{
                //insert in highest and increase the corresponding
                //key if page is not yet full
                nextPage = page.getValueOfHighestKey();
                usedKey = page.getHighestKey();
                highestKeyNeedsUpdate = true;
            }
        }

        SplitResult<K> result = insertAndSplit(key, value, nextPage);

        if (result.hasSplit){
            //if the lower page was split: update the current page and insert the new sibling
            if (page.hasAdditionalSpace()){
                //update original entry
                //TODO: key
                page.update(usedKey, result.pageHighestKey);
                //insert sibling
                page.insert(result.siblingHighestKey, result.siblingPageID);

                //unfix frame from result
                bm.unfixPage(result.pageFrame, true);

                return SplitResult<K>(false, frame);

            }else if (!page.getUpperExists()){
                //there is no space for a key, value pair, but we can put the child in upper
                page.setUpper(result.siblingPageID);
                
                //unfix frame from result
                bm.unfixPage(result.pageFrame, true);

                return SplitResult<K>(false, frame);
            }else{
                //must split current page to accomodate sibling

                if (! traversingUpper){
                    //update original entry with new lower max
                    page.update(key, result.pageHighestKey);
                }

                //split self to make space for sibling
                SplitResult<K> selfSplit = splitPage(frame, true);

                //insert sibling
                BufferFrame* lowerSiblingFrame;
                bool needsUnfix = false;
                if (cmp(result.siblingHighestKey, selfSplit.pageHighestKey)){
                    lowerSiblingFrame = &frame;
                }else{
                    lowerSiblingFrame = &bm.fixPage(segmentId, selfSplit.siblingPageID, true);
                    needsUnfix = true;
                }
                BPlusPage<K, PageID> putLowerSibling(lowerSiblingFrame->getData(), pageSize, cmp);
                putLowerSibling.insert(result.siblingHighestKey, result.siblingPageID);

                if (needsUnfix){
                    bm.unfixPage(*lowerSiblingFrame, true);
                }
                bm.unfixPage(result.pageFrame, true);
                return selfSplit;
            }

        }else{
            //child has not split, insert was successful. Not much to be done
            bm.unfixPage(result.pageFrame, true);

            if (highestKeyNeedsUpdate){
                page.update(page.getHighestKey(), key);
            }

            return SplitResult<K>(false, frame);
        }
    }
}

template<typename K, typename V>
SplitResult<K> BPlusSegment<K,V>::splitPage(BufferFrame &frame, const bool inner){


    //TODO: find a better way to work with the types here
    if (inner){
        //notice: K, PageID here instead of V
        BPlusPage<K, PageID> page(frame.getData(), pageSize, cmp);
        assert(!page.hasAdditionalSpace());

        PageID siblingPageID = pageCount;
        BufferFrame& siblingFrame = bm.fixPage(segmentId, siblingPageID, true);
        pageCount++;
        //notice: K, PageID here instead of V
        BPlusPage<K, PageID> sibling(siblingFrame.getData(), pageSize, cmp);
        sibling.initialize();
        sibling.setLeaf(false);
        sibling.takeUpperFrom(page);

        //handle upper key on split
        if (page.getUpperExists()){
            K newKey = findGreatestKey(&siblingFrame);
            sibling.insert(newKey, page.getUpper());

            page.setUpperNotExists();
        }

        K siblingHighestKey = sibling.getHighestKey();
        K pageHighestKey = page.getHighestKey();
        bm.unfixPage(siblingFrame, true);
        return SplitResult<K>(siblingPageID, siblingHighestKey, pageHighestKey, true, frame);
    }else{
        BPlusPage<K, V> page(frame.getData(), pageSize, cmp);
        assert(!page.hasAdditionalSpace());

        PageID siblingPageID = pageCount;
        BufferFrame& siblingFrame = bm.fixPage(segmentId, siblingPageID, true);
        pageCount++;
        BPlusPage<K, V> sibling(siblingFrame.getData(), pageSize, cmp);
        sibling.initialize();
        sibling.takeUpperFrom(page);
        K siblingHighestKey = sibling.getHighestKey();
        K pageHighestKey = page.getHighestKey();
        bm.unfixPage(siblingFrame, true);
        return SplitResult<K>(siblingPageID, siblingHighestKey, pageHighestKey, true, frame);
    }

}

template<typename K, typename V>
V BPlusSegment<K, V>::findGreatestKey(BufferFrame* startFrame) const{

    BufferFrame* pageFrame = startFrame;
    bool isFirst = true;

    while(!isLeaf(pageFrame->getData())){
        BPlusPage<K, PageID> page(pageFrame->getData(), pageSize, cmp);
        PageID nextPage;
        if (page.getUpperExists()){
            nextPage = page.getUpper();
        }else{
            nextPage = page.getValueOfHighestKey();
        }

        BufferFrame* oldBF = pageFrame;
        pageFrame = &bm.fixPage(segmentId, nextPage, false);
        //don't unfix first frame, as it is controlled by caller
        if (isFirst){
            isFirst = false;
        }else{
            bm.unfixPage(*oldBF, false);
        }
    }

    BPlusPage<K, V> page(pageFrame->getData(), pageSize, cmp);

    K highestKey = page.getHighestKey();

    if (!isFirst) bm.unfixPage(*pageFrame, false);

    return highestKey;
}

template<typename K, typename V>
BufferFrame* BPlusSegment<K,V>::fixLeafFor(const K& key, const bool exclusive) const{

    BufferFrame* pageOfKey = &bm.fixPage(segmentId, root, exclusive);

    while(!isLeaf(pageOfKey->getData())){
        BPlusPage<K, PageID> page(pageOfKey->getData(), pageSize, cmp);
        PageID nextPage;
        try{
             nextPage = page.lookupSmallestGreaterThan(key).value;
        }catch(NotFoundException e){
            //upper exists?
            if (page.getUpperExists()){
                nextPage = page.getUpper();
            }else{
                bm.unfixPage(*pageOfKey, false);
                throw NotFoundException();
            }
        }
        BufferFrame* oldBF = pageOfKey;
        pageOfKey = &bm.fixPage(segmentId, nextPage, exclusive);
        bm.unfixPage(*oldBF, false);
    }


    return pageOfKey;
}

template<typename K, typename V>
bool BPlusSegment<K,V>::isLeaf(const void* data) const{
    return static_cast<const BPlusHeader*>(data)->isLeaf();
}

template<typename K, typename V>
BPlus_iterator<K, V> BPlusSegment<K,V>::lookupRange(const K &startKey) const {
    //TODO: manage NotFoundException
    BufferFrame& targetFrame = *fixLeafFor(startKey, false);
    BPlusPage<K, PageID> page(targetFrame.getData(), pageSize, cmp);
    uint64_t targetOffset = page.getPositionFor(startKey);
    PageID targetPageID = targetFrame.getMappedPageId();
    bm.unfixPage(targetFrame, false);

    return BPlus_iterator<K,V>(bm, segmentId, targetPageID, targetOffset);
}

template<typename K, typename V>
void BPlusSegment<K, V>::visualize(std::ostream& output) const {
    output << "digraph exportedBTree {" << endl;
    uint64_t curPageCount = this->pageCount;
    vector<vector<PageID>> links(curPageCount);
    vector<PageID> pages;
    pages.reserve(curPageCount);
    pages.push_back(this->root);
    // output all the pages, collecting links
    for (int i=0; i<curPageCount; i++) {
        visualizePage(output, pages[i], links[i]);
        for (auto it = links[i].begin(); it != links[i].end(); ++it) {
            auto it2 = pages.begin();
            while (it2 != pages.end() && *it2 != *it) {
                it2++;
            }
            if (it2 == pages.end()) {
                pages.push_back(*it);
            }
        }
    }
    // output links
    for (int i=0; i<pages.size(); i++) {
        PageID pageIDFrom = pages[i];
        if (links[i].size() > 1) {
            for (int k=0; k<(links[i].size()-1); k++) {
                output << "node" << pageIDFrom << ":ptr" << k << " -> node" << links[i][k] << ":count" << endl;
            }
        } else if (links[i].size() == 1) {
            output << "node" << pageIDFrom << ":next" << " -> node" << links[i][0] << ":count" << endl;
        }
    }
    output << "}" << endl;
}

template<typename K, typename V>
void BPlusSegment<K, V>::visualizePage(ostream& output, const PageID pageID, vector<PageID> &pageLinks) const {
    BufferFrame& frame = bm.fixPage(segmentId, pageID, false);
    BPlusPage<K, PageID> page(frame.getData(), pageSize, cmp);
    output << "node" << pageID << " [shape=record, label= \"";
    page.visualizePage(output, pageLinks);
    output << "\"];" << endl;
    bm.unfixPage(frame, false);
}

template<typename K, typename V>
void BPlusSegment<K,V>::initialize(){

    if (pageCount == 0){
        BufferFrame& frame = bm.fixPage(segmentId, 0, true);
        pageCount = 1;
        root = 0;
        scm.segmentResized(*this);
        BPlusPage<K, V> page(frame.getData(), pageSize, cmp);
        page.initialize();
        page.setLeaf(true);

        bm.unfixPage(frame, true);
    }
}

#endif
