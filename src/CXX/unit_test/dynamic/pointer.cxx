/**
 *  \brief  Smart pointers unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/01/15
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

#include "dynamic/pointer.hxx"

#include <iostream>
#include <exception>
#include <stdexcept>


/** Test object announcing its destruction */
class swan {
    private:

    bool & croak;  /**< Well, this one isn't too musical ;-) */

    public:

    /**
     *  \brief  Another one born every minute
     *
     *  \param  song  Snuff-it song sung flag
     */
    swan(bool & song): croak(song) {
        croak = false;

        std::cerr
            << "Swan " << this << " was born"
            << std::endl;
    }

    /** Pushing up the daisies */
    ~swan() {
        if (true == croak)
            throw std::logic_error("zombie swan  8-(  (");

        croak = true;

        std::cerr
            << "Swan " << this << " died"
            << std::endl;
    }

};  // end of class swan


/**
 *  \brief  \c dynamic::unique_ptr unit test
 *
 *  The test function creates a new dynamic object and handles it via
 *  \c dynamic::unique_ptr.
 *  It checks whether the object was destroyed upon the pointer
 *  destruction, too.
 *
 *  \return 0 on UT passed, non-zero value as UT failed reason code
 */
int test_unique_ptr() {
    bool swan_down = false;

    {
        // Check construction
        dynamic::unique_ptr<swan> swan1(new swan(swan_down));

        if (!swan1.valid()) {
            std::cerr
                << "dynamic::unique_ptr test FAILED" << std::endl
                << "Reason: validity check"          << std::endl;

            return 1;
        }

        // Check copy-construction
        dynamic::unique_ptr<swan> swan2(swan1);

        if (swan1.valid() || !swan2.valid()) {
            std::cerr
                << "dynamic::unique_ptr test FAILED"     << std::endl
                << "Reason: copy constr. validity check" << std::endl;

            return 2;
        }

        // Check assignment
        dynamic::unique_ptr<swan> swan3;

        swan3 = swan2;

        if (swan2.valid() || !swan3.valid()) {
            std::cerr
                << "dynamic::unique_ptr test FAILED"   << std::endl
                << "Reason: assignment validity check" << std::endl;

            return 2;
        }
    }

    // The object is supposed to be destroyed by now
    if (!swan_down) {
        std::cerr
            << "dynamic::unique_ptr test FAILED" << std::endl
            << "Reason: the dynamic object wasn't destroyed with the pointer"
            << std::endl;

        return 64;
    }

    // Test passed
    std::cerr
        << "dynamic::unique_ptr test PASSED"
        << std::endl;

    return 0;
}


/** Smart pointers unit test */
int main_impl(int argc, char * const argv[]) {
    int exit_code = 0;

    // Test unique_ptr
    exit_code = test_unique_ptr();

    if (0 != exit_code) return exit_code;

    return 0;  // all tests passed
}

/** Exception-safeness wrapper */
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
