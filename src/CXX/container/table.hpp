#ifndef container__table_hpp
#define container__table_hpp

/**
 *  \brief  Multi-keyed table
 *
 *  Note that the key order is reversed, i.e. the right-most key
 *  represents the top-most layer of the table hierarchy.
 *  That's OK since the keys have in fact the same priority.
 *  Reversing the keys type list wouldn't affect the efficiency,
 *  directly, however indirectly it would since it would also
 *  be necessary to revert value lists (on run-time).
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2013/02/06
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

#include "value_list.hpp"

#include <map>
#include <stdexcept>


namespace container {

/**
 *  \brief  Multi-keyed table
 */
template <typename tlist, typename V>
class table {
    private:

    /** End iterator flag */
    typedef enum { iterator_end } iterator_end_t;

    public:

    class iterator;
    class const_iterator;

    typedef tlist keys_t;

    typedef V value_t;

    typedef table<typename tlist::head, V> subtab_t;

    typedef typename subtab_t::iterator       subtab_iterator;
    typedef typename subtab_t::const_iterator subtab_const_iterator;

    typedef std::map<typename tlist::tail, subtab_t> impl_t;

    typedef typename impl_t::iterator       level_iterator;
    typedef typename impl_t::const_iterator const_level_iterator;

    private:

    template <typename tab_impl_t, typename level_impl_t, typename sub_impl_t>
    class iterator_base;

    typedef iterator_base<impl_t, level_iterator, subtab_iterator> iterator_base_t;

    typedef iterator_base<const impl_t, const_level_iterator, subtab_const_iterator> const_iterator_base_t;

    public:

    class entry_keys {
        friend class iterator_base<impl_t, level_iterator, subtab_iterator>;

        friend class iterator_base<const impl_t, const_level_iterator, subtab_const_iterator>;

        private:

        //const value_list<tlist> m_impl;
        const_reference_list<tlist> m_impl;

        entry_keys(
            const typename subtab_t::entry_keys & keys,
            const typename tlist::tail & key):
        m_impl(keys, key) {}

        public:

        //inline operator const value_list<tlist> & () const {
#if (0)
        inline operator const_reference_list<tlist> & () {
            return m_impl;
        }
#endif

        inline operator const const_reference_list<tlist> & () const {
            return m_impl;
        }

        template <int key_index>
        inline const typename meta::get<tlist, key_index>::type & get() const {
            return const_reference<tlist, key_index>(m_impl);
        }

    };  // end of class entry_keys

    private:

    /** Iterator common base */
    template <typename tab_impl_t, typename level_impl_t, typename sub_impl_t>
    class iterator_base {
        protected:

        struct impl_t {
            level_impl_t lvl;
            level_impl_t lvl_end;
            sub_impl_t   prop;

            impl_t(): lvl(), lvl_end(), prop() {}

            impl_t(tab_impl_t & tab_impl):
                lvl(tab_impl.begin()),
                lvl_end(tab_impl.end()),
                prop(lvl == lvl_end ? sub_impl_t() : lvl->second.begin()) {}

            impl_t(tab_impl_t & tab_impl, iterator_end_t):
                lvl(tab_impl.end()), lvl_end(lvl), prop(subtab_t().end()) {}

            impl_t(const impl_t & orig):
                lvl(orig.lvl), lvl_end(orig.lvl_end), prop(orig.prop) {}

            impl_t(const level_impl_t & _lvl, const level_impl_t & _lvl_end, const sub_impl_t & _prop):
                lvl(_lvl), lvl_end(_lvl_end), prop(_prop) {}

            inline bool eq(const level_impl_t & _lvl, const sub_impl_t _prop) const {
                if (lvl == _lvl) {
                    if (lvl_end == lvl)
                        return true;

                    return prop == _prop;
                }

                return false;
            }

            inline bool operator == (const impl_t & that) const {
                return eq(that.lvl, that.prop);
            }

            inline bool operator != (const impl_t & that) const {
                return !eq(that.lvl, that.prop);
            }

        };  // end of struct impl_t

        impl_t m_impl;

        iterator_base(tab_impl_t & tab_impl):
            m_impl(tab_impl) {}

        iterator_base(tab_impl_t & tab_impl, iterator_end_t):
            m_impl(tab_impl, iterator_end) {}

