/**
 *  \brief  Random numbers generator
 *
 *  Provider of large number of random numbers (for UT purposes).
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/11/20
 *
 *  Legal notices
 *
 *  Copyright 2013 Vaclav Krpec
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

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <climits>

#define DEFAULT_COUNT 20
#define DEFAULT_MIN   0
#define DEFAULT_MAX   (INT_MAX - 1)


int main(int argc, char * const argv[]) {
    ::srand(time(NULL));

    // Number count
    size_t cnt = DEFAULT_COUNT;

    if (argc > 1) {
        std::stringstream ss(argv[1]);

        ss >> cnt;
    }

    // Min
    int min = DEFAULT_MIN;

    if (argc > 2) {
        std::stringstream ss(argv[2]);

        ss >> min;
    }

    // Max
    int max = DEFAULT_MAX;

    if (argc > 2) {
        std::stringstream ss(argv[3]);

        ss >> max;
    }

    int mod = max - min + 1;

    // Sanity check
    if (mod <= 0) {
        std::cerr
            << "Invalid range: [" << min << ", " << max << "]"
            << std::endl;

        ::exit(1);
    }

    // Generate numbers
    while (cnt-- > 0) {
        int rnd = ::rand();

        rnd %= mod;
        rnd += min;

        std::cout << rnd << ' ';
    }

    std::cout << std::endl;

    return 0;
}
