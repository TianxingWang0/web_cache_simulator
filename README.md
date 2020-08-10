# web cache simulatior:
## A simulator for CDN caching and web caching policies.

This project is forked from [dasebe/webcachesim](https://github.com/dasebe/webcachesim), [Daniel S. Berger](https://www.cs.cmu.edu/~dberger1/)

Simulate a variety of existing caching policies by replaying request traces efficiently, easy to add new ones into this framework.

## Compiling 

GCC 4.8.1 upwards (with -std=c++11). Ubuntu 18.04 is recommended.

The Makefile is offered

    make


## Using an exisiting policy

The basic interface is

    ./webcachesim traceFile cacheType cacheSize [cacheParams]

where

 - traceFile: a request trace (see below)
 - cacheType: one of the caching policies (see below)
 - cacheSize: the cache capacity in bytes
 - cacheParams: optional cache parameters, can be used to tune cache policies (see below)

### Request trace format (Change Attention)

Request traces must be given in a space-separated format with **TWO** colums

- id should be a long long int, used to uniquely identify objects
- size should be a long long int, this is object's size in bytes

|  id | size |
| --- | ---- |
|  1  |  120 |
|  2  |   64 |
|  1  |  120 |
|  3  |  14  |
|  1 |  120 |

Example trace in file "test.tr".

### Content Size and Cache Size

The last column in trace file is the content size, now can run experiment in 2 ways : use real size in trace file and set the cache size as the real memory size in bytes, or regard all size as 1 and set the cache size as the maximun number of contents to cache. Set the param as `filesize=true` to use real content size, default as not use content size.

### Available single caching policies

There are currently ten caching policies. This section describes each one, in turn, its parameters, and how to run it on the "test.tr" example trace with cache size 1000 Bytes.

#### Example usage

    ./webcachesim trace_file_path cache_policy [paramName=paramValue]

#### LRU

does: least-recently used eviction

params: none

example usage:

    ./webcachesim test.tr LRU 1000

or use real content size:

    ./webcachesim test.tr LRU 1000 filesize=true
     
#### FIFO

does: first-in first-out eviction

params: none

example usage:

    ./webcachesim test.tr FIFO 1000
    
#### GDS

does: greedy dual size eviction

params: none

example usage:

    ./webcachesim test.tr GDS 1000
    
#### GDSF

does: greedy dual-size frequency eviction

params: none

example usage:

    ./webcachesim test.tr GDSF 1000
    
#### LFU-DA

does: least-frequently used eviction with dynamic aging

params: none

example usage:

    ./webcachesim test.tr LFUDA 1000
    
    
#### Filter-LRU

does: LRU eviction + admit only after N requests

**params**: `n` - admit after n requests)

example usage (admit after 10 requests):

    ./webcachesim test.tr Filter 1000 n=10
    
#### Threshold-LRU

does: LRU eviction + admit only after N requests

**params**: `t` - the size threshold in log form (base 2)

example usage (admit only objects smaller than 512KB):

    ./webcachesim test.tr ThLRU 1000 t=19
    
#### ExpProb-LRU

does: LRU eviction + admit with probability exponentially decreasing with object size

**params**: `c` - the size which has a 50% chance of being admitted (used to determine the exponential family)

example usage (admit objects with size 256KB with about 50% probability):

    ./webcachesim test.tr ExpLRU 1000 c=18
  
#### LRU-K

does: evict object which has oldest K-th reference in the past

**params**: `k` - eviction based on k-th reference in the past

example usage (each segment gets half the capacity)

    ./webcachesim test.tr LRUK 1000 k=4

#### AdaptSize (version 0.1)