        iterator_base(
            const level_impl_t & lvl,
            const level_impl_t & lvl_end,
            const sub_impl_t & prop):
            m_impl(lvl, lvl_end, prop) {}

        public:

        iterator_base(): m_impl() {}

        iterator_base(const iterator_base & orig): m_impl(orig.m_impl) {}

        inline iterator_base & operator = (const iterator_base & orig) {
            m_impl = orig.m_impl;

            return *this;
        }

        inline const entry_keys keys() const {
            return entry_keys(m_impl.prop.keys(), m_impl.lvl->first);
        }

        inline const V & value() const {
            return m_impl.prop.value();
        }

        inline bool operator == (const iterator_base & that) const {
            return m_impl == that.m_impl;
        }

        inline bool operator != (const iterator_base & that) const {
            return m_impl != that.m_impl;
        }

        iterator_base & operator ++ () {
            do {  // pragmatic do while (0) loop
                ++m_impl.prop;

                if (m_impl.lvl->second.end() != m_impl.prop)
                    break;

                ++m_impl.lvl;

                if (m_impl.lvl_end == m_impl.lvl)
                    break;

                m_impl.prop = m_impl.lvl->second.begin();

            } while (0);  // pragmatic loop break target

            return *this;
        }

        inline iterator_base operator ++ (int) {
            throw std::logic_error("NOT IMPLEMENTED: use pre-increment where possible");
        }

    };  // end of class iterator_base

    public:

    /** Table iterator */
    class iterator: public iterator_base_t {
        friend class table;
        friend class const_iterator;

        private:

        iterator(impl_t & impl):
            iterator_base_t(impl) {}

        iterator(impl_t & impl, iterator_end_t):
            iterator_base_t(impl, iterator_end) {}

        public:

        iterator(): iterator_base_t() {}

    };  // end of class iterator

    /** Constant table iterator */
    class const_iterator: public const_iterator_base_t {
        friend class table;

        public:

        using const_iterator_base_t::operator ==;
        using const_iterator_base_t::operator !=;
        using const_iterator_base_t::operator  =;

        private:

        const_iterator(impl_t & impl):
            const_iterator_base_t(impl) {}

        const_iterator(impl_t & impl, iterator_end_t):
            const_iterator_base_t(impl, iterator_end) {}

        const_iterator(const impl_t & impl):
            const_iterator_base_t(impl) {}

        const_iterator(const impl_t & impl, iterator_end_t):
            const_iterator_base_t(impl, iterator_end) {}

        public:

        const_iterator(): const_iterator_base_t() {}

        const_iterator(const iterator & iter):
            const_iterator_base_t(
                iter.m_impl.lvl,
                iter.m_impl.lvl_end,
                iter.m_impl.prop)
        {}

        inline const_iterator & operator = (const iterator & iter) {
            this->m_impl.lvl     = iter.m_impl.lvl;
            this->m_impl.lvl_end = iter.m_impl.lvl_end;
            this->m_impl.prop    = iter.m_impl.prop;

            return *this;
        }

        inline bool operator == (const iterator & that) {
            return this->m_impl.eq(that.m_impl.lvl, that.m_impl.prop);
        }

        inline bool operator != (const iterator & that) {
            return !(this->m_impl.eq(that.m_impl.lvl, that.m_impl.prop));
        }

    };  // end of class const_iterator

    private:

    impl_t m_impl;

    public:

    table() {}

    table(const table & orig): m_impl(orig.m_impl) {}

    inline bool empty() const {
        return m_impl.empty();
    }

    inline table operator = (const table & orig) {
        m_impl = orig.m_impl;

        return *this;
    }

    inline const V & operator [] (const value_list<tlist> & keys) const {
        typename impl_t::const_iterator subtab = m_impl.find(keys.tail());

        if (m_impl.end() == subtab) return V();

        return subtab->second[keys.head()];
    }

    V & operator [] (const value_list<tlist> & keys) {
        typedef typename tlist::tail key_t;

        const key_t & key = keys.tail();

        typename impl_t::iterator subtab = m_impl.lower_bound(key);

        if (m_impl.end() == subtab || key != subtab->first) {
            std::pair<const key_t, subtab_t> key_tab(key, subtab_t());

            subtab = m_impl.insert(subtab, key_tab);
        }

        return subtab->second[keys.head()];
    }

