#ifndef test__parser_tables_h
#define test__parser_tables_h

/**
 *  \brief   LR(1) parser tables declarations
 *
 *  The file is part of CTX Fryer C target language libraries development.
 *
 *  \date  2012/08/24
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

#include "syntax_analyser.h"


/**
 *  \brief  LR(1) action table
 *
 *  The parser tables are generated.
 *  Don't change the code; change the definition, if required.
 */
extern const lr1_action_tab_t test_action_tab;


/**
 *  \brief  LR(1) goto table
 *
 *  The parser tables are generated.
 *  Don't change the code; change the definition, if required.
 */
extern const lr1_goto_tab_t test_goto_tab;


/**
 *  \brief  LR(1) rules info table
 *
 *  The parser tables are generated.
 *  Don't change the code; change the definition, if required.
 */
extern const lr1_rule_tab_t test_rule_tab;


/**
 *  \brief  Attribute definitions table
 *
 *  The parser tables are generated.
 *  Don't change the code; change the definition, if required.
 */
extern const attribute_tab_t test_attribute_tab;

#endif  /* end of #ifndef test__parser_tables_h */