does: uses adaptive ExpLRU (ExpProb-LRU) policy that adapts with request traffic, [adapted from the official implementation](https://github.com/dasebe/AdaptSize)

**params**: `t` - reconfiguration interval (default 500K), `i` - numeric iteration (precision, default 15)

example usage

    ./webcachesim test.tr AdaptSize 1000 t=1000000 i=5

## Distributed cache policy

#### Consistent hash
**params**: `n` - the number of caches in cluster, `vnode` - the number of virtual nodes for each server 

    ./webcachesim test.tr CH 1000 n=4 vnode=40

#### Shuffler Matrix
**params**: `n` - the number of caches in cluster

    ./webcachesim test.tr SFM 1000 n=4 vnode=40 alpha=5 W=10000 t=1000



## RTT Platform QoE Bench
the platform that measure the **QoE** for each request
**format for rtt file:**
    RTT platform, namely the platform that consider `round-trip time` as a metrics to evaluate the performance of a cache system. There are many `client`, `cache`, `origin` at different geographic location, we encode each of them in `0, 1, 2...`. We use two two-dimension matrices to discribe:`client2cache[client_number][cache_number]` and `cache2origin[cache_number][origin_number]`. So that `client2cache[i][j]` is the RTT between `client_i` and `cache_j`, `cache2origin[i][j]` is the RTT between `cache_i` and `origin_j`. Please refer to the example file `rtt_file.txt` at the root fold. **The time is in milliseconds.**
**trace file:**
In the trace file, each line present one request, contains 4 part:

    client_index<space>content_id<space>content_size<space>origin_index
all in **integer**. Remember the encoding way of `clients` and `origins` described before.

#### GQD Cache on RTT Platform
**params**: `cache_number`-the number of caches in the system, `clinet_number`-the number of different clinets in the system, `origin_number`-the number of different origin servers in the system, `file_path`-the file path of the text file recording the two rtt tables, `timer`-the time span life for each content in GQD cache

    ./webcachesim test.tr RTT_GQD 1000 cache_number=3 client_number=3 origin_number=3 file_path=rtt_file.txt timer=50


#### LRU Cache on RTT Platform
**params**: `cache_number`-the number of caches in the system, `clinet_number`-the number of different clinets in the system, `origin_number`-the number of different origin servers in the system, `file_path`-the file path of the text file recording the two rtt tables. *be aware that only GQD cache need param timer*

    ./webcachesim test.tr RTT_LRU 1000 cache_number=3 client_number=3 origin_number=3 file_path=rtt_file.txt

#### LFUDA Cache on RTT Platform

    ./webcachesim test.tr RTT_LFUDA 1000 cache_number=3 client_number=3 origin_number=3 file_path=rtt_file.txt

#### GDSF Cache on RTT Platform

    ./webcachesim test.tr RTT_GDSF 1000 cache_number=3 client_number=3 origin_number=3 file_path=rtt_file.txt

#### LRU-k Cache on RTT Platform

    ./webcachesim test.tr RTT_LRUK 1000 cache_number=3 client_number=3 origin_number=3 file_path=rtt_file.txt k=2

#### Adaptsize Cache on RTT Platform
**params**: `cache_number`-the number of caches in the system, `clinet_number`-the number of different clinets in the system,`t` & `i`-refer to Adaptsize cache above, `origin_number`-the number of different origin servers in the system, `file_path`-the file path of the text file recording the two rtt tables. *be aware that only GQD cache need param timer*

    ./webcachesim test.tr RTT_AptSize 1000 cache_number=3 t=10000 i=5 client_number=3 origin_number=3 file_path=rtt_file.txt timer=50

## How to get traces:(We Do Not Use This Part)


### Generate your own traces with a given distribution 

One example is a Pareto (Zipf-like) popularity distribution and Bounded-Pareto object size distribution.
The "basic_trace" tool takes the following parameters:

 - how many unique objects
 - how many requests to generate for most popular object (total request length will be a multiple of that)
 - Pareto shape
 - min object size
 - max object size
 - output name for trace

Here's an example that recreates the "test.tr" trace for the examples above. This uses the "basic_trace" generator with 1000 objects, about 10000 requests overall, Pareto shape 1.8 and object sizes between 1 and 10000 bytes.

    g++ tracegenerator/basic_trace.cc -std=c++11 -o basic_trace
    ./basic_trace 1000 1000 1.8 1 10000 test.tr
    make
    ./webcachesim test.tr 0 LRU 1000


### Rewrite existing open-source traces

Example: download a public 1999 request trace ([trace description](http://www.cs.bu.edu/techreports/abstracts/1999-011)), rewrite it into our format, and run the simulator.

    wget http://www.cs.bu.edu/techreports/1999-011-usertrace-98.gz
    gunzip 1999-011-usertrace-98.gz
    g++ -o rewrite -std=c++11 ../traceparser/rewrite_trace_http.cc
    ./rewrite 1999-011-usertrace-98 test.tr
    make
    ./webcachesim test.tr 0 LRU 1073741824


## Implement a new policy

All cache implementations inherit from "Cache" (in policies/cache.h) which defines common features such as the cache capacity, statistics gathering, and the request interface. Defining a new policy needs override the virtual function in class cache.

    class YourPolicy: public Cache {
    public:
      // override virtual function in Cache
      // for policy need to set interface to set arbitrary parameters request
      virtual void setPar(string parName, string parValue) {
        if(parName=="myPar") {
          myPar = stof(parValue);
        }
      }
    
      /**
      * rest implement `lookup` and `admit` function for
      * interfere with requests, see webcachesim.cpp for
      * more infomation
      */
    
    protected:
      double myPar;  // setted param of this policy
    };

    // register your policy with the framework
    static Factory<YourPolicy> factoryYP("YourPolicy");
 
This allows the user interface side to conveniently configure and use your new policy. Code bellow will be automatically run in `webcachesim.cpp` no need for more change.

    // create new cache
    unique_ptr<Cache> webcache = Cache::create_unique("YourPolicy");
    // set cache capacity
    webcache->setSize(1000);
    // set an arbitrary param (parser implement by yourPolicy)
    webcache->setPar("myPar", "0.94");