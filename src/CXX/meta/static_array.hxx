#ifndef meta__static_array_hxx
#define meta__static_array_hxx

/**
 *  \brief  Static array with template initialiser
 *
 *  Templates allowing for static arrays compile-time initialisation.
 *  Uses C++11 variadic templates.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/06/05
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

#include "config.hxx"

#include <stdexcept>


namespace meta {

#ifdef HAVE_CXX11  // C++11 required for variadic templates

/** Type base */
template <typename T>
class base {
    private:

    /** Compile-time initialised array (implementation) */
    template <T... args>
    struct static_array_impl {
        static const T data[sizeof...(args)];  /**< Data */
    };  // end of template class static_array_impl

    /** Initialiser */
    template <size_t N, template <size_t> class F, T... args>
    struct static_array_init {
        typedef
            typename static_array_init<N - 1, F, F<N>::value, args...>::values
            values;
    };  // end of template struct static_array_init

    /** Initialiser (recursion fixed point) */
    template <template <size_t> class F, T... args>
    struct static_array_init<0, F, args...> {
        typedef static_array_impl<F<0>::value, args...> values;
    };  // end of template struct static_array_init<0, ...>

    public:

    /**
     *  \brief  Compile-time initialised array
     *
     *  The array is initialised via the \c F meta-functor on compile-time.
     *  The functor is a template type that for any template argument \c N
     *  defines (an enum with) value named \c value.
     *  \c F<N>::value shall be stored in the array at index \c N.
     *
     *  \tparam  T  Base type
     *  \tparam  N  Array size
     *  \tparam  F  Initialisation meta-functor
     */
    template <size_t N, template <size_t> class F>
    class static_array {
        private:

        /** Implementation */
        typedef typename static_array_init<N - 1, F>::values impl_t;

        public:

        /** Static array accessor */
        inline static const T & at(size_t i) throw(std::range_error) {
            if (!(i < N))
                throw std::range_error(
                    "meta::base::static_array: out of range");

            return impl_t::data[i];
        }

        /** Array accessor */
        inline const T & operator [] (size_t i) const { return at(i); }

    };  // end of template struct static_array

};  // end of template struct base

// static array implementation initialisation
template <typename T> template<T... args>
const T base<T>::static_array_impl<args...>::data[sizeof...(args)] = { args... };

#endif  // end of #ifdef HAVE_CXX11

}  // end of namespace meta

#endif  // end of #ifndef meta__static_array_hxx
