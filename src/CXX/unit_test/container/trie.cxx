/**
 *  \brief  Trie unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/10
 *
 *  Legal notices
 *
 *  Copyright 2014 Vaclav Krpec
 *
 *  CTX Fryer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "container/trie.hxx"
#include "sys/time.hxx"

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <ctime>
#include <cassert>

extern "C" {
    #include <unistd.h>
}


// Test trie type
typedef std::pair<std::string, unsigned>   my_entry1_t;
typedef container::trie<char, my_entry1_t> my_trie1_t;

// Test map (to check keys)
typedef std::map<std::string, unsigned> my_map1_t;


/** Generate random number within bounds */
static unsigned rand_from(unsigned lo, unsigned hi) {
    assert(lo <= hi);

    unsigned x = hi - lo + 1;

    return lo + (::rand() % x);
}


/** Generate random key of length \c len_min - \c len_max */
static std::string gen_key(size_t len_min, size_t len_max) {
    static const char alphabet[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    size_t len = rand_from((unsigned)len_min, (unsigned)len_max);

    std::string key;
    key.reserve(len);

    for (size_t i = 0; i < len; ++i) {
        unsigned ch_i = rand_from(0, sizeof(alphabet) / sizeof(char) - 1);

        key.push_back(alphabet[ch_i]);
    }

    return key;
}


/** Generate key map */
static std::vector<std::string> gen_keys(
    size_t size, size_t len_min, size_t len_max)
{
    std::vector<std::string> keys;
    keys.reserve(size);

    for (size_t i = 0; i < size; ++i)
        keys.push_back(gen_key(len_min, len_max));

    return keys;
}


// Usage
static void usage(const std::string & self) {
    std::cerr
        << "Usage: " << self << " [OPTIONS]" << std::endl
        << std::endl
        << "    OPTIONS:" << std::endl
        << "        -h                  Show this help and exit" << std::endl
        << "        -v                  Verbose output" << std::endl
        << "        -S <seed>           RNG seed" << std::endl
        << "        -n <#keys>          Number of generated keys (10)" << std::endl
        << "        -m <min. len.>      Min. key length (0)" << std::endl
        << "        -M <max. len.>      Max. key length (128)" << std::endl
        << std::endl;
}

// Trie unit test
static int main_impl(int argc, char * const argv[]) {
    // Process options
    unsigned rng_seed    = ::time(NULL);
    bool     verbose     = false;
    size_t   key_cnt     = 10;
    size_t   key_min_len = 0;
    size_t   key_max_len = 128;

    for (;;) {
        int opt = ::getopt(argc, argv, "hvS:n:m:M:");

        if (-1 == opt) break;

        switch (opt) {
            case 'h':  // show usage and exit
                usage(argv[0]);
                return 0;

            case 'v':  // verbose operation
                verbose = true;
                break;

            case 'S':  // RNG seed
                rng_seed = (unsigned)::atol(::optarg);
                break;

            case 'n':  // number of keys
                key_cnt = (size_t)::atol(::optarg);
                break;

            case 'm':  // min. key length
                key_min_len = (size_t)::atol(::optarg);
                break;

            case 'M':  // max. key length
                key_max_len = (size_t)::atol(::optarg);
                break;

            default:  // unknown option
                usage(argv[0]);
                return 64;
        }
    }

    // Options sanity checks
    if (!(key_min_len <= key_max_len))
        throw std::logic_error(
            "trie UT: FAULTY OPTIONS: key length bounds underflow");

    // Seed RNG
    ::srand(rng_seed);

    std::cerr << "RNG seeded by " << rng_seed << std::endl;

    // Unit test
    int exit_code = 0;

    my_trie1_t trie1;

    std::string str;

    // Generate keys
    std::cout << "Generating key list..." << std::endl;

    std::vector<std::string> keys = gen_keys(
        key_cnt, key_min_len, key_max_len);

    // Create key map
    std::cout << "Creating key map (for checking purposes)..." << std::endl;

    my_map1_t key_map;
    double    key_map_ins_tsum = 0.0;

    std::vector<std::string>::const_iterator k = keys.begin();

    for (unsigned v = 0; k != keys.end(); ++k, ++v) {
        if (verbose)
            std::cout
                << "Entry: [\"" << *k << "\", " << v << "]" << std::endl;

        std::pair<std::string, unsigned> entry(*k, v);

        sys::timer t; t.start();

        key_map.insert(entry);

        key_map_ins_tsum += t.elapsed();
    }

    // Populate trie
    std::cout << "Creating the trie..." << std::endl;

#if (0)
    for (unsigned i = 0; std::cin >> str; ++i) {
        trie1.insert(str, my_entry1_t(str, i));
    }
#endif

    double trie1_ins_tsum = 0.0;

    k = keys.begin();

    for (unsigned v = 0; k != keys.end(); ++k, ++v) {
        my_entry1_t entry(*k, v);

        sys::timer t; t.start();

        trie1.insert(*k, entry);

        trie1_ins_tsum += t.elapsed();
    }

    // Print trie contents
    if (verbose) {
        std::cout << "Trie content:" << std::endl;

        my_trie1_t::iterator e = trie1.begin();

        for (; e != trie1.end(); ++e)
            std::cout << "\"" << e->first << "\" : " << e->second << std::endl;
    }

    std::cout << "Checking the trie contents..." << std::endl;

    std::vector<std::string>::const_reverse_iterator rk = keys.rbegin();

    for (; rk != keys.rend(); ++rk) {
        my_map1_t::const_iterator m_search = key_map.find(*rk);

        // The key MUST be in the map...
        if (key_map.end() == m_search)
            throw std::logic_error(
                "trie UT: INTERNAL ERROR: key not found in map as it shoud be");

        my_trie1_t::iterator t_search = trie1.find(*rk);

        // The key should be in the trie
        if (trie1.end() == t_search) {
            std::cerr << "FAILED key: \"" << *rk << "\"" << std::endl;

            throw std::logic_error(
                "trie UT: FAILURE: key not found in trie");
        }

        // The stored key should match
        if (*rk != t_search->first) {
            std::cerr << "FAILED key: \"" << *rk << "\"" << std::endl;

            throw std::logic_error(
                "trie UT: FAILURE: stored key doesn't match");
        }

        // The value should match key_map
        if (t_search->second != m_search->second) {
            std::cerr
                << "FAILED key: \"" << *rk << "\", "
                << "expected value: " << m_search->second << ", "
                << "trie value: " << t_search->second << std::endl;

            throw std::logic_error(
                "trie UT: FAILURE: stored value doesn't match");
        }
    }

    // Print statistics
    double trie1_ins_tpo    = trie1_ins_tsum   / keys.size();
    double key_map_ins_tpo  = key_map_ins_tsum / keys.size();
    double ins_speed_factor = trie1_ins_tsum / key_map_ins_tsum;

    std::cout
        << "Statistics:" << std::endl
        << "\tTrie insertion time total: " << trie1_ins_tsum
        << " s, that is " << trie1_ins_tpo << " s/op avg" << std::endl
        << "\tMap insertion time total: " << key_map_ins_tsum
        << " s, that is " << key_map_ins_tpo << " s/op avg" << std::endl
        << "\tInsertion time speed factor: " << ins_speed_factor
        << std::endl;

    std::cout << "All tests PASSED" << std::endl;

    return exit_code;
}


// Exception-safe wrapper around the "real" main function
int main(int argc, char * const argv[]) {
    int exit_code = 127;  // exception caught

    try {
        exit_code = main_impl(argc, argv);
    }
    catch (std::exception & x) {
        std::cerr
            << "Standard exception caught: "
            << x.what()
            << std::endl;
    }
    catch (...) {
        std::cerr
            << "Unknown exception caught"
            << std::endl;
    }

    ::exit(exit_code);
}
