#include <fstream>
#include <unordered_map>
#include <limits>
#include <cmath>
#include <cassert>
#include <cmath>
#include <cassert>
#include <string.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <queue>
#include <cmath>
#include <typeinfo>
#include "rtt_variants.h"
#include "gqd_cache.h"
#include "gd_variants.h"
#include "../random_helper.h"

/*
    rtt bench
*/

void RTT_GQD_Cache::setPar(std::string parName, std::string parValue)
{
    // if (parName.compare("type") == 0)
    // {
    //     if (parValue.compare("LRU") == 0)
    //         type = LRU;
    //     else if (parValue.compare("GQD") == 0)
    //         type = GQD;
    //     else {
    //         std::cerr << "unsupported cache type: " << parValue << std::endl;
    //     }
    // }
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        caches_list = new GQDCache[cache_number];
        // if (type == LRU)
        // {
        //     //caches_list = new LRUCache[cache_number];
        // }
        // else if (type == GQD)
        // {
        //     caches_list = new GQDCache[cache_number];
        // }
        for (int i = 0; i < cache_number; ++i)
        {
            caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else if (parName.compare("timer") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        timer_max_time = n;
        QOECacheObject::max_time = n;
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

void RTT_GQD_Cache::init()
{
    std::cerr << "init begin!" << std::endl;
    std::ifstream infile(rtt_file, std::ios::in);
    uint16_t min_rtt = UINT16_MAX;
    uint16_t max_rtt = 0;
    clients2caches = new uint16_t *[client_number];
    norm_clients2caches = new double *[client_number];
    for (int row = 0; row < client_number; row++)
    {
        clients2caches[row] = new uint16_t[cache_number];
        norm_clients2caches[row] = new double[cache_number];
        for (int col = 0; col < cache_number; col++)
        {
            infile >> clients2caches[row][col];
            if (clients2caches[row][col] < min_rtt)
                min_rtt = clients2caches[row][col];
            if (clients2caches[row][col] > max_rtt) {
                max_rtt = clients2caches[row][col];
            }
        }
    }
    std::string line;
    getline(infile, line);
    caches2origins = new uint16_t *[cache_number];
    norm_caches2origins = new double *[cache_number];
    for (int row = 0; row < cache_number; row++)
    {
        caches2origins[row] = new uint16_t[origin_number];
        norm_caches2origins[row] = new double[origin_number];
        for (int col = 0; col < origin_number; col++)
        {
            infile >> caches2origins[row][col];
            if (caches2origins[row][col] < min_rtt)
                min_rtt = caches2origins[row][col];
            if (caches2origins[row][col] > max_rtt) {
                max_rtt = caches2origins[row][col];
            }
        }
    }
    double range = max_rtt - min_rtt;
    redirect_table = new uint16_t *[client_number];
    if (QOECacheObject::max_time) {
        for (int client_index = 0; client_index < client_number; client_index++)
        {
            redirect_table[client_index] = new uint16_t[origin_number];
            for (int origin_index = 0; origin_index < origin_number; origin_index++)
            {
                uint16_t min_rtt_value = clients2caches[client_index][0];
                std::vector<uint16_t> candidate_cache_index(1, 0);
                for (int cache_index = 1; cache_index < cache_number; cache_index++)
                {
                    if (clients2caches[client_index][cache_index] < min_rtt_value)
                    {
                        candidate_cache_index.clear();
                        candidate_cache_index.push_back(cache_index);
                        min_rtt_value = clients2caches[client_index][cache_index];
                    }
                    else if (clients2caches[client_index][cache_index] == min_rtt_value)
                    {
                        candidate_cache_index.push_back(cache_index);
                    }
                }
                if (candidate_cache_index.size() == 1)
                {
                    redirect_table[client_index][origin_index] = candidate_cache_index[0];
                }
                else {
                    uint16_t min_rtt_value = caches2origins[candidate_cache_index[0]][origin_index];
                    uint8_t min_rtt_index = candidate_cache_index[0];
                    for (uint32_t cache_index = 0; cache_index < candidate_cache_index.size(); cache_index++) {
                        if (candidate_cache_index[cache_index] < min_rtt_value) {
                            min_rtt_value = candidate_cache_index[cache_index];
                            min_rtt_index = cache_index;
                        }
                    }
                    redirect_table[client_index][origin_index] = candidate_cache_index[min_rtt_index];
                }
            }
        }
    }
    else {
        for (int client_index = 0; client_index < client_number; client_index++)
        {
            redirect_table[client_index] = new uint16_t[origin_number];
            for (int origin_index = 0; origin_index < origin_number; origin_index++)
            {
                uint16_t max_rtt_value = clients2caches[client_index][0];
                std::vector<uint16_t> candidate_cache_index(1, 0);
                for (int cache_index = 1; cache_index < cache_number; cache_index++)
                {
                    if (clients2caches[client_index][cache_index] > max_rtt_value)
                    {
                        candidate_cache_index.clear();
                        candidate_cache_index.push_back(cache_index);
                        max_rtt_value = clients2caches[client_index][cache_index];
                    }
                    else if (clients2caches[client_index][cache_index] == max_rtt_value)
                    {
                        candidate_cache_index.push_back(cache_index);
                    }
                }
                if (candidate_cache_index.size() == 1)
                {
                    redirect_table[client_index][origin_index] = candidate_cache_index[0];
                }
                else {
                    uint16_t max_rtt_value = caches2origins[candidate_cache_index[0]][origin_index];
                    uint8_t max_rtt_index = candidate_cache_index[0];
                    for (uint32_t cache_index = 0; cache_index < candidate_cache_index.size(); cache_index++) {
                        if (candidate_cache_index[cache_index] > max_rtt_value) {
                            max_rtt_value = candidate_cache_index[cache_index];
                            max_rtt_index = cache_index;
                        }
                    }
                    redirect_table[client_index][origin_index] = candidate_cache_index[max_rtt_index];
                }
            }
        }
    }
    std::cout << std::endl; 
    std::cout << "client to cache table: " << std::endl; 
    for (int i = 0; i < client_number; i++) {
        for (int j = 0; j < cache_number; j++) {
            std::cout << clients2caches[i][j] << " ";
            norm_clients2caches[i][j] = (clients2caches[i][j] - min_rtt) / range;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl; 
    std::cout << "cache to origin table: " << std::endl; 
    for (int i = 0; i < cache_number; i++) {
        for (int j = 0; j < origin_number; j++) {
            std::cout << caches2origins[i][j] << " ";
            norm_caches2origins[i][j] = (caches2origins[i][j] - min_rtt) / range;
        }
        std::cout << std::endl;
    }

    std::cout << std::endl; 
    std::cout << "redirect table: " << std::endl; 
    for (int i = 0; i < client_number; i++) {
        for (int j = 0; j < origin_number; j++) {
            std::cout << redirect_table[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl; 
    std::cerr << "init done!" << std::endl;
}

double RTT_GQD_Cache::rtt2qoe(int rtt) {
    if (rtt <= 100) {
        return 100;
    }
    if (rtt > 20000) {
        return 0;
    }
    return A * rtt + B;
}

double RTT_GQD_Cache::rtt2qoe(double rtt) {
    if (rtt < 0.5) {
        return 1.0 - 1.0 / (1.0 + std::exp(-(10 * rtt - 5)));
    }
    else {
        return 1.0 / (1996 * rtt -996);
    }
}

bool RTT_GQD_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    update();
    for (int i = 0; i < cache_number; i++)
    {
        if (caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    caches_list[redirect_table[client][origin]].admit(req, caches2origins[redirect_table[client][origin]][origin] / req->getSize());
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}

void RTT_GQD_Cache::update() {
    for (int i = 0; i < cache_number; i++) {
        caches_list[i].update();
    }
}

void RTT_LRU_Cache::setPar(std::string parName, std::string parValue)
{
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        _caches_list = new LRUCache[cache_number];
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool RTT_LRU_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    for (int i = 0; i < cache_number; i++)
    {
        if (_caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    _caches_list[redirect_table[client][origin]].admit(req);
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}

void RTT_AptSize_Cache::setPar(std::string parName, std::string parValue)
{
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        _caches_list = new AdaptSizeCache[cache_number];
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("i") == 0) {
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setPar("i", parValue);
        }
    }
    else if (parName.compare("t") == 0) {
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setPar("t", parValue);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool RTT_AptSize_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    for (int i = 0; i < cache_number; i++)
    {
        if (_caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    _caches_list[redirect_table[client][origin]].admit(req);
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}


void RTT_GDSF_Cache::setPar(std::string parName, std::string parValue)
{
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        _caches_list = new GDSFCache[cache_number];
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool RTT_GDSF_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    for (int i = 0; i < cache_number; i++)
    {
        if (_caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    _caches_list[redirect_table[client][origin]].admit(req);
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}

void RTT_LRUK_Cache::setPar(std::string parName, std::string parValue)
{
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        _caches_list = new LRUKCache[cache_number];
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else if (parName.compare("k") == 0) {
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setPar("k", parValue);
        }
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool RTT_LRUK_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    for (int i = 0; i < cache_number; i++)
    {
        if (_caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    _caches_list[redirect_table[client][origin]].admit(req);
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}

void RTT_LFUDA_Cache::setPar(std::string parName, std::string parValue)
{
    if (parName.compare("cache_number") == 0)
    {
        const int n = stoull(parValue);
        assert(n > 1);
        cache_number = n;
        _caches_list = new LFUDACache[cache_number];
        for (int i = 0; i < cache_number; ++i)
        {
            _caches_list[i].setSize(_cacheSize);
        }
    }
    else if (parName.compare("client_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        client_number = n;
    }
    else if (parName.compare("origin_number") == 0) {
        const int n = stoull(parValue);
        assert(n > 1);
        origin_number = n;
    }
    else if (parName.compare("file_path") == 0)
    {
        rtt_file = parValue;
    }
    else
    {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool RTT_LFUDA_Cache::request(SimpleRequest *req, uint8_t client, uint8_t origin)
{
    for (int i = 0; i < cache_number; i++)
    {
        if (_caches_list[i].lookup(req)) {
            sum_QoE += rtt2qoe(clients2caches[client][i]);
            norm_sum_QoE += rtt2qoe(norm_clients2caches[client][i]);
            return true;
        }
            
    }
    _caches_list[redirect_table[client][origin]].admit(req);
    sum_QoE += rtt2qoe(clients2caches[client][redirect_table[client][origin]] + caches2origins[redirect_table[client][origin]][origin]);
    norm_sum_QoE += rtt2qoe(norm_clients2caches[client][redirect_table[client][origin]] + norm_caches2origins[redirect_table[client][origin]][origin]);
    return false;
}