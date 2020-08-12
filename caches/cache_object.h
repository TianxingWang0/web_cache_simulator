#ifndef CACHE_HASH_H
#define CACHE_HASH_H

#include "request.h"

// CacheObject is used by caching policies to store a representation of an "object, i.e., the object's id and its size
class CacheObject
{
public:
    IdType id;
    uint64_t size;

    CacheObject(SimpleRequest *req)
        : id(req->getId()),
          size(req->getSize())
    {
    }

    // comparison is based on all three properties
    bool operator==(const CacheObject &rhs) const
    {
        return (rhs.id == id) && (rhs.size == size);
    }
};

class QOECacheObject: public CacheObject
{
public:
    double qoe;
    uint32_t timer;
    static uint32_t max_time;
    uint32_t freq;


    QOECacheObject(SimpleRequest *req, double q):CacheObject(req)
    {
        qoe = q;
        timer = 1;
    }

    void hit() {
        timer = 1;
        freq++;
    }

    void update() {   
        if (timer != max_time) {
            timer++;
        }
        else {
            freq = 1;
        }
    }

    double getQOE() {
        if (timer == max_time) {
            return 0;
        }
        return qoe * freq / timer;
    }
};

// forward definition of extendable hash
template <class T> void hash_combine(std::size_t &seed, const T &v);

// definition of a hash function on CacheObjects
// required to use unordered_map<CacheObject, >
namespace std {
template <> struct hash<CacheObject> {
  inline size_t operator()(const CacheObject cobj) const {
    size_t seed = 0;
    hash_combine<IdType>(seed, cobj.id);
    hash_combine<uint64_t>(seed, cobj.size);
    return seed;
  }
};
} // namespace std

// hash_combine derived from boost/functional/hash/hash.hpp:212
// Copyright 2005-2014 Daniel James.
// Distributed under the Boost Software License, Version 1.0.
// (See http://www.boost.org/LICENSE_1_0.txt)
template <class T> inline void hash_combine(std::size_t &seed, const T &v) {
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


#endif /* CACHE_HASH_H */
