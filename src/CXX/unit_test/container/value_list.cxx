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
