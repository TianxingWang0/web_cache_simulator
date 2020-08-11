#include <fstream>
#include <unordered_map>
#include <limits>
#include <cmath>
#include <cassert>
#include <cmath>
#include <cassert>
#include "lru_variants.h"
#include "../random_helper.h"
#include "gqd_cache.h"

/*
  GQD Cache: qoe density for each content as its priority
*/
bool GQDCache::lookup(SimpleRequest *req)
{
    // CacheObject: defined in cache_object.h
    CacheObject obj(req);
    // _cacheMap defined in class GQDCache in lru_variants.h
    auto it = _cacheMap.find(obj);
    if (it != _cacheMap.end())
    {
        // log hit
        LOG("h", 0, obj.id, obj.size);
        hit(it, obj.size);
        return true;
    }
    return false;
}

void GQDCache::admit(SimpleRequest *req, double q)
{
    const uint64_t size = req->getSize();
    // object feasible to store?
    if (size > _cacheSize)
    {
        LOG("L", _cacheSize, req->getId(), size);
        return;
    }
    if (_currentSize + size > _cacheSize) {
        double spare_size = 0.0;
        QOEListIteratorType lit = _cacheList.end();
        lit--;
        while (lit->getQOE() <= q && spare_size < size) {
            spare_size += lit->size;
            lit--;
        }
        if (spare_size < size)
            return;
    }
    // check eviction needed
    while (_currentSize + size > _cacheSize)
    {
        evict();
    }
    // admit new object
    QOECacheObject obj(req, q);
    CacheObject _obj(req);
    if (_cacheMap.empty())
    {
        _cacheList.push_back(obj);
        _cacheMap[_obj] = _cacheList.begin();
        _currentSize += size;
        LOG("a", _currentSize, obj.id, obj.size);
    }
    else
    {
        for (QOEListIteratorType pos = _cacheList.begin(); pos != _cacheList.end(); pos++)
        {
            if (pos->getQOE() <= q)
            {
                _cacheList.insert(pos, obj);
                pos--;
                _cacheMap[_obj] = pos;
                _currentSize += size;
                LOG("a", _currentSize, obj.id, obj.size);
                break;
            }
        }
    }
}

void GQDCache::evict()
{
    // evict least popular (i.e. last element)
    if (_cacheList.size() > 0)
    {
        QOEListIteratorType lit = _cacheList.end();
        lit--;
        CacheObject obj = *lit;
        LOG("e", _currentSize, obj.id, obj.size);
        // SimpleRequest* req = new SimpleRequest(obj.id, obj.size);
        _currentSize -= obj.size;
        _cacheMap.erase(obj);
        _cacheList.erase(lit);
    }
}

// const_iterator: a forward iterator to const value_type, where
// value_type is pair<const key_type, mapped_type>
void GQDCache::hit(QOECacheMapType::const_iterator it, uint64_t size)
{
    it->second->reset_timer();
    double target_qoe = it->second->qoe;
    for (QOEListIteratorType pos = _cacheList.begin(); pos != it->second; pos++)
    {
        if (pos->getQOE() <= target_qoe)
        {
            _cacheList.splice(pos, _cacheList, it->second);
            return;
        }
    }
}

void GQDCache::update()
{
    for (auto it : _cacheList)
    {
        it.update();
    }
}
