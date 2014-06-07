/**
 *  \brief  Bit info table unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/06
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

#include "meta/bit_info.hxx"

#include <iostream>
#include <string>


// Uncomment (or add -DDEBUG_LOG g++ option) to get log
//#define DEBUG_LOG


/** Unit test implementation call */
#define TEST(what, type) test_ ## what<type>(#type)


/** \ref meta::bit_info::set_cnt unit test */
template <typename T>
int test_set_cnt(const std::string & type) {
    for (T i = 0; ; ++i) {
        unsigned n = 0;

        for (T mask = 0x1; mask; mask <<= 1)
            n += !!(i & mask);

        unsigned N = meta::bit_info::set_cnt(i);

        if (n != N) {
            std::cerr
                << "meta::bit_info::set_cnt((" << type << ")"
                << (unsigned)i << ") == " << N << ", expected "
                << n << std::endl;

            return 1;
        }

#ifdef DEBUG_LOG
        std::cout
            << "meta::bit_info::set_cnt((" << type << ")"
            << (unsigned)i << ") == " << N << std::endl;
#endif

        if (i == (T)-1) break;
    }

    return 0;
}

/** \ref meta::bit_info::clear_cnt unit test */
template <typename T>
int test_clear_cnt(const std::string & type) {
    for (T i = 0; ; ++i) {
        unsigned n = 0;

        for (T mask = 0x1; mask; mask <<= 1)
            n += !(i & mask);

        unsigned N = meta::bit_info::clear_cnt(i);

        if (n != N) {
            std::cerr
                << "meta::bit_info::clear_cnt((" << type << ")"
                << (unsigned)i << ") == " << N << ", expected "
                << n << std::endl;

            return 1;
        }

#ifdef DEBUG_LOG
        std::cout
            << "meta::bit_info::clear_cnt((" << type << ")"
            << (unsigned)i << ") == " << N << std::endl;
#endif

        if (i == (T)-1) break;
    }

    return 0;
}

/** \ref meta::bit_info::ls1b_off unit test */
template <typename T>
int test_ls1b_off(const std::string & type) {
    for (T i = 0; ; ++i) {
        unsigned off = 0;

        for (T mask = 0x1; mask; mask <<= 1, ++off)
            if (i & mask) break;

        unsigned OFF = meta::bit_info::ls1b_off(i);

        if (off != OFF) {
            std::cerr
                << "meta::bit_info::ls1b_off((" << type << ")"
                << (unsigned)i << ") == " << OFF << ", expected "
                << off << std::endl;

            return 1;
        }

#ifdef DEBUG_LOG
        std::cout
            << "meta::bit_info::ls1b_off((" << type << ")"
            << (unsigned)i << ") == " << OFF << std::endl;
#endif

        if (i == (T)-1) break;
    }

    return 0;
}

/** \ref meta::bit_info::ms1b_off unit test */
template <typename T>
int test_ms1b_off(const std::string & type) {
    unsigned bit_cnt = sizeof(T) * 8;

    for (T i = 0; ; ++i) {
        unsigned off = bit_cnt;

        for (T mask = 0x1 << (bit_cnt - 1); mask; mask >>= 1, --off)
            if (i & mask) break;

        off = off ? off - 1 : bit_cnt;

        unsigned OFF = meta::bit_info::ms1b_off(i);

        if (off != OFF) {
            std::cerr
                << "meta::bit_info::ms1b_off((" << type << ")"
                << (unsigned)i << ") == " << OFF << ", expected "
                << off << std::endl;

            return 1;
        }

#ifdef DEBUG_LOG
        std::cout
            << "meta::bit_info::ms1b_off((" << type << ")"
            << (unsigned)i << ") == " << OFF << std::endl;
#endif

        if (i == (T)-1) break;
    }

    return 0;
}


/** Unit test */
int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    exit_code = TEST(set_cnt, unsigned char);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(set_cnt, uint16_t);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(clear_cnt, unsigned char);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(clear_cnt, uint16_t);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(ls1b_off, unsigned char);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(ls1b_off, uint16_t);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(ms1b_off, unsigned char);
    if (0 != exit_code) return exit_code;

    exit_code = TEST(ms1b_off, uint16_t);
    if (0 != exit_code) return exit_code;

    return 0;  // all tests passed
}

/** Exception-safety wrapper */
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
