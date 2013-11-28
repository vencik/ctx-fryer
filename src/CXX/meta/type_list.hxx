#ifndef meta__type_list_hxx
#define meta__type_list_hxx

/**
 *  \brief  Type list
 *
 *  Formal structure bearing solely types definitions.
 *  All instances are empty and therefore optimise out.
 *  It's only purpose is to keep the types definitions for compile-time
 *  computations (aka meta-programming).
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/01/20
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

#include <typeinfo>


/** \cond */
#define type_list0 meta::type_list<meta::null_type, meta::null_type >
#define type_list1(T1) meta::type_list<type_list0, T1 >
#define type_list2(T1, T2) meta::type_list<type_list1(T1), T2 >
#define type_list3(T1, T2, T3) meta::type_list<type_list2(T1, T2), T3 >
#define type_list4(T1, T2, T3, T4) meta::type_list<type_list3(T1, T2, T3), T4 >
#define type_list5(T1, T2, T3, T4, T5) meta::type_list<type_list4(T1, T2, T3, T4), T5 >
#define type_list6(T1, T2, T3, T4, T5, T6) meta::type_list<type_list5(T1, T2, T3, T4, T5), T6 >
#define type_list7(T1, T2, T3, T4, T5, T6, T7) meta::type_list<type_list6(T1, T2, T3, T4, T5, T6), T7 >
#define type_list8(T1, T2, T3, T4, T5, T6, T7, T8) meta::type_list<type_list7(T1, T2, T3, T4, T5, T6, T7), T8 >

/*  Add more such macra above if required */
/** \endcond */


namespace meta {

/** Recursion fixed point */
struct null_type {};


/** Type list (generic form) */
template <typename T1, typename T2>
struct type_list {
    typedef T1 head;
    typedef T2 tail;
    enum { length = head::length + 1 };
};

/** Type list (empty) */
template <>
struct type_list<null_type, null_type> {
    typedef null_type head;
    typedef null_type tail;
    enum { length = 0 };
};


/** Type list: reversion */
template <typename tlist, typename rtlist = type_list0>
struct reverse {
    typedef typename tlist::tail tail_t;
    typedef type_list<rtlist, tail_t> rhead_t;
    typedef typename reverse<typename tlist::head, rhead_t>::list list;
};

/** Type list: reversion (recursion fixed point) */
template <typename rtlist>
struct reverse<type_list0, rtlist> {
    typedef rtlist list;
};


/** Type list: join */
template <typename tlist1, typename tlist2>
struct join {
    typedef typename join<tlist1, typename tlist2::head>::list head;
    typedef type_list<head, typename tlist2::tail> list;
};

/** Type list: join (empty list) */
template <typename tlist>
struct join<tlist, type_list0> {
    typedef tlist list;
};


/** Type list: grep -v <type> */
template <typename tlist, typename T>
struct grepv {
    typedef typename grepv<typename tlist::head, T>::list head;
    typedef type_list<head, typename tlist::tail> list;
};

/**
 *  \brief  Type list: grep -v <type> (tail match)
 *
 *  Note that grepv<null_type, T>::list == null_type.
 *  Therefore, grepv<type_list0, null_type>::list == null_type.
 *  This is an inherent indication of invalidity of such
 *  a grep.
 */
template <typename tlist, typename T>
struct grepv<type_list<tlist, T>, T> {
    typedef grepv<tlist, T> list;
};

/**
 *  \brief  Type list: grep -v <type> (recursion fixed point)
 *
 *  grepv<null_type, T>::list == null_type
 */
template <typename T>
struct grepv<null_type, T> {
    typedef null_type list;
};


/** Type getter (generic inverted) */
template <typename tlist, int index>
struct _get {
    typedef typename _get<typename tlist::head, index - 1>::type type;
};

/** Type getter (OK inverted) */
template <typename tlist>
struct _get<tlist, 0> {
    typedef typename tlist::tail type;
};

/** Type getter (out-of-bounds inverted) */
template <int index>
struct _get<null_type, index> {
    typedef null_type type;
};

/** Type getter */
template <typename tlist, int index>
struct get {
    typedef typename _get<tlist, tlist::length - 1 - index>::type type;
};


/** Type index (generic) */
template <typename tlist, typename T, int default_value = tlist::length>
struct index {
    typedef index<typename tlist::head, T, default_value> prev;
    enum { value = prev::value };
};

/** Type index (OK) */
template <typename tlist, int default_value>
struct index<tlist, typename tlist::tail, default_value> {
    typedef index<typename tlist::head, typename tlist::tail, default_value> prev;
    enum { _val  = tlist::length - 1 };
    enum { value = default_value == prev::value ? _val : prev::value };
};

/** Type index (out-of-bounds) */
template <typename T, int default_value>
struct index<null_type, T, default_value> {
    enum { value = default_value };
};


/** Type info index search (prototype) */
template <typename tlist>
int typeid2index(const std::type_info& tinfo, int max_index = tlist::length);

/** Type info index search (generic) */
template <typename tlist>
inline int typeid2index(const std::type_info& tinfo, int max_index) {
    int index = typeid2index<typename tlist::head>(tinfo, max_index);

    if (index < max_index) return index;

    if (tinfo == typeid(typename tlist::tail)) return tlist::length - 1;

    return max_index;
}

/** Type info index search (done) */
template <>
inline int typeid2index<null_type>(const std::type_info& tinfo, int max_index) {
    return max_index;
}

}  // end of namespace meta

#endif  // end of #ifndef meta__type_list_hxx
