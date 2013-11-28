#include "container/table.hxx"

#include <iostream>
#include <sstream>
#include <list>


typedef type_list3(int, std::string, char) keys_t;

typedef std::string value_t;

typedef container::table<keys_t, value_t> table_t;


typedef container::key_ok<int>         int_ok_t;
typedef container::key_eq<int>         int_eq_t;
typedef container::key_eq<std::string> str_eq_t;
typedef container::key_ok<char>        char_ok_t;
typedef container::key_eq<char>        char_eq_t;

typedef type_list3(int_ok_t, str_eq_t, char_ok_t) selector_types_t;
typedef type_list3(int_eq_t, str_eq_t, char_ok_t) selector_types_2_t;
typedef type_list3(int_eq_t, str_eq_t, char_eq_t) selector_types_3_t;

typedef container::value_list<selector_types_t>   selectors_t;
typedef container::value_list<selector_types_2_t> selectors_2_t;
typedef container::value_list<selector_types_3_t> selectors_3_t;

typedef type_list2(int, char) cut_keys_t;
typedef type_list1(char)      cut_keys_2_t;
typedef type_list0            cut_keys_3_t;

typedef container::table<cut_keys_t,   value_t> table_cut_t;
typedef container::table<cut_keys_2_t, value_t> table_cut_2_t;
typedef container::table<cut_keys_3_t, value_t> table_cut_3_t;


template <typename tlist>
class vlist2str {
    public:

    typedef std::list<std::string> slist_t;

    private:

    typedef container::value_list<tlist> vlist_t;

    slist_t m_impl;

    public:

    vlist2str(const vlist_t & vlist) {
        vlist2str<typename tlist::head> head_str(vlist.head());

        m_impl = head_str.list();

        std::stringstream tail;

        tail << vlist.tail();

        m_impl.push_back(tail.str());
    }

    const slist_t & list() const { return m_impl; }

};  // end of template class vlist2str

template <>
class vlist2str<type_list0> {
    public:

    typedef std::list<std::string> slist_t;

    private:

    static const slist_t s_empty;

    public:

    vlist2str(const container::value_list<type_list0> & vlist) {}

    const slist_t & list() const { return s_empty; }

};  // end of template class entryKeys2str (recursion fixed point)

const typename vlist2str<type_list0>::slist_t vlist2str<type_list0>::s_empty;


template <typename tlist>
class reflist2str {
    public:

    typedef std::list<std::string> slist_t;

    private:

    typedef container::const_reference_list<tlist> rlist_t;

    slist_t m_impl;

    public:

    reflist2str(const rlist_t & rlist) {
        reflist2str<typename tlist::head> head_str(rlist.head());

        m_impl = head_str.list();

        std::stringstream tail;

        tail << rlist.tail();

        m_impl.push_back(tail.str());
    }

    const slist_t & list() const { return m_impl; }

};  // end of template class vlist2str

template <>
class reflist2str<type_list0> {
    public:

    typedef std::list<std::string> slist_t;

    private:

    static const slist_t s_empty;

    public:

    reflist2str(const container::const_reference_list<type_list0> & rlist) {}

    const slist_t & list() const { return s_empty; }

};  // end of template class entryKeys2str (recursion fixed point)

const typename reflist2str<type_list0>::slist_t reflist2str<type_list0>::s_empty;


template <typename tlist, typename V>
void printTab(const container::table<tlist, V> & tab) {
    typedef container::table<tlist, V> tab_t;

    std::cout << "Table printout:" << std::endl;

    typename tab_t::const_iterator i = tab.begin();

    for (; i != tab.end(); ++i) {
#if (0)
        tab_t::entry_keys keys = i.keys();

        int         k0 = keys.get<0>();
        std::string k1 = keys.get<1>();
        char        k2 = keys.get<2>();
#endif

        //const container::value_list<tlist> & keys = i.keys();
        //container::const_reference_list<tlist> keys = i.keys();
        const container::const_reference_list<tlist> & keys = i.keys();

        //vlist2str<tlist> keys_str(keys);
        reflist2str<tlist> keys_str(keys);

        //const typename vlist2str<tlist>::slist_t & keys_str_list = keys_str.list();
        const typename reflist2str<tlist>::slist_t & keys_str_list = keys_str.list();

        //typename vlist2str<tlist>::slist_t::const_iterator key = keys_str_list.begin();
        typename reflist2str<tlist>::slist_t::const_iterator key = keys_str_list.begin();

        std::cout << "@[";

        while (keys_str_list.end() != key) {
            std::cout << *key;

            ++key;

            if (keys_str_list.end() == key) break;

            std::cout << ", ";
        }

        std::cout << "] == " << i.value() << std::endl;
    }

    std::cout << "Table printout end" << std::endl;
}


int main() {
    table_t my_tab;

    my_tab[vl_item(int, 12), vl_item(std::string, "bibi"), vl_item(char, 'p')] = "HU!";

    std::cout << "my_tab[vl_item(int, 12), vl_item(std::string, \"bibi\"), vl_item(char, 'p')] == \""
              << my_tab[vl_item(int, 12), vl_item(std::string, "bibi"), vl_item(char, 'p')]
              << "\"" << std::endl;

    my_tab[vl_item(int, 12), vl_item(std::string, "nuoi"), vl_item(char, 'p')] = "HI!";
    my_tab[vl_item(int, 12), vl_item(std::string, "nuoi"), vl_item(char, 'v')] = "HE!";
    my_tab[vl_item(int, 42), vl_item(std::string, "pitu"), vl_item(char, 'v')] = "HU!";
    my_tab[vl_item(int, 40), vl_item(std::string, "pitu"), vl_item(char, 'v')] = "HO!";

    printTab<keys_t, value_t>(my_tab);

    selectors_t my_selectors = (
        vl_item(int_ok_t,  int_ok_t()),
        vl_item(str_eq_t,  str_eq_t("pitu")),
        vl_item(char_ok_t, char_ok_t())
    );

    table_cut_t my_cut = container::table_cut<table_t, selector_types_t>(my_tab, my_selectors);

    printTab<cut_keys_t, value_t>(my_cut);

    selectors_2_t my_selectors_2 = (
        vl_item(int_eq_t,  int_eq_t(12)),
        vl_item(str_eq_t,  str_eq_t("nuoi")),
        vl_item(char_ok_t, char_ok_t())
    );

    table_cut_2_t my_cut_2 = container::table_cut<table_t, selector_types_2_t>(my_tab, my_selectors_2);

    printTab<cut_keys_2_t, value_t>(my_cut_2);

    selectors_3_t my_selectors_3 = (
        vl_item(int_eq_t,  int_eq_t(12)),
        vl_item(str_eq_t,  str_eq_t("nuoi")),
        vl_item(char_eq_t, char_eq_t('r'))
    );

    table_cut_3_t my_cut_3 = container::table_cut<table_t, selector_types_3_t>(my_tab, my_selectors_3);

    printTab<cut_keys_3_t, value_t>(my_cut_3);

    return 0;
}