    inline const_level_iterator level_begin() const {
        return m_impl.begin();
    }

    inline level_iterator level_begin() {
        return m_impl.begin();
    }

    inline const_level_iterator level_end() const {
        return m_impl.end();
    }

    inline level_iterator level_end() {
        return m_impl.end();
    }

    inline const_level_iterator level_find(const typename keys_t::tail & key) const {
        return m_impl.find(key);
    }

    inline level_iterator level_find(const typename keys_t::tail & key) {
        return m_impl.find(key);
    }

    inline const_level_iterator level_lower_bound(const typename keys_t::tail & key) const {
        return m_impl.lower_bound(key);
    }

    inline level_iterator level_lower_bound(const typename keys_t::tail & key) {
        return m_impl.lower_bound(key);
    }

    inline std::pair<level_iterator, bool> level_insert(
        const typename keys_t::tail & key, const subtab_t & stab)
    {
        return m_impl.insert(typename impl_t::value_type(key, stab));
    }

    inline level_iterator level_insert(
        level_iterator position, const typename keys_t::tail & key, const subtab_t & stab)
    {
        return m_impl.insert(position, typename impl_t::value_type(key, stab));
    }

    inline void level_erase(const level_iterator pos) {
        return m_impl.erase(pos);
    }

    inline const_iterator begin() const {
        return const_iterator(m_impl);
    }

    inline iterator begin() {
        return iterator(m_impl);
    }

    inline const_iterator end() const {
        return const_iterator(m_impl, iterator_end);
    }

    inline iterator end() {
        return iterator(m_impl, iterator_end);
    }

};  // end of template class table

template <typename tlist, typename V>
inline bool operator == (
    const typename table<tlist, V>::iterator & iter,
    const typename table<tlist, V>::const_iterator & const_iter)
{
    return const_iter == iter;
}

template <typename tlist, typename V>
inline bool operator != (
    const typename table<tlist, V>::iterator & iter,
    const typename table<tlist, V>::const_iterator & const_iter)
{
    return const_iter != iter;
}


/**
 *  \brief  Nullary table (one value at most)
 */
template <typename V>
class table<type_list0, V> {
    private:

    /** End iterator flag */
    typedef enum { iterator_end } iterator_end_t;

    public:

    class level_iterator;
    class const_level_iterator;

    class iterator;
    class const_iterator;

    typedef V value_t;

    struct impl_t {
        V    val;
        bool empty;

        impl_t(): val(), empty(true) {}

        impl_t(const V & _val): val(_val), empty(false) {}

        impl_t(const impl_t & orig): val(orig.val), empty(orig.empty) {}

    };  // end of struct impl_t

    private:

    template <typename tab_impl_t, typename val_t>
    class level_iterator_base;

    typedef level_iterator_base<impl_t, V> level_iterator_base_t;

    typedef level_iterator_base<const impl_t, const V> const_level_iterator_base_t;

    template <typename tab_impl_t, typename level_impl_t>
    class iterator_base;

    typedef iterator_base<impl_t, level_iterator> iterator_base_t;

    typedef iterator_base<const impl_t, const_level_iterator> const_iterator_base_t;

    public:

    class entry_keys {
        friend class iterator_base<impl_t, level_iterator>;

        friend class iterator_base<const impl_t, const_level_iterator>;

        private:

        typedef const_reference_list<type_list0> keys_impl_t;

        //const value_list<type_list0> m_impl;
        keys_impl_t m_impl;

        entry_keys(): m_impl(keys_impl_t::head_t(), keys_impl_t::tail_t()) {}

        public:

#if (0)
        inline operator const_reference_list<type_list0> & () {
            return m_impl;
        }
#endif

        //inline operator const value_list<type_list0> & () const {
        inline operator const const_reference_list<type_list0> & () const {
            return m_impl;
        }

    };  // end of class entry_keys

    private:

    /** Level iterator common base */
    template <typename tab_impl_t, typename val_t>
    class level_iterator_base {
        protected:

        struct impl_t {
            val_t * value;
            bool    at_end;

            impl_t(): value(NULL), at_end(true) {}

