#ifndef BPLUSSEGMENT_C
#define BPLUSSEGMENT_C

#include "bplussegment.h"

#include <iostream>

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
void BPlusSegment<K,V>::erase(const K &key)
{
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
BPlus_iterator<V> BPlusSegment<K,V>::lookupRange(const K &startKey) const
{
}

template<typename K, typename V>
char *BPlusSegment<K,V>::visualize()
{
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
