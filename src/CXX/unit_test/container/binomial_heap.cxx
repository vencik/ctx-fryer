/**
 *  \brief  Binomial heap unit test
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

#include "container/heap.hxx"

#include <iostream>
#include <climits>


int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    container::binomial_heap<int> heap1;

    int n;

    std::cerr << "Creating the heap..." << std::endl;

    while (std::cin >> n) {
        heap1.add(n);
    }

    container::binomial_heap<int> heap2(heap1);

    int old_min = INT_MIN;

    std::cerr << "Checking heap property..." << std::endl;

    while (!heap2.empty()) {
        int min = heap2.get_min();

        // Sanity check
        if (min < old_min) {
            std::cerr
                << "FAILED: the current minimum " << min
                << " is lower than a previously encountered one: " << old_min
                << std::endl;

            exit_code = 1;
        }

        std::cout << min << ' ';

        heap2.delete_min();

        old_min = min;
    }

    std::cout << std::endl;

    return exit_code;
}


// Wrapper around main_impl to ensure that all destructors of objects
// created in main are executed before ::exit is called
int main(int argc, char * const argv[]) {
    ::exit(main_impl(argc, argv));
}
