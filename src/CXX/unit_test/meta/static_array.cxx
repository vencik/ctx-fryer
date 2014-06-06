/**
 *  \brief  Compile-time initialised array unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/05
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

#include "meta/static_array.hxx"

#include <iostream>


/** Array initialiser meta-functor for meta::static_array unit test */
template <size_t N>
class array_init {
    public:

    enum { value = 2 * N };  /**< F(X) == 2*X */

    inline static unsigned check(unsigned n) { return 2 * n; }

};  // end of template class array_init

static const size_t array_size = 876;  // note that the default limit is 900

/** Array initialised at compile time by template initialiser */
static meta::base<unsigned>::static_array<array_size, array_init> array;

/** \ref meta::static_array unit test */
int test_static_array() {
    int exit_code = 0;

    for (size_t i = 0; i < array_size; ++i) {
        unsigned expected = array_init<array_size>::check(i);

        if (array[i] != expected) {
            std::cerr
                << "meta::static_array UT: eq[ " << i << "]: expected "
                << expected << ", got " << array[i] << std::endl;

            exit_code = 1;
        }
    }

    return exit_code;
}


/** Unit test */
int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    // Test static_array
    exit_code = test_static_array();

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
