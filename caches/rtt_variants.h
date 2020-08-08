#ifndef RTT_VARIANTS_H
#define RTT_VARIANTS_H

#include <unordered_map>
#include <list>
#include <set>
#include <random>
#include "cache.h"
#include "cache_object.h"
#include "gd_variants.h"
#include "lru_variants.h"
#include "gqd_cache.h"
#include "../consistent_hash/consistent_hash.h"
#include "../matrix.h"
#include "../double_queue_node/double_queue_node.h"
#include <chrono>

/*
    this file defines a distributed cache system which directs requests based on its RTT.
    consider rtt of 
        1. clients between caches 
        2. caches between origins
    QOE infocomm paper experiment: fit different single caches into this system 
*/

/*
    rtt gqd platform
*/

class RTT_GQD_Cache : public Cache
{
protected:
    std::string rtt_file; // the rtt between clients, caches and origins
    int client_number = 0;
    int cache_number = 0;
    int origin_number = 0;
    uint16_t **clients2caches; // [client index][caches index] is the rtt time between two ends
    uint16_t **caches2origins;  // [caches index][origin index] is the rtt time between two ends
    uint16_t **redirect_table; // [client index][origin index] the optimal cache index
                               // for request from `client index` stored on `origin index`
    uint32_t timer_max_time;
    double sum_QoE = 0;

    double A = -1.0 / 198;
    double B = 10000.0 / 99;

private:
    GQDCache *caches_list; // Caches cluster

public:
    RTT_GQD_Cache() : Cache() {}

    virtual ~RTT_GQD_Cache() {}

    virtual void setPar(std::string parName, std::string parValue);
    void init();
    virtual bool lookup(SimpleRequest *req) { return false; };
    virtual void admit(SimpleRequest *req){};
    virtual void evict(SimpleRequest *req){};
    virtual void evict(){};
    virtual void update();
    virtual double rtt2qoe(uint32_t rtt);
    virtual double get_sum_QoE() {return sum_QoE;};
    bool request(SimpleRequest *req, uint8_t client, uint8_t origin);
    //virtual void setSize(uint64_t cs) {_cacheSize = cs; _currentSize = 0;};
};

static Factory<RTT_GQD_Cache> factoryRTT_GQD("RTT_GQD");

class RTT_LRU_Cache : public RTT_GQD_Cache
{
protected:
    LRUCache *_caches_list;

public:
    virtual void setPar(std::string parName, std::string parValue);
    virtual void update() {};
    bool request(SimpleRequest *req, uint8_t client, uint8_t origin);
};

static Factory<RTT_LRU_Cache> factoryRTT_LRU("RTT_LRU");


class RTT_AptSize_Cache : public RTT_GQD_Cache
{
protected:
    AdaptSizeCache *_caches_list;

public:
    virtual void setPar(std::string parName, std::string parValue);
    virtual void update() {};
    bool request(SimpleRequest *req, uint8_t client, uint8_t origin);
};

static Factory<RTT_AptSize_Cache> factoryRTT_AptSize("RTT_AptSize");




#endif