            impl_t(val_t * _val, bool _end):
                value(_val), at_end(_end) {}

        };  // end of struct impl

        impl_t m_impl;

        level_iterator_base(tab_impl_t & tab_impl):
            m_impl(&tab_impl.val, tab_impl.empty) {}

        level_iterator_base(tab_impl_t & tab_impl, iterator_end_t):
            m_impl(NULL, true) {}

        level_iterator_base(val_t * val, bool at_end):
            m_impl(val, at_end) {}

        public:

        level_iterator_base() {}

        level_iterator_base(const level_iterator_base & orig): m_impl(orig.m_impl) {}

        inline level_iterator_base & operator = (const level_iterator_base & rval) {
            m_impl = rval.m_impl;

            return *this;
        }

        inline val_t & value() const throw(std::runtime_error) {
            if (m_impl.at_end)
                throw std::runtime_error("Dereference of an end iterator");

            return *m_impl.value;
        }

        inline bool operator == (const level_iterator_base & that) const {
            return m_impl.at_end == that.m_impl.at_end;
        }

        inline bool operator != (const level_iterator_base & that) const {
            return !(*this == that);
        }

        inline level_iterator_base & operator ++ () {
            m_impl.at_end = true;

            return *this;
        }

        inline level_iterator_base operator ++ (int) {
            throw std::logic_error("NOT IMPLEMENTED: use pre-increment where possible");
        }

    };  // end of class level_iterator_base

    public:

    /** Table level iterator */
    class level_iterator: public level_iterator_base_t {
        friend class table<type_list0, V>;
        friend class iterator;
        friend class const_level_iterator;

        level_iterator(impl_t & impl):
            level_iterator_base_t(impl) {}

        level_iterator(impl_t & impl, iterator_end_t):
            level_iterator_base_t(impl, iterator_end) {}

        public:

        level_iterator(): level_iterator_base_t() {}

    };  // end of class level_iterator

    /** Constant table level iterator */
    class const_level_iterator: public const_level_iterator_base_t {
        friend class table<type_list0, V>;
        friend class const_iterator;

        public:

        using const_level_iterator_base_t::operator ==;
        using const_level_iterator_base_t::operator !=;
        using const_level_iterator_base_t::operator  =;

        private:

        const_level_iterator(const impl_t & tab_impl):
            const_level_iterator_base_t(tab_impl) {}

        const_level_iterator(const impl_t & tab_impl, iterator_end_t):
            const_level_iterator_base_t(tab_impl, iterator_end) {}

        const_level_iterator(impl_t & tab_impl):
            const_level_iterator_base_t(tab_impl) {}

        const_level_iterator(impl_t & tab_impl, iterator_end_t):
            const_level_iterator_base_t(tab_impl, iterator_end) {}

        public:

        const_level_iterator(): const_level_iterator_base_t() {}

        const_level_iterator(const const_level_iterator & orig):
            const_level_iterator_base_t(orig) {}

        const_level_iterator(const level_iterator & iter):
            const_level_iterator_base_t(iter.m_impl.value, iter.m_impl.at_end) {}

        inline const_level_iterator & operator = (const level_iterator & that) {
            this->m_impl.value  = that.m_impl.value;
            this->m_impl.at_end = that.m_impl.at_end;

            return *this;
        }

        inline bool operator != (const level_iterator & that) const {
            return m_impl.at_end xor that.m_impl.at_end;
        }

        inline bool operator == (const level_iterator & that) const {
            return !(*this != that);
        }

    };  // end of class const_level_iterator

    private:

    /** Table iterator base */
    template <typename tab_impl_t, typename level_impl_t>
    class iterator_base {
        protected:

        level_impl_t m_impl;

        iterator_base(tab_impl_t & tab_impl): m_impl(tab_impl) {}

        iterator_base(tab_impl_t & tab_impl, iterator_end_t):
            m_impl(tab_impl, iterator_end) {}

        iterator_base(const level_impl_t & impl): m_impl(impl) {}

        public:

        iterator_base() {}

        iterator_base(const iterator_base & orig): m_impl(orig.m_impl) {}

        inline const entry_keys keys() const {
            return entry_keys();
        }

        inline const V & value() const {
            return m_impl.value();
        }

        inline bool operator == (const iterator_base & that) const {
            return m_impl == that.m_impl;
        }

