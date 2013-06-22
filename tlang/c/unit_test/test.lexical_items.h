#ifndef CTXFryer__lexical_items_h
#define CTXFryer__lexical_items_h

/**
 *  \brief   FSA : Accepted lexical items
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/06/15
 *
 *  \author  Vaclav Krpec <vencik@razdva.cz>
 *
 *  Legal notices
 *
 *  Copyright 2012 Vaclav Krpec
 *
 *  CTX Fryer C library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#define LEXI_number 1  /**< Lexical item for "number" terminal symbol */
#define LEXI_add    2  /**< Lexical item for "add"    terminal symbol */
#define LEXI_mul    3  /**< Lexical item for "mul"    terminal symbol */
#define LEXI_rpar   4  /**< Lexical item for "rpar"   terminal symbol */
#define LEXI_lpar   5  /**< Lexical item for "lpar"   terminal symbol */

#define LEXICNT 6  /**< Number of lexical items (including EoF) */

#endif /* end of #ifndef CTXFryer__lexical_items_h */
