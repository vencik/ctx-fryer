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
