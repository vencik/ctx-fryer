/**
 *  \brief  Value list unit test
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/11/14
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

#include "container/value_list.hxx"

#include <iostream>


typedef type_list4(unsigned int, char, double, char) my_types_t;

int main() {
    vlist(my_types_t) l1 = (
        vl_item(unsigned int, 123),
        vl_item(char,         'd'),
        vl_item(double,       0.4),
        vl_item(char,         'q')
    );

    unsigned ui = vlist_at(my_types_t, l1, 0);
    char     c1 = vlist_at(my_types_t, l1, 1);
    double   fl = vlist_at(my_types_t, l1, 2);
    char     c2 = vlist_at(my_types_t, l1, 3);

    std::cout << "ui == " << ui << std::endl;
    std::cout << "c1 == " << c1 << std::endl;
    std::cout << "fl == " << fl << std::endl;
    std::cout << "c2 == " << c2 << std::endl;

    return 0;
}