        inline bool operator != (const iterator_base & that) const {
            return !(*this == that);
        }

        inline iterator_base & operator ++ () {
            ++m_impl;

            return *this;
        }

        inline iterator_base operator ++ (int) {
            throw std::logic_error("NOT IMPLEMENTED: use pre-increment where possible");
        }

    };  // end of class iterator_base

    public:

    /** Table iterator */
    class iterator: public iterator_base_t {
        friend class table<type_list0, V>;
        friend class const_iterator;

        iterator(impl_t & impl):
            iterator_base_t(impl) {}

        iterator(impl_t & impl, iterator_end_t):
            iterator_base_t(impl, iterator_end) {}

        public:

        iterator(): iterator_base_t() {}

    };  // end of class iterator

    /** Constant table iterator */
    class const_iterator: public const_iterator_base_t {
        friend class table<type_list0, V>;

        public:

        using const_iterator_base_t::operator ==;
        using const_iterator_base_t::operator !=;
        using const_iterator_base_t::operator  =;

        private:

        const_iterator(impl_t & impl):
            const_iterator_base_t(impl) {}

        const_iterator(impl_t & impl, iterator_end_t):
            const_iterator_base_t(impl, iterator_end) {}

        const_iterator(const impl_t & impl):
            const_iterator_base_t(impl) {}

        const_iterator(const impl_t & impl, iterator_end_t):
            const_iterator_base_t(impl, iterator_end) {}

        public:

        const_iterator(): const_iterator_base_t() {}

        const_iterator(const iterator & iter):
            const_iterator_base_t(iter.m_impl) {}

        inline const_iterator & operator = (const iterator & iter) {
            this->m_impl = iter.m_impl;

            return *this;
        }

        inline bool operator == (const iterator & that) const {
            return m_impl == that.m_impl;
        }

        inline bool operator != (const iterator & that) const {
            return !(*this == that);
        }

    };  // end of class const_iterator

    private:

    impl_t m_impl;

    public:

    table() {}

    table(const table & orig): m_impl(orig.m_impl) {}

    inline bool empty() const {
        return m_impl.empty;
    }

    inline table & operator = (const table & orig) {
        m_impl = orig.m_impl;

        return *this;
    }

    inline const V & get() const {
        return m_impl.val;
    }

    inline const V & operator [] (const value_list<type_list0> &) const {
        return get();
    }

    inline V & set(const V & val) {
        m_impl.empty = false;

        return m_impl.val = val;
    }

    inline V & operator [] (const value_list<type_list0> &) {
        m_impl.empty = false;

        return m_impl.val;
    }

    inline V & operator = (const V & val) {
        return set(val);
    }

    inline const_level_iterator level_begin() const {
        return const_level_iterator(m_impl);
    }

    inline level_iterator level_begin() {
        return level_iterator(m_impl);
    }

    inline const_level_iterator level_end() const {
        return const_level_iterator(m_impl, const_level_iterator::end);
    }

    inline level_iterator level_end() {
        return level_iterator(m_impl, level_iterator::end);
    }

    inline void level_erase(const level_iterator pos) {
        if (level_end() != pos) {
            m_impl.val = V();

            m_impl.empty = true;
        }
    }

    inline const_iterator begin() const {
        return const_iterator(m_impl);
    }

    inline iterator begin() {
        return iterator(m_impl);
    }

    inline const_iterator end() const {
        return const_iterator(m_impl, iterator_end);
    }

    inline iterator end() {
        return iterator(m_impl, iterator_end);
    }

};  // end of template class table

/**
 *  \brief  Stream output operator
 *
 *  \param  stream  Output stream
 *  \param  tab     Table
 *
 *  \return Stream reference
 */
template <typename V>
inline std::ostream & operator << (std::ostream & stream, const table<type_list0, V> & tab) {
    return stream << tab.get();
}

/**
 *  \brief  Comparison of const. and non-const iterator
 *
 *  \param  iter        Iterator
 *  \param  const_iter  Iterator
 *
 *  \retval true   \c iter position equals \c const_iter
 *  \retval false  otherwise
 */
template <typename V>
inline bool operator == (
    const typename table<type_list0, V>::iterator & iter,
    const typename table<type_list0, V>::const_iterator & const_iter)
{
    return const_iter == iter;
}


