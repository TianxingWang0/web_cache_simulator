#ifndef GQD_CACHE_H
#define GQD_CACHE_H

#include <unordered_map>
#include <list>
#include <random>
#include "cache.h"
#include "cache_object.h"

typedef std::list<QOECacheObject>::iterator QOEListIteratorType;
typedef std::unordered_map<CacheObject, QOEListIteratorType> QOECacheMapType;

/*
  GQD Cache: qoe density for each content as its priority
*/
class GQDCache : public Cache
{
protected:
    // list for qoe density order
    // std::list is a container, usually, implemented as a doubly-linked list 
    std::list<QOECacheObject> _cacheList;
    // map to find objects in list
    QOECacheMapType _cacheMap;

    virtual void hit(QOECacheMapType::const_iterator it, uint64_t size);

public:
    GQDCache()
        : Cache()
    {
    }
    virtual ~GQDCache()
    {
    }

    virtual bool lookup(SimpleRequest* req);
    virtual void admit(SimpleRequest* req) {};
    virtual void admit(SimpleRequest* req, double q);
    virtual void evict(SimpleRequest* req) {};
    virtual void evict();
    virtual void update();
    //virtual void setSize(uint64_t cs) {_cacheSize = cs; _currentSize = 0;};
};

static Factory<GQDCache> factoryGQD("GQD");



#endif