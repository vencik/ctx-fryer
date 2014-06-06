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


/** \ref meta::bit_info::set_cnt unit test */
int test_set_cnt() {
    for (unsigned i = 0; i < 256; ++i) {
        unsigned n = 0;

        for (unsigned char mask = 0x1; mask; mask <<= 1)
            n += !!((unsigned char)i & mask);

        if (n != meta::bit_info::set_cnt(i)) {
            std::cerr
                << "meta::bit_info::set_cnt(" << i << ") == "
                << meta::bit_info::set_cnt(i) << ", expected "
                << n << std::endl;

            return 1;
        }
    }

    return 0;
}

/** \ref meta::bit_info::clear_cnt unit test */
int test_clear_cnt() {
    for (unsigned i = 0; i < 256; ++i) {
        unsigned n = 0;

        for (unsigned char mask = 0x1; mask; mask <<= 1)
            n += !((unsigned char)i & mask);

        if (n != meta::bit_info::clear_cnt(i)) {
            std::cerr
                << "meta::bit_info::clear_cnt(" << i << ") == "
                << meta::bit_info::clear_cnt(i) << ", expected "
                << n << std::endl;

            return 1;
        }
    }

    return 0;
}

/** \ref meta::bit_info::ls1b_off unit test */
int test_ls1b_off() {
    for (unsigned i = 0; i < 256; ++i) {
        unsigned off = 0;

        for (unsigned char mask = 0x1; mask; mask <<= 1, ++off)
            if ((unsigned char)i & mask) break;

        if (off != meta::bit_info::ls1b_off(i)) {
            std::cerr
                << "meta::bit_info::ls1b_off(" << i << ") == "
                << meta::bit_info::ls1b_off(i) << ", expected "
                << off << std::endl;

            return 1;
        }
    }

    return 0;
}

/** \ref meta::bit_info::ms1b_off unit test */
int test_ms1b_off() {
    for (unsigned i = 0; i < 256; ++i) {
        unsigned off = 8;

        for (unsigned char mask = 0x80; mask; mask >>= 1, --off)
            if ((unsigned char)i & mask) break;

        off = off ? off - 1 : 8;

        if (off != meta::bit_info::ms1b_off(i)) {
            std::cerr
                << "meta::bit_info::ms1b_off(" << i << ") == "
                << meta::bit_info::ms1b_off(i) << ", expected "
                << off << std::endl;

            return 1;
        }
    }

    return 0;
}


/** Unit test */
int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    exit_code = test_set_cnt();
    if (0 != exit_code) return exit_code;

    exit_code = test_clear_cnt();
    if (0 != exit_code) return exit_code;

    exit_code = test_ls1b_off();
    if (0 != exit_code) return exit_code;

    exit_code = test_ms1b_off();
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
