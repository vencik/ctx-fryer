#ifndef meta__postpone_hxx
#define meta__postpone_hxx

/**
 *  \brief  Postponing certain action till end of scope
 *
 *  \c meta::postpone object performs certain action upon destruction.
 *  If instantiated as a local variable, it may be easily used for
 *  scheduling execution of a functor (or, ideally, a lambda function)
 *  at the end of scope.
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2015/03/12
 *
 *  Legal notices
 *
 *  Copyright 2015 Vaclav Krpec
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


namespace meta {


/** \cond */
// Instance name constructor for postpone4scope
#define _postpone_name_impl(base, line_no) base ## line_no
#define _postpone_name(base, line_no) _postpone_name_impl(base, line_no)
/** \endcond */

/**
 *  \brief  Postpone operation till end of scope
 *
 *  The convenience macro shall instantiate a \c meta::postpone object,
 *  passing it a functor (or, ideally, lambda function) that will be executed
 *  upon the object destruction.
 *
 *  \param  fn  Functor
 */
#define postpone4scope(fn) \
    meta::postpone<decltype(fn)> _postpone_name(__postpone_, __LINE__)(fn)


/** Schedule execution at the point of object destruction */
template <typename Functor>
class postpone {
    private:

    Functor m_f;  /**< Scheduled action */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  f  Functor
     */
    postpone(Functor f): m_f(f) {}

    /** Destructor */
    ~postpone() { m_f(); }

};  // end of template class postpone

}  // end of namespace meta

#endif  // end of #ifndef meta__postpone_hxx