/**
 *  \brief  Comparison of const. and non-const iterator (negative)
 *
 *  \param  iter        Iterator
 *  \param  const_iter  Iterator
 *
 *  \retval false  \c iter position equals \c const_iter
 *  \retval true   otherwise
 */
template <typename V>
inline bool operator != (
    const typename table<type_list0, V>::iterator & iter,
    const typename table<type_list0, V>::const_iterator & const_iter)
{
    return const_iter != iter;
}


/** Table cutter */
template <typename klist, typename slist, typename V>
class table_cutter {
    public:

    typedef table<klist, V> itab_t;

    typedef table_cutter<typename klist::head, typename slist::head, V> subcut_t;

    // Note that the following is necessary since cut may ommit certain keys (see key_eq, below)
    typedef meta::type_list<typename subcut_t::keys_t, typename klist::tail> keys_t;

    typedef table<keys_t, V> otab_t;

    table_cutter(const itab_t & itab, const value_list<slist> & selectors, otab_t & otab) {
        typename itab_t::const_level_iterator kv = itab.level_begin();

        for (; kv != itab.level_end(); ++kv)
            if (selectors.tail()(kv->first)) {
                typename otab_t::level_iterator stab = otab.level_lower_bound(kv->first);

                if (otab.level_end() == stab || stab->first != kv->first) {
                    typename otab_t::subtab_t tab;

                    stab = otab.level_insert(stab, kv->first, tab);
                }

                subcut_t(kv->second, selectors.head(), stab->second);

                if (stab->second.empty())
                    otab.level_erase(stab);
            }
    }

};  // end of template class table_cutter

/** Table cutter (recursion end) */
template <typename V>
class table_cutter<type_list0, type_list0, V> {
    public:

    typedef type_list0 keys_t;

    typedef V value_t;

    typedef table<keys_t, V> itab_t;

    typedef itab_t otab_t;

    table_cutter(const itab_t & itab, const value_list<type_list0> & selector, otab_t & otab) {
        otab = itab;
    }

};  // end of template class table_cutter (recursion fixed point specialisation)

/** Table cut key selector for key fixation */
template <typename K>
class key_eq {
    public:

    const K key;

    key_eq(const K & _key): key(_key) {}

    inline bool operator () (const K & _key) const {
        return key == _key;
    }

};  // end of template class key_eq

/** Table cutter (with key_eq level selector) */
template <typename klist, typename slist_head, typename V>
class table_cutter<klist, meta::type_list<slist_head, key_eq<typename klist::tail> >, V> {
    public:

    typedef V value_t;

    typedef key_eq<typename klist::tail> selector_t;

    typedef meta::type_list<slist_head, selector_t> slist;

    typedef table<klist, V> itab_t;

    typedef table_cutter<typename klist::head, slist_head, V> subcut_t;

    typedef typename subcut_t::keys_t keys_t;

    typedef table<keys_t, V> otab_t;

    table_cutter(const itab_t & itab, const value_list<slist> & selectors, otab_t & otab) {
        const selector_t & selector = selectors.tail();

        typename itab_t::const_level_iterator kv = itab.level_find(selector.key);

        if (itab.level_end() != kv)
            subcut_t(kv->second, selectors.head(), otab);
    }

};  // end of template class table_cutter (with key_eq level selector)

/** Table cut key selector (idempotent) */
template <typename K>
class key_ok {
    public:

    key_ok() {}

    inline bool operator () (const K & _key) const { return true; }

};  // end of template class key_ok


/**
 *  \brief  Cut table
 *
 *  The function templateuses \ref table_cutter to create table cut.
 *
 *  \param  t  Table
 *  \param  s  Selectors
 *
 *  \return  Table cut
 */
template <typename tab, typename slist>
inline
typename table_cutter<typename tab::keys_t, slist, typename tab::value_t>::otab_t
table_cut(const tab & t, const value_list<slist> & s) {
    typedef table_cutter<typename tab::keys_t, slist, typename tab::value_t> cutter_t;

    typename cutter_t::otab_t ot;

    cutter_t(t, s, ot);

    return ot;
}

}  // end of namespace container

#endif  // end of #ifndef container__table_hpp
