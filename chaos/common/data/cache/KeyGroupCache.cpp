/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */

#include <chaos/common/data/cache/KeyGroupCache.h>

using namespace chaos::common::utility;
using namespace chaos::common::data::cache;
using namespace std;
KeyGroupCache::KeyGroupCache() {
    chCachePtrArray = NULL;
}

KeyGroupCache::~KeyGroupCache() {
    
    
        //delete all channel cache creation
    if(chCachePtrArray) {
            //cicle all channel cache element of the array
        for (int idx = 0; idx < channelInfoMap.size(); idx++) {
                //delete channel cache
            delete(chCachePtrArray[idx]);
        }
        
            //delete empty cache array
        delete[] chCachePtrArray;
    }
    
        //deallocate map info
    for (std::map<std::string, ChannelInfo*>::iterator iter = channelInfoMap.begin();
         iter != channelInfoMap.end();
         iter++) {
            //delete the info
        delete(iter->second);
    }
    channelInfoMap.clear();
}

    //---------------------------------------------------------------

    //---------------------------------------------------------------

void KeyGroupCache::init(void* initParam)  {
    LFDataCache *tmpLFDataCache = NULL;
    
        //configure the cache using the maximum and minimum channel size
    memoryPool.init(minimumChannelSize+(uint32_t)sizeof(SlbCachedInfo), maximumChannelSize+(uint32_t)sizeof(SlbCachedInfo), 0, 1.2, 0, 1000);
    
        //allocate space for channel cache array
    chCachePtrArray = new LFDataCache*[channelInfoMap.size()];
    if(!chCachePtrArray)
        throw CException(1, "Error allocating channel cache array","KeyGroupCache::init");
    
        //clear all memory
    memset(chCachePtrArray, 0, sizeof(LFDataCache*)*channelInfoMap.size());
    
        //allocate alla channel cache
    for (std::map<std::string, ChannelInfo*>::iterator iter = channelInfoMap.begin();
         iter != channelInfoMap.end();
         iter++) {
        
        tmpLFDataCache = new LFDataCache(&memoryPool);
        if(!tmpLFDataCache) throw CException(2, "Error allocating channel cache","KeyGroupCache::init");
        
            //inizialize channel cache
        tmpLFDataCache->init(iter->first.c_str(), iter->second->maxLength);
        
            //allocate the channel cache array into his index
        chCachePtrArray[iter->second->index] = tmpLFDataCache;
    }
}


    //---------------------------------------------------------------

    //---------------------------------------------------------------
void KeyGroupCache::start()  {
}

    //---------------------------------------------------------------

    //---------------------------------------------------------------
void KeyGroupCache::stop()  {
}

    //---------------------------------------------------------------

    //---------------------------------------------------------------
void KeyGroupCache::deinit()  {
    memoryPool.deinit();
}

//---------------------------------------------------------------

//---------------------------------------------------------------
void KeyGroupCache::garbageCache() {
    for (std::map<std::string, ChannelInfo*>::iterator iter = channelInfoMap.begin();
         iter != channelInfoMap.end();
         iter++) {
        
        //allocate the channel cache array into his index
        chCachePtrArray[iter->second->index]->garbageCache();
    }
}
    //---------------------------------------------------------------

    //---------------------------------------------------------------
int KeyGroupCache::addKeyInfo(const char * key, chaos::DataType::DataType type, uint32_t channelMaxLength) {
    int err = 0;
        //check if we can receive other new channel
    if(getServiceState() != CUStateKey::DEINIT)
        throw CException(1, "Operation not permited, the cache is not initialized","KeyGroupCache::addChannel");
    
        //check if is already present into the hash
    if(channelInfoMap.count(key)) return 1;
    
        // insert a new channel
    ChannelInfo *chInfo = new ChannelInfo();
    if(!chInfo)
        throw CException(2, "Channel infor creation error","KeyGroupCache::addChannel");
    
    chInfo->index = (uint8_t)channelInfoMap.size();
    switch (chInfo->type = type) {
        case chaos::DataType::TYPE_BOOLEAN:
            chInfo->maxLength = sizeof(char);
            break;
        case chaos::DataType::TYPE_DOUBLE:
            chInfo->maxLength = sizeof(double);
            break;
        case chaos::DataType::TYPE_INT32:
            chInfo->maxLength = sizeof(int32_t);
            break;
        case chaos::DataType::TYPE_INT64:
            chInfo->maxLength = sizeof(int64_t);
            break;
        case chaos::DataType::TYPE_CLUSTER:
        case chaos::DataType::TYPE_STRING:
        case chaos::DataType::TYPE_BYTEARRAY:
            chInfo->maxLength = channelMaxLength;
            break;
        default:
            break;
    }

    channelInfoMap.insert(make_pair(key, chInfo));
    
        //check the size and memorize the msximum and minimum size
    maximumChannelSize = std::max(chInfo->maxLength, maximumChannelSize);
    minimumChannelSize = std::min(chInfo->maxLength, (minimumChannelSize==0)?maximumChannelSize:minimumChannelSize);
    return err;
}

void KeyGroupCache::updateKeyValue(const char *key, const void* value, uint32_t legth) {
    if(!channelInfoMap.count(key)) return;
    updateKeyValue(channelInfoMap[key]->index, value, legth);
}

void KeyGroupCache::updateKeyValue(uint16_t keyIndex, const void* value, uint32_t legth) {
    LFDataCache *ptr = chCachePtrArray[keyIndex];
    if(!ptr) return;
    ptr->updateValue(value, legth);
}

SlbCachedInfoPtr KeyGroupCache::getCurrentKeyValue(const char *key) {
    if(!channelInfoMap.count(key)) return NULL;
    return getCurrentKeyValue(channelInfoMap[key]->index);
}

SlbCachedInfoPtr KeyGroupCache::getCurrentKeyValue(uint16_t keyIndex) {
    LFDataCache *ptr = chCachePtrArray[keyIndex];
    if(!ptr) return NULL;
    return ptr->getCurrentCachedPtr();
}

void KeyGroupCache::getCurrentKeyAccessor(const char *key, ChannelValueAccessor& accessorPtr) {
    if(!channelInfoMap.count(key)) {
        accessorPtr.reset(NULL);
        return;
    }
    getCurrentKeyAccessor(channelInfoMap[key]->index, accessorPtr);
}

void KeyGroupCache::getCurrentKeyAccessor(uint16_t keyIndex, ChannelValueAccessor& accessorPtr) {
    LFDataCache *ptr = chCachePtrArray[keyIndex];
    if(!ptr) {
        accessorPtr.reset(NULL);
        return;
    }
    ptr->fillAccessorWithCurrentValue(accessorPtr);
}
