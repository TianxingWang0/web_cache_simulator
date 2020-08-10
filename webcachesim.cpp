#include "caches/cluster_variants.h"
#include "caches/gd_variants.h"
#include "caches/lru_variants.h"
#include "request.h"
#include <chrono>
#include <random>

using namespace std;
ofstream outTp;

int main(int argc, char *argv[]) {

  // output help if insufficient params
  auto wall_clock = chrono::steady_clock::now();
  if (argc < 4) {
    cerr << "webcachesim traceFile cacheType cacheSizeBytes [cacheParams]"
         << endl;
    return 1;
  }
  cerr << "beginning..." << endl;

  // trace properties
  const char *path = argv[1];

  // create cache
  const string cacheType = argv[2];
  unique_ptr<Cache> webcache = Cache::create_unique(cacheType);
  if (webcache == nullptr)
    return 1;
  // configure cache size
  const uint64_t cache_size = std::stoull(argv[3]);
  webcache->setSize(cache_size);

  bool file_size = false;
  // parse cache parameters(cache number)
  regex opexp("(.*)=(.*)");
  cmatch opmatch;
  string paramSummary;
  for (int i = 4; i < argc; i++) {
    regex_match(argv[i], opmatch, opexp);
    if (opmatch.size() != 3) {
      std::cerr << "each cacheParam needs to be in form name=value" << endl;
      return 1;
    }
    cerr << "beginning..." << endl;

    // trace properties
    const char *path = argv[1];

    // create cache
    const string cacheType = argv[2];
    cout << cacheType << endl;
    unique_ptr<Cache> webcache = Cache::create_unique(cacheType);
    if (webcache == nullptr)
        return 1;
    // configure cache size
    const uint64_t cache_size = std::stoull(argv[3]);
    webcache->setSize(cache_size);
    // parse cache parameters(cache number)
    regex opexp("(.*)=(.*)");
    cmatch opmatch;
    string paramSummary;
    for (int i = 4; i < argc; i++)
    {
        regex_match(argv[i], opmatch, opexp);
        if (opmatch.size() != 3)
        {
            std::cerr << "each cacheParam needs to be in form name=value" << endl;
            return 1;
        }
        webcache->setPar(opmatch[1], opmatch[2]);
        paramSummary += opmatch[1];
        paramSummary += "=";
        paramSummary += opmatch[2];
        paramSummary += " ";
    }
    ifstream infile;
    long long reqs = 0, hits = 0;
    long long reqs_size = 0, hits_size = 0;
    long long t, id, size, client, origin;
    //uint8_t client, origin;
    SimpleRequest *req = new SimpleRequest(0, 0);

    cerr << "running..." << endl;

    infile.open(path);
    cout << cacheType << endl;
    if (cacheType.compare("SF") && cacheType.compare("CH") && cacheType.compare("SFM") &&
        cacheType.compare("RTT_GQD") && cacheType.compare("RTT_LRU") && cacheType.compare("RTT_AptSize"))
    {   // single cache 
        while (infile >> t >> id >> size)
        {
            size = 1;
            reqs++;
            reqs_size += size;

            req->reinit(id, size);
            if (webcache->lookup(req))
            {
                hits++;
                hits_size += size;
            }
            else
            {
                webcache->admit(req);
            }
        }
    }
    else if (cacheType.compare("RTT_GQD") == 0 || cacheType.compare("RTT_LRU") == 0 || cacheType.compare("RTT_AptSize") == 0) {   // rtt cache system
        // the trace format is <client ip, content id, content size, origin ip>
        webcache->init();
        while (infile >> client >> id >> size >> origin) {
            //std::cout << client << " " << id << " " << size << " " << origin << std::endl;
            reqs++;
            reqs_size += size;
            req->reinit(id, size);
            if (webcache->request(req, client, origin))
            {
                hits++;
                hits_size += size;
            }
        } 
        //   //fack trace
        // while (infile >> t >> id >> size) {
        //     reqs++;
        //     reqs_size += size;
        //     req->reinit(id, size);
        //     //client = std::rand() % 3;
        //     client = id % 3;
        //     origin = id % 3;
        //     if (webcache->request(req, client, origin))
        //     {
        //         hits++;
        //         hits_size += size;
        //     }
        // }
    }
    else // cluster variants
    {
        webcache->init_mapper();
        while (infile >> t >> id >> size)
        {
            size = 1;
            reqs++;
            reqs_size += size;
            req->reinit(id, size);
            if (webcache->request(req))
            {
                hits++;
                hits_size += size;
            }
            if (reqs % 100000 == 0) {
                webcache->print_hash_space();
            }
        }
        // if (reqs % 100000 == 0) {
        //   webcache->print_hash_space();
        // }
      }
    }
    delete req;

    infile.close();
    outTp.open("results.txt", ofstream::app);
    const long timeElapsed = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - wall_clock).count();
    cout << "time duration : " << timeElapsed << "ms" << endl;
    cout << "trace : " << path << endl;
    cout << "cacheType : " << cacheType << "\tcache_size : " << cache_size << "\tparamSummary : " << paramSummary << endl;
    cout << "reqs : " << reqs << "\thits : " << hits << "\thit rate : " << double(hits) / reqs << endl;
    cout << "reqs size : " << reqs_size << "\thit size : " << hits_size << "\thit size rate : " << double(hits_size) / reqs_size << endl;
    if (cacheType.compare("RTT_GQD") == 0 || cacheType.compare("RTT_LRU") == 0 || cacheType.compare("RTT_AptSize") == 0) {
        cout << "QoE : " << webcache->get_sum_QoE() / reqs << endl;
    }

    outTp << endl;
    outTp << "time duration : " << timeElapsed << "ms" << endl;
    outTp << "trace : " << path << endl;
    outTp << "cacheType : " << cacheType << "\tcache_size : " << cache_size << "\tparamSummary : " << paramSummary << endl;
    outTp << "reqs : " << reqs << "\thits : " << hits << "\thit rate : " << double(hits) / reqs << endl;
    outTp << "reqs size : " << reqs_size << "\thit size : " << hits_size << "\thit size rate : " << double(hits_size) / reqs_size << endl;
    if (cacheType.compare("RTT_GQD") == 0 || cacheType.compare("RTT_LRU") == 0) {
        outTp << "QoE : " << webcache->get_sum_QoE() / reqs << endl;
    }

    outTp.close();
    return 0;
}

/*
  consistent hash:  ./webcachesim test.tr CH size_of_each_cache n=cache_number
                eg: ./webcachesim test.tr CH 1000 n=4

  shuffler : ./webcachesim
  /mnt/c/Users/28347/Documents/CDN/trace/wiki_100million_1/sim_100million_1.tr
  SF 1073741824 n=4 alpha=15 K=20 W=10000

  Shuffler Matrix: ./webcachesim /mnt/c/Users/28347/Documents/CDN/trace/wiki_100million_1/sim_100million_1.tr SFM 1073741824 n=4 alpha=15 W=12800 vnode=40 t=1000


  rtt:
    ./webcachesim test.tr RTT 1000 n=3 type=LRU file_path=rtt_file.txt timer=50  


*/