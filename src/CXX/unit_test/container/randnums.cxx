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
