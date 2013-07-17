#ifndef container__value_list_hpp
#define container__value_list_hpp

/**
 *  \brief  Value list
 *
 *  List of values of different types.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/01/31
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

#include "meta/type_list.hpp"


/** Value list type shortcut for typelist */
#define vlist(types) container::value_list<types>

/** Value list type shortcut for explicit types */
#define vlist_of(len, types ...) container::value_list<meta::type_list ## len(types)>

/** Value list item type shortcut */
#define vl_item(type, value) container::item<type>(value)

/** Value list item getter */
#define vlist_at(tlist, vlist, index) container::value_list_get<tlist, index>(vlist)


namespace container {

/** Value list */
template <typename tlist>
class value_list {
    public:

    typedef tlist base_t;

    typedef value_list<typename tlist::head> head_t;

    typedef typename tlist::tail tail_t;

    private:

    head_t m_head;
    tail_t m_tail;

    public:

    value_list(): m_head(), m_tail() {}

    value_list(const head_t & head, const tail_t & tail):
        m_head(head), m_tail(tail) {}

    value_list(const value_list & orig):
        m_head(orig.m_head), m_tail(orig.m_tail) {}

    inline value_list & operator = (const value_list & rval) {
        m_head = rval.m_head;
        m_tail = rval.m_tail;

        return *this;
    }

    inline const head_t & head() const { return m_head; }

    inline head_t & head() { return m_head; }

    inline const tail_t & tail() const { return m_tail; }

    inline tail_t & tail() { return m_tail; }

};  // end of template class value_list

/** Value list (recursion fixed point) */
template <>
class value_list<meta::null_type> {
    public:

    value_list() {}

    value_list(const meta::null_type & null) {}

    value_list(const value_list & orig) {}

    inline value_list & operator = (const value_list & rval) {
        return *this;
    }

};  // end of template class spec. value_list<meta::null_type>


/** Constant references list */
template <typename tlist>
class const_reference_list {
    public:

    typedef tlist base_t;

    typedef const_reference_list<typename tlist::head> head_t;

    typedef typename tlist::tail tail_t;

    private:

    const head_t m_head;

    const tail_t & m_tail;

    public:

    const_reference_list(const head_t & head, const tail_t & tail):
        m_head(head), m_tail(tail) {}

    const_reference_list(const const_reference_list & orig):
        m_head(orig.m_head), m_tail(orig.m_tail) {}

    inline const head_t & head() const { return m_head; }

    inline const tail_t & tail() const { return m_tail; }

};  // end of template class const_reference_list

/** Constant references list (recursion fixed point) */
template <>
class const_reference_list<meta::null_type> {
    public:

    const_reference_list() {}

    const_reference_list(const meta::null_type & null) {}

    const_reference_list(const const_reference_list & orig) {}

};  // end of template class spec. const_reference_list<meta::null_type>


/** Value container item */
template <typename T>
class item {
    public:

    typedef value_list<type_list1(T)> list_t;

    private:

    T m_impl;

    public:

    item(const T & value): m_impl(value) {}

    inline operator const T & () const { return m_impl; }

    inline operator list_t () const {
        return list_t(value_list<type_list0>(), m_impl);
    }

};  // end of template class item

/**
 *  \brief  List items separating comma operator
 *
 *  \param  item1  List item
 *  \param  item2  List item
 *
 *  \return List containing the items
 */
template <typename T1, typename T2>
inline value_list<type_list2(T1, T2)>
operator , (const item<T1> & item1, const item<T2> & item2) {
    return value_list<type_list2(T1, T2)>(item1, item2);
}

/**
 *  \brief  List tail item separating comma operator
 *
 *  \param  vlist  List
 *  \param  item   List tail
 *
 *  \return List containing all the items
 */
template <typename tlist, typename T>
inline value_list<meta::type_list<tlist, T> >
operator , (const value_list<tlist> & vlist, const item<T> & item) {
    return value_list<meta::type_list<tlist, T> >(vlist, item);
}


/**
 *  \brief  Constant value reference (inverted index)
 */
template <typename tlist, int i>
class _const_reference {
    public:

    typedef typename meta::_get<tlist, i>::type item_t;

    private:

    typedef _const_reference<typename tlist::head, i - 1> head_t;

    const item_t & m_impl;

    public:

    _const_reference(const value_list<tlist> & vlist):
        m_impl(head_t(vlist.head())) {}

    _const_reference(const const_reference_list<tlist> & rlist):
        m_impl(head_t(rlist.head())) {}

    inline operator const item_t & () const { return m_impl; }

};  // end of template struct _const_reference

/**
 *  \brief  Constant value reference (OK, inverted index)
 */
template <typename tlist>
class _const_reference<tlist, 0> {
    public:

    typedef typename tlist::tail item_t;

    private:

    const item_t & m_impl;

    public:

    _const_reference(const value_list<tlist> & vlist):
        m_impl(vlist.tail()) {}

    _const_reference(const const_reference_list<tlist> & rlist):
        m_impl(rlist.tail()) {}

    inline operator const item_t & () const { return m_impl; }

};  // end of template struct _const_reference partial specialisation

/**
 *  \brief  Constant value reference (out-of-bounds, inverted index)
 */
template <int i>
class _const_reference<meta::null_type, i> {
    private:

    /** Instantiation forbidden */
    _const_reference() {}

};  // end of template struct _const_reference partial specialisation

/**
 *  \brief  Constant value reference
 */
template <typename tlist, int i>
class const_reference {
    private:

    typedef _const_reference<tlist, tlist::length - 1 - i> impl_t;

    public:

    typedef typename impl_t::item_t item_t;

    private:

    impl_t m_impl;

    public:

    const_reference(const value_list<tlist> & vlist): m_impl(vlist) {}

    const_reference(const const_reference_list<tlist> & rlist): m_impl(rlist) {}

    inline operator const item_t & () const { return m_impl; }

};  // end of template class const_reference


/**
 *  \brief  Value list getter
 *
 *  \param  vlist  Value list
 *
 *  \return i-th value from the list
 */
template <typename tlist, int i>
inline const typename meta::get<tlist, i>::type &
value_list_get(const value_list<tlist> & vlist) {
    return const_reference<tlist, i>(vlist);
}


/**
 *  \brief  Constant references list getter
 *
 *  \param  rlist  Constant references list
 *
 *  \return i-th reference from the list
 */
template <typename tlist, int i>
inline const typename meta::get<tlist, i>::type &
const_reference_list_get(const const_reference_list<tlist> & rlist) {
    return const_reference<tlist, i>(rlist);
}

}  // end of namespace container

#endif  // end of #ifndef container__value_list_hpp
