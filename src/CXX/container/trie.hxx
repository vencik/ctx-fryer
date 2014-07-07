#ifndef container__trie_hxx
#define container__trie_hxx

/**
 *  \brief  Trie
 *
 *  Trie is a tree structure for reTRIEving data by a string key.
 *
 *  Each node represents a key prefix, each branch represents
 *  one or more characters (in broad sense) that may follow
 *  such prefix.
 *  Characters of branches from a single node may not share non-zero
 *  length prefix (i.e. branch must be identified by the 1st character).
 *  Every node may carry data in general.
 *
 *  IMPLEMENTATION NOTES
 *
 *  This implementation guarrantees trie path traversal in O(n)
 *  time, where n is the path length in characters.
 *  Character is a general term, here; the implementation is a template
 *  one, so virtually any type may be used as the "character" type.
 *
 *  Node branch access is done in constant time.
 *  That is facilitated by the fact that the character string is actually
 *  treated as string of half-bytes, reducing the character alphabet
 *  to 16 values.
 *  Each node carries an array of all the 16 possible branch entries.
 *  That's not that much of an overhead, while speeds up searching nicely.
 *
 *  The branches, are condensed so that the only nodes
 *  that just sprout one branch are those carrying payload.
 *
 *  REFERENCES
 *
 *  http://en.wikipedia.org/wiki/Trie
 *
 *  \author  Vaclav Krpec  <vencik@razdva.cz>
 *  \date    2014/05/17
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
#include "meta/bit_info.hxx"

#include <map>
#include <stdexcept>
#include <cassert>
#include <climits>


namespace container {

namespace impl {

/** 4 bits */
class uint4_t {
    private:

    unsigned char m_impl;  /**< Implementation */

    public:

    /** Default constructor */
    uint4_t(): m_impl(0) {}

    /**
     *  \brief  Constructor (with initialisation)
     *
     *  \c 0xf bit-mask is shifted by \c shift bits left
     *  and applied on the initialiser.
     *  The result is shifted back right and stored.
     *  By default, lower half of the initialiser is used
     *  (i.e. \c shift == \c 0).
     *
     *  \param  init   Initialiser
     *  \param  shift  Left bitwise shift applied on bit-mask
     */
    uint4_t(unsigned char init, int shift = 0):
        m_impl((init & (0xf << shift)) >> shift)
    {}

    /** Value getter */
    inline operator unsigned char () const { return m_impl; }

    /** Value setter */
    inline uint4_t & operator = (unsigned char byte) {
        m_impl = byte & 0xf;

        return *this;
    }

};  // end of class uint4_t

/** String of 4-bit values */
typedef std::basic_string<uint4_t> uint4_string;


/** Get key string size in 4 bit units */
template <typename C>
inline size_t size4b(const std::basic_string<C> & str) {
    return str.size() * sizeof(C) * 2;
}


/** Get 4 bits from string */
template <typename C>
inline uint4_t get4b(const std::basic_string<C> & str, size_t off) {
    size_t char_off = (off / 2) / sizeof(C);
#ifdef WORDS_BIGENDIAN
    size_t byte_off = (off / 2) % sizeof(C);
#else
    size_t byte_off = sizeof(C) - 1 - ((off / 2) % sizeof(C));
#endif  // end of #ifdef WORDS_BIGENDIAN
    int    shift    = (1 - (off % 2)) * 4;  // MS1/2B first

    assert(char_off < str.size());

    unsigned char * bytes = (unsigned char *)(&str[char_off]);

    return ((0xf << shift) & bytes[byte_off]) >> shift;
}


/**
 *  \brief  Traverse substring by 4 bits (generic algorithm)
 *
 *  \tparam  C  Character type
 *  \tparam  F  Functor
 *
 *  \param  str  String
 *  \param  off  Offset
 *  \param  len  Length
 *  \param  f    Functor instance
 */
template <typename C, class F>
void traverse(
    const std::basic_string<C> & str,
    size_t off, size_t len, F & f)
{
    assert(off <  str.size() * sizeof(C) * 2);
    assert(len <= str.size() * sizeof(C) * 2 - off);

    const size_t end = off + len;

    size_t char_off    = (off / 2) / sizeof(C);
#ifdef WORDS_BIGENDIAN
    size_t byte_off    = (off / 2) % sizeof(C);
#else
    size_t byte_off    = sizeof(C) - 1 - ((off / 2) % sizeof(C));
#endif  // end of #ifdef WORDS_BIGENDIAN
    unsigned char mask = 0xf << ((1 - (off % 2)) * 4);  // MS1/2B first

    for (; off < end && !f.done(); ++off) {
        const C & ch = str[char_off];
        const unsigned char * bytes = (unsigned char *)(&ch);

        unsigned char half_byte = mask & bytes[byte_off];

        mask = ~mask;  // mask simply alters

        int shift = 0x4 & mask;

        f(half_byte >> shift);

        // Crossing byte boundary
#ifdef WORDS_BIGENDIAN
        if (shift) {
#else
        if (!shift) {
#endif  // end of #ifdef WORDS_BIGENDIAN
            // Within character boundary
            // Note that for sizeof(C) == 1, the following condition
            // is identically false, so compiler should optimise
            // the if statement, turning it into the else branch, only
#ifdef WORDS_BIGENDIAN
            if (byte_off < sizeof(C) - 1)
                ++byte_offset;
#else
            if (byte_off)
                --byte_off;
#endif  // end of #ifdef WORDS_BIGENDIAN

            // Crossing character boundary
            else {
                ++char_off;
                byte_off = 0;
            }
        }
    }
}


/** 4 bit substring collector */
class substr4b_collector {
    private:

    uint4_string m_value;   /**< Collected string */
    const size_t m_length;  /**< String length    */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  len  Collected string length
     */
    substr4b_collector(size_t length):
        m_length(length)
    {
        m_value.reserve(m_length);
    }

    /**
     *  \brief  Append another 4 bits to string
     *
     *  \param  bits  Appended 4 bits
     */
    inline void operator () (uint4_t bits) { m_value.push_back(bits); }

    /** \c true if the operation has finished */
    inline bool done() const { return m_value.size() == m_length; }

    /** String getter */
    inline const uint4_string & get() const { return m_value; }

};  // end of class substr4b_collector

/** Create 4 bit sub-string from string */
template <typename C>
inline uint4_string substr4b(
    const std::basic_string<C> & str,
    size_t off = 0,
    size_t len = std::basic_string<C>::npos)
{
    size_t rest = str.size() * sizeof(C) * 2 - off;

    if (len > rest)
        len = rest;

    substr4b_collector substr(len);

    traverse<C, substr4b_collector>(str, off, len, substr);

    return substr.get();
}


/** Find longest pattern match */
class pattern4b_match {
    private:

    const uint4_string & m_str;  /**< Matched pattern */
    size_t               m_off;  /**< Offset          */
    size_t               m_end;  /**< End offset      */

    public:

    /**
     *  \brief  Constructor
     *
     *  \param  str  Matched pattern
     *  \param  off  Offset in pattern
     *  \param  len  Length of pattern
     */
    pattern4b_match(
        const uint4_string & str,
        size_t               off,
        size_t               len)
    :
        m_str(str),
        m_off(off),
        m_end(off + len)
    {
        assert(m_off <  m_str.size());
        assert(m_end <= m_str.size());
    }

    /**
     *  \brief  Check another 4 bits
     *
     *  \param  bits  4 bits to match
     */
    inline void operator () (uint4_t bits) {
        if (bits != m_str[m_off])
            m_end = m_off;
        else
            ++m_off;
    }

    /** \c true if the operation has finished */
    inline bool done() const { return m_off >= m_end; }

    /** Offset getter */
    inline size_t offset() const { return m_off; }

};  // end of class pattern4b_match

/**
 *  \brief  Find mismatch offset
 *
 *  \param  str      String
 *  \param  pat      Match pattern
 *  \param  str_off  Offset in string
 *  \param  pat_off  Offset in pattern
 *  \param  len      Max. amount of 4-bit match operations
 *
 *  \return Mismatch offset in pattern
 */
template <typename C>
inline size_t match4b(
    const std::basic_string<C> & str,
    const uint4_string         & pat,
    size_t                       str_off = 0,
    size_t                       pat_off = 0,
    size_t                       len     = std::basic_string<C>::npos)
{
    size_t str_rest = str.size() * sizeof(C) * 2 - str_off;
    size_t pat_rest = pat.size() - pat_off;

    if (len > str_rest)
        len = str_rest;
    if (len > pat_rest)
        len = pat_rest;

    if (0 == len) return pat_off;

    pattern4b_match match(pat, pat_off, len);

    traverse<C, pattern4b_match>(str, str_off, len, match);

    return match.offset();
}


/**
 *  \brief  Trie node
 *
 *  \tparam  C  Character type
 *  \tparam  T  Value type
 */
template <typename C, typename T>
class trie_node {
    public:

    typedef std::basic_string<C> key_t;  /**< Key string */

    private:

    /** Trie branch */
    struct trie_branch {
        uint4_string key;   /**< Incremental key */
        trie_node *  node;  /**< Trie node       */

        /** Default constructor */
        trie_branch(): node(NULL) {}

        /** Validity check */
        inline bool is_valid() const { return NULL != node; }

    };  // end of struct trie_branch

    bool        m_internal;      /**< Internal node flag                 */
    trie_node * m_parent;        /**< Parent node                        */
    uint4_t     m_at;            /**< Index of branch to the node        */
    trie_branch m_branches[16];  /**< Branches (indexed by 1st 1/2-byte) */
    uint16_t    m_branch_map;    /**< Valid branches bitmap              */
    size_t      m_offset;        /**< Key offset of the node (4-bit)     */
    T           m_payload;       /**< Payload                            */

    /**
     *  \brief  Internal node constructor
     *
     *  \param  parent  Parent node
     *  \param  at      Index of branch to te node
     *  \brief  offset  Key offset
     */
    trie_node(
        trie_node * parent,
        uint4_t     at,
        size_t      offset)
    :
        m_internal(true),
        m_parent(parent),
        m_at(at),
        m_branch_map(0),
        m_offset(offset)
    {}

    /**
     *  \brief  Value node constructor
     *
     *  \param  parent   Parent node
     *  \param  at       Index of branch to te node
     *  \brief  offset   Key offset
     *  \param  payload  Stored value
     */
    trie_node(
        trie_node * parent,
        uint4_t     at,
        size_t      offset,
        const T &   payload)
    :
        m_internal(false),
        m_parent(parent),
        m_at(at),
        m_branch_map(0),
        m_offset(offset),
        m_payload(payload)
    {}

#ifdef HAVE_CXX11
    /**
     *  \brief  Value node constructor (rvalue value)
     *
     *  \param  parent   Parent node
     *  \param  at       Index of branch to te node
     *  \brief  offset   Key offset
     *  \param  payload  Stored value (rvalue)
     */
    trie_node(
        trie_node * parent,
        uint4_t     at,
        size_t      offset,
        T &&        payload)
    :
        m_internal(false),
        m_parent(parent),
        m_at(at),
        m_branch_map(0),
        m_offset(offset),
        m_payload(std::move(payload))
    {}
#endif  // end of #ifdef HAVE__CXX11

    public:

    /** Check if node is internal (i.e. doesn't bare value) */
    inline bool is_internal() const { return m_internal; }

    /** Key offset getter */
    inline size_t key_offset() const { return m_offset; }

    /** Node payload getter */
    inline T & get_payload() throw(std::logic_error) {
        if (is_internal())
            throw std::logic_error(
                "container::trie: request for payload on internal node");

        return m_payload;
    }

    private:

    /**
     *  \brief  Make node internal
     *
     *  \param  val_node  Value node pointer
     */
    inline void set_internal() {
        m_internal = true;
        m_payload  = T();  // make sure that value is destroyed
    }

    /** Branch count getter */
    inline size_t branch_cnt() const {
        return meta::bit_info::set_cnt(m_branch_map);
    }

    /**
     *  \brief  Declare branch valid
     *
     *  \param  idx  Branch index
     */
    inline void branch_valid(size_t idx) {
        assert(idx < 16);

        m_branch_map |= 0x1 << idx;
    }

    /** 1st branch index */
    inline size_t branch_1st_idx() const {
        return meta::bit_info::ls1b_off(m_branch_map);
    }

    /** Last branch index */
    inline size_t branch_last_idx() const {
        return meta::bit_info::ms1b_off(m_branch_map);
    }

    /** 1st branch */
    inline trie_branch & branch_1st() const throw(std::logic_error) {
        size_t idx = branch_1st_idx();

        if (!(idx < 16))
            throw std::logic_error(
                "container::trie: no 1st branch");

        return m_branches[idx];
    }

    /** Last branch */
    inline trie_branch & branch_last() const throw(std::logic_error) {
        size_t idx = branch_last_idx();

        if (!(idx < 16))
            throw std::logic_error(
                "container::trie: no last branch");

        return m_branches[idx];
    }

    /** \c this branch */
    inline trie_branch & branch_mine() const throw(std::logic_error) {
        if (NULL == m_parent)
            throw std::logic_error(
                "container::trie: no branch to root");

        return m_parent->m_branches[m_at];
    }

    /**
     *  \brief  Get previous branch index
     *
     *  \param  map  Branch map
     *  \param  idx  Branch index
     *
     *  \return Closest valid lower branch index or 16 if there isn't one
     */
    inline static size_t prev_branch_idx(uint16_t map, size_t idx) {
        assert(idx < 16);

        // Consider previous branch bits
        uint16_t mask = 0xffff >> (16 - idx);

        return meta::bit_info::ms1b_off((uint16_t)(map & mask));
    }

    /** Get previous branch index (of \c this) */
    inline size_t prev_branch_idx(size_t idx) const {
        return prev_branch_idx(m_branch_map, idx);
    }

    /**
     *  \brief  Next branch index
     *
     *  \param  map  Branch map
     *  \param  idx  Branch index
     *
     *  \return Closest valid higher branch index or 16 if there isn't one
     */
    inline static size_t next_branch_idx(uint16_t map, size_t idx) {
        assert(idx < 16);

        // Consider next branch bits
        uint16_t mask = 0xffff << (idx + 1);

        return meta::bit_info::ls1b_off((uint16_t)(map & mask));
    }

    /** Get next branch index (of \c this) */
    inline size_t next_branch_idx(size_t idx) const {
        return next_branch_idx(m_branch_map, idx);
    }

    /** 1st child getter (or \c NULL if childless) */
    inline trie_node * child_1st() const {
        size_t at = branch_1st_idx();

        if (!(at < 16)) return NULL;

        const trie_branch & branch = m_branches[at];

        assert(branch.is_valid());

        return branch.node;
    }

    /** Last child getter (or \c NULL if childless) */
    inline trie_node * child_last() const {
        size_t at = branch_last_idx();

        if (!(at < 16)) return NULL;

        const trie_branch & branch = m_branches[at];

        assert(branch.is_valid());

        return branch.node;
    }

    /** Previous sibling getter (or \c NULL if 1st child) */
    trie_node * prev_sibling() const {
        if (NULL == m_parent) return NULL;  // root has no siblings

        size_t at = prev_branch_idx(m_parent->m_branch_map, m_at);

        if (at < 16) {
            assert(m_parent->m_branches[at].is_valid());

            return m_parent->m_branches[at].node;
        }

        return NULL;  // no such sibling
    }

    /** Next sibling getter (or \c NULL if last child) */
    trie_node * next_sibling() const {
        if (NULL == m_parent) return NULL;

        size_t at = next_branch_idx(m_parent->m_branch_map, m_at);

        if (at < 16) {
            assert(m_parent->m_branches[at].is_valid());

            return m_parent->m_branches[at].node;
        }

        return NULL;  // no such sibling
    }

    /**
     *  \brief  Get next node (depth-first search)
     *
     *  \param  node  Current node
     *  \param  stop  Backtracking stop (optional)
     *
     *  \return Next node in depth-first order
     */
    static trie_node * get_next_dfs(
        trie_node * node,
        trie_node * stop = NULL)
    {
        // Go deep if possible
        trie_node * next = node->child_1st();

        if (NULL != next) return next;

        while (node != stop) {
            if (NULL == node->m_parent)
                return NULL;  // back at root

            // Get next sibling
            next = node->next_sibling();

            if (NULL != next) return next;

            // Backtrack
            node = node->m_parent;
        }

        return NULL;  // at stop
    }

    /**
     *  \brief  Get previous node (depth-first search)
     *
     *  \param  node  Current node
     *
     *  \return Previous node in depth-first order (or \c NULL)
     */
    inline static trie_node * get_prev_dfs(
        trie_node * node)
    {
        // Previous sibling's bottom-right most offspring (if any)
        trie_node * prev = node->prev_sibling();

        if (NULL == prev) return node->m_parent;

        for (;;) {
            trie_node * child = prev->child_last();

            if (NULL == child) break;

            prev = child;
        }

        return prev;
    }

    /**
     *  \brief  Insert node into condensed branch (implementation)
     *
     *  IMPORTANT NOTE
     *  The function DOES NOT fix the node parent nor the value node
     *  pointer and its incoming branch index.
     *  That must have been done by the calling (implemented) function,
     *  already.
     *
     *  \param  br_idx  Branch index
     *  \param  at      Insertion position in key
     *  \param  node    Inserted node
     */
    void split_branch_impl(
        size_t      br_idx,
        size_t      at,
        trie_node * node)
    {
        assert(br_idx < 16);
        assert(at > 0);

        trie_branch & branch = m_branches[br_idx];

        assert(at < branch.key.size());
        assert(NULL != node);

        // Get "tail" branch from the inserted node
        // (i.e. continuation of the split)
        uint4_t tail_key_head = branch.key[at];

        trie_branch & tail = node->m_branches[tail_key_head];

        // Set the tail
        tail.key  = branch.key.substr(at);
        tail.node = branch.node;

        node->branch_valid(tail_key_head);

        // Set the branch
        branch.key.erase(at);
        branch.node = node;

        // Fix tail target node attributes
        tail.node->m_parent = node;
        tail.node->m_at     = tail_key_head;
    }

    /**
     *  \brief  Insert value node into condensed branch
     *
     *  \param  branch  Branch index
     *  \param  at      Insertion position
     *  \param  offset  Key offset
     *  \param  val     Value
     *
     *  \return Inserted node
     */
    inline trie_node * split_branch(
        size_t    branch,
        size_t    at,
        size_t    offset,
        const T & val)
    {
        trie_node * node = new trie_node(this, branch, offset, val);

        split_branch_impl(branch, at, node);

        return node;
    }

    /**
     *  \brief  Insert internal node into condensed branch
     *
     *  \param  branch  Branch index
     *  \param  at      Insertion position
     *  \param  offset  Key offset
     *
     *  \return Inserted node
     */
    inline trie_node * split_branch(
        size_t branch,
        size_t at,
        size_t offset)
    {
        trie_node * node = new trie_node(this, branch, offset);

        split_branch_impl(branch, at, node);

        return node;
    }

    /**
     *  \brief  Insert by key (iterative algorithm)
     *
     *  \param  node     Current trie node
     *  \param  key      Key
     *  \param  val      Value for new node
     *  \param  success  Value inserted flag (\c true iff the value was added)
     *  \param  offset   Key offset
     *
     *  \return Inserted or existing node
     */
    static trie_node * insert_impl(
        trie_node *   node,
        const key_t & key,
        const T &     val,
        bool &        success,
        size_t        offset)
    {
        size_t key_offset = offset;
        size_t key_size   = size4b(key);

        success = true;  // optimistic assumption

        for (;;) {  // main loop
            assert(NULL != node);

            // Node found
            if (key_offset == key_size) {
                // Turn internal node into value node
                if (node->is_internal())
                    node->set_payload(val);

                // Duplicity!
                else success = false;

                return node;  // existing node
            }

            assert(key_offset < key_size);

            uint4_t       key_head = get4b(key, key_offset);
            trie_branch & branch   = node->m_branches[key_head];

            // A branch beginning with the character already exists
            if (branch.is_valid()) {
                assert(branch.key[0] == key_head);

                // Find node position
                size_t pos = match4b(key, branch.key, key_offset + 1, 1);

                // Due position found
                if (!(pos < key_size - key_offset)) {
                    key_offset += pos;

                    // Node exists on position
                    if (!(pos < branch.key.size())) {
                        node = branch.node;

                        continue;  // leave it for the main loop
                    }

                    // Insert node inside condensed path
                    return node->split_branch(
                               key_head, pos, key_offset, val);
                }

                // Descend deeper
                if (!(pos < branch.key.size()))
                    node = branch.node;

                // New internal node position found
                else
                    node = node->split_branch(
                        key_head, pos, key_offset + pos);

                // Passed another node on path
                key_offset += pos;
            }

            // Insert new node directly
            else {
                branch.key  = substr4b(key, key_offset);
                branch.node = new trie_node(node, key_head, key.size(), val);

                node->branch_valid(key_head);

                return branch.node;
            }
        }
    }

    /**
     *  \brief  Find node by key
     *
     *  The function provides either node with the key or upper
     *  bound hint that may be used for speedy insertion.
     *
     *  \param  key      Key
     *  \param  success  Success flag (\c true iff node found)
     *  \param  offset   Key offset
     *
     *  \return Node with the key or upper bound
     */
    static trie_node * find_impl(
        trie_node *   node,
        const key_t & key,
        bool &        success,
        size_t        offset)
    {
        size_t key_offset = offset;
        size_t key_size   = size4b(key);

        success = false;  // pessimistic assumption

        while (key_offset < key_size) {
            assert(NULL != node);

            uint4_t       key_head = get4b(key, key_offset);
            trie_branch & branch   = node->m_branches[key_head];

            // Upper bound found (path ends in node)
            if (!branch.is_valid())
                return node;

            assert(branch.key[0] == key_head);

            // Traverse condensed part of the path
            size_t pos = match4b(key, branch.key, key_offset + 1, 1);

            // Upper bound found (path ends in condensed branch)
            if (pos < branch.key.size())
                return node;

            node = branch.node;  // go forth
            key_offset += pos;
        }

        // Path traversed, node found
        success = !node->is_internal();
        return node;
    }

    /**
     *  \brief  Remove branch
     *
     *  \param  idx  Branch index
     */
    inline void remove_branch(size_t idx) {
        trie_branch & branch = m_branches[idx];

        delete branch.node;

        branch.key.clear();
        branch.node = NULL;
    }

    /**
     *  \brief  Remove node from tree
     *
     *  \param  node  Removed node
     */
    static void remove_impl(trie_node * node) {
        for (;;) {
            assert(NULL != node);

            trie_node * parent = node->m_parent;

            // Trie root is never removed; it always serves at least
            // as structural key base
            if (NULL == parent) {
                node->set_internal();

                return;
            }

            switch (node->branch_cnt()) {
                // Leaf (childless)
                case 0: {
                    // Just cut the branch to the leaf
                    parent->remove_branch(node->m_at);

                    // Internal parent with less than 2 children
                    // shall be removed
                    if (!parent->is_internal() || 1 < parent->m_branch_cnt)
                        return;

                    node = parent;
                    break;
                }

                // Knot on a string (just 1 branch)
                case 1: {
                    // Condense branch
                    trie_branch & node_branch  = node->branch_mine();
                    trie_branch & child_branch = node->branch_1st();

                    node_branch.key.append(child_branch.key);
                    node_branch.node = child_branch.node;

                    node_branch.node->m_at = node->m_at;

                    child_branch.node = NULL;
                    node->m_parent    = NULL;

                    delete node;

                    return;
                }

                // Fork point (at least 2 branches)
                default:
                    // Fork point is turned into internal node
                    node->set_internal();

                    return;
            }
        }
    }

    public:

    /** Root constructor (the only public one) */
    trie_node():
        m_internal(true),  // root begins as internal
        m_parent(NULL),    // root indeed
        m_at(0),           // unimportant for root
        m_branch_map(0),   // no children
        m_offset(0)        // root is at the begin
    {}

    /** Node offset getter */
    inline size_t get_offset() const { return m_offset; }

    /**
     *  \brief  Node payload getter
     *
     *  IMPORTANT NOTE:
     *  The function provides (read-only) access to the node payload
     *  WITHOUT checking whether it is or is not an internal node, only.
     */
    inline const T & get_payload(const T & payload) const {
        return m_payload;
    }

    /** Node payload setter */
    inline void set_payload(const T & payload) {
        m_internal = false;
        m_payload  = payload;
    }

#ifdef HAVE_CXX11
    /** Node payload setter */
    inline void set_payload(T && payload) {
        m_internal = false;
        m_payload  = payload;
    }
#endif  // end of #ifdef HAVE__CXX11

    /**
     *  \brief  Insert by key
     *
     *  \param  key      Key
     *  \param  val      Value for new node
     *  \param  success  Value inserted flag (\c true iff value was added)
     *  \param  offset   Key offset
     *
     *  \return Value node with \c value (either a new one or existing one
     *          in case of duplicity)
     */
    inline trie_node * insert(
        const key_t & key,
        const T &     val,
        bool &        success,
        size_t        offset)
    {
        return insert_impl(this, key, val, success, offset);
    }

    /**
     *  \brief  Insert by key
     *
     *  \param  key      Key
     *  \param  val      Value for new node
     *  \param  offset   Key offset
     *
     *  \return Inserted node or \c NULL in case of duplicity
     */
    inline trie_node * insert(
        const key_t & key,
        const T &     val,
        size_t        offset)
    {
        bool success;
        trie_node * node = insert_impl(this, key, val, success, offset);

        return success ? node : NULL;
    }

    /**
     *  \brief  Find node by key (in sub-tree)
     *
     *  The function provides either node with the key or upper
     *  bound hint that may be used for speedy insertion.
     *
     *  \param  key      Key
     *  \param  success  Success flag
     *  \param  offset   Key offset
     *
     *  \return Node with the key or upper bound
     */
    inline trie_node * find(
        const key_t & key,
        bool &        success,
        size_t        offset)
    {
        return find_impl(this, key, success, offset);
    }

    /**
     *  \brief  Find node by key (in sub-tree)
     *
     *  \param  key     Key
     *  \param  offset  Key offset
     *
     *  \return Node with the key or \c NULL
     */
    inline trie_node * find(const key_t & key, size_t offset) {
        bool success;

        trie_node * node = find_impl(this, key, success, offset);

        return success ? node : NULL;
    }

    /**
     *  \brief  Remove node
     */
    inline void remove() { remove_impl(this); }

    /**
     *  \brief  Remove node by key (from sub-tree)
     *
     *  \param  key  Key
     */
    inline void remove(const key_t & key) {
        bool success;

        trie_node * node = find_impl(this, key, success);

        if (!success) return;

        assert(NULL != node);

        node->remove();
    }

    /** Get next node (depth-first search) */
    inline trie_node * get_next_dfs() const { return get_next_dfs(this); }

    /** Get previous node (depth-first search reversed) */
    inline trie_node * get_prev_dfs() const { return get_prev_dfs(this); }

    /**
     *  \brief  Get next value node (for forward iteration)
     *
     *  The function provides _only_ value nodes (i.e. skips
     *  internal nodes).
     *  It does so in the depth-first order; therefore, it's
     *  ideal for (the most natural) lexicographic iteration
     *  over the keys.
     *
     *  \return Next value node or \c NULL if no such node exists
     */
    inline trie_node * get_next_val_dfs() {
        trie_node * node = this;

        do node = get_next_dfs(node);
        while (NULL != node && node->is_internal());

        return node;
    }

    /**
     *  \brief  Get prev value node (for backward iteration)
     *
     *  Like the \ref get_next_val function.
     *
     *  \return Previous value node or \c NULL if no such node exists
     */
    inline trie_node * get_prev_val_dfs() {
        trie_node * node = this;

        do node = get_prev_dfs(node);
        while (NULL != node && node->is_internal());

        return node;
    }

    /** Destructor (deep destruction) */
    ~trie_node() {
        // Destroy children
        size_t idx = branch_1st_idx();

        for (; 16 > idx; idx = next_branch_idx(idx)) {
            trie_node * & child = m_branches[idx].node;

            assert(NULL != child);

            delete child;
            child = NULL;
        }
    }

    private:

    /** Copying is forbidden */
    trie_node(const trie_node & orig) {}

    /** Assignment is forbidden */
    void operator = (const trie_node & rval) {}

};  // end of template class trie_node

}  // end of namespace impl


/**
 *  \brief  Trie
 *
 *  \tparam  C  Key character type
 *  \tparam  T  Value type (must have default constructor)
 */
template <typename C, typename T>
class trie {
    private:

    typedef impl::trie_node<C, T>  node_t;  /**< Node type */
    typedef typename node_t::key_t key_t;   /**< Key  type */

    node_t m_root;  /**< Trie root */

    public:

    /** Constructor (empty trie) */
    trie() {}

    // Forward declarations
    class iterator;

    /**
     *  \brief  Structural iterator
     *
     *  The implementation contains "internal nodes"; nodes that don't
     *  hold values and merely implement key string branches.
     *  This yields 2 kinds of iterators; an iterator that traverses
     *  tree paths regardles on the node kind (this one) and "value"
     *  (or the normal) iterator (which skips internal nodes).
     *
     *  Structural iterator is still public, since it may be needed
     *  for performance purposes (as the find-or-insert position hint).
     *  Structural iterator can't be dereferenced (it doesn't make
     *  much sense).
     *
     *  Structural iterator may be converted to value iterator.
     *  The other way round is, of course, trivial.
     */
    class struct_iterator {
        friend class trie;
        friend class iterator;

        private:

        node_t * m_node;  /**< Node */

        /** Constructor */
        struct_iterator(node_t * node): m_node(node) {}

        public:

        /** Increment */
        inline struct_iterator & operator ++ () throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::struct_iterator: ++ at end");

            m_node = m_node->get_next_dfs();

            return *this;
        }

        /** Decrement */
        inline struct_iterator & operator -- () throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::struct_iterator: -- at end");

            m_node = m_node->get_prev_dfs();

            return *this;
        }

        /** Comparison */
        inline bool operator == (const struct_iterator & rval) const {
            return m_node == rval.m_node;
        }

        /** Negative comparison */
        inline bool operator != (const struct_iterator & rval) const {
            return !(*this == rval);
        }

        /** Conversion to normal (value) iterator */
        inline operator iterator () const { return iterator(m_node); }

    };  // end of class struct_iterator

    /** Iterator */
    class iterator {
        friend class trie;
        friend class struct_iterator;

        private:

        node_t * m_node;  /**< Node */

        /**
         *  \brief  Constructor
         *
         *  The construction is based on the following rules:
         *  1/ on node with value => iterator on the same node
         *  2/ iterator on closest previous (DFS) node that has value (if any)
         *  3/ iteratir on closest next (DFS) node that has value (if any)
         *  4/ end iterator
         */
        iterator(node_t * node): m_node(node) {
            if (NULL == m_node) return;  // end

            if (m_node->is_internal()) {
                node_t * node = m_node->get_prev_val_dfs();

                if (NULL == node)
                    node = m_node->get_next_val_dfs();

                m_node = node;
            }
        }

        public:

        /** Increment */
        inline iterator & operator ++ () throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::iterator: ++ at end");

            m_node = m_node->get_next_val_dfs();

            return *this;
        }

        /** Decrement */
        inline iterator & operator -- () throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::iterator: -- at end");

            m_node = m_node->get_prev_val_dfs();

            return *this;
        }

        /** Dereference */
        T & operator * () const throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::iterator: invalid");

            assert(!m_node->is_internal());

            return m_node->get_payload();
        }

        /** Dereference */
        T * operator -> () const throw(std::range_error) {
            if (NULL == m_node)
                throw std::range_error(
                    "container::trie::iterator: invalid ptr");

            assert(!m_node->is_internal());

            return &m_node->get_payload();
        }

        /** Comparison */
        inline bool operator == (const iterator & rval) const {
            return m_node == rval.m_node;
        }

        /** Negative comparison */
        inline bool operator != (const iterator & rval) const {
            return !(*this == rval);
        }

        /** Conversion to structural iterator */
        inline operator struct_iterator () const {
            return struct_iterator(m_node);
        }

    };  // end of class iterator

    /** Begin iterator */
    iterator begin() { return iterator(&m_root); }

    /** End iterator */
    iterator end() { return iterator(NULL); }

    private:

    /**
     *  \brief  Insert another key/value pair (implementation)
     *
     *  In case the key already exists, it is NOT inserted;
     *  the duplicity is indicated by the return value.
     *
     *  \param  node    Insertion starting point
     *  \param  key     Key
     *  \param  val     Value
     *  \param  offset  Key offset
     *
     *  \return Iterator to entry + indication of insertion success
     */
    inline static std::pair<iterator, bool> insert_impl(
        node_t *      node,
        const key_t & key,
        const T &     val,
        size_t        offset)
    {
        bool found;
        node = node->insert(key, val, found, offset);

        return std::pair<iterator, bool>(node, found);
    }

    public:

    /**
     *  \brief  Find lower bound by \c key
     *
     *  The function provodes iterator to either node with the value
     *  or node representing lower bound hint for speedy insertion.
     *
     *  \param  key  Key
     *
     *  \return Structural iterator to node with the value or lower bound hint
     */
    inline std::pair<struct_iterator, bool>
    lower_bound(const key_t & key) {
        bool found;
        node_t * node = m_root.find(key, found, 0);

        return std::pair<struct_iterator, bool>(node, found);
    }

    /** Find value by \c key */
    inline iterator find(const key_t & key) {
        return iterator(m_root.find(key, 0));
    }

    /**
     *  \brief  Insert another key/value pair
     *
     *  In case the key already exists, it is NOT overwritten,
     *  the previous entry + indication of duplicity is reported.
     *
     *  \param  key  Key
     *  \param  val  Value
     *
     *  \return Iterator to entry + indication of insertion success
     */
    inline std::pair<iterator, bool>
    insert(const key_t & key, const T & val) {
        return insert_impl(&m_root, key, val, 0);
    }

    /**
     *  \brief  Insert key/value pair (with position hint)
     *
     *  In case the key already exists, the value is overwritten.
     *
     *  IMPORTANT NOTE:
     *  The method is provided in order to allow efficient
     *  search-set operation (where setting may not happen or be otherwise
     *  dependent on the search operation result).
     *  Example:
     *
     *  \code
     *
     *  std::pair<trie::struct_iterator, bool> search = dict.find(key);
     *
     *  if (search.second)
     *      std::cout << "Already there" << std::endl;
     *  else
     *      dict.insert(key, val, search.first);  // fast insertion
     *
     *  \endcode
     *
     *  In the example above, an action that depends on the search result
     *  is injected between (eventual) setting of the value.
     *  The \ref insert operation then may benefit from the fact that
     *  the value position was already (partially) found; insertion
     *  commences from a node (represented by the structural iterator)
     *  that lays on a path representing valid \c key prefix.
     *  Actually, since that path represents the longest possible \c key
     *  prefix, the addition by \ref insert is then performed in
     *  constant time (thanks to branch condensation).
     *  On the contrary, \ref insert without the hint would have to retrace
     *  the whole path again, doubling the operation time complexity.
     *
     *  IMPORTANT NOTE:
     *  The position HINT MUST REPRESENT correct KEY PREFIX.
     *  IT IS NOT CHECKED that the prefix matches.
     *  Therefore, IF the position iterator does NOT represent
     *  trie node with key that indeed is valid prefix of the provided
     *  \c key, the VALUE SHALL BE INSERTED UNDER A DIFFERENT KEY
     *  than provided!
     *  You've been warned.
     *
     *  In case of duplicity, the value is NOT overwritten.
     *
     *  \param  key  Key
     *  \param  val  Value
     *  \param  pos  Position hint (e.g. from \ref find)
     *
     *  \return Iterator to entry + indication of success
     */
    inline std::pair<iterator, bool> insert(
        const key_t &   key,
        const T &       val,
        struct_iterator pos)
    {
        size_t offset = pos->m_node->get_offset();

        return insert_impl(pos->m_node, key, val, offset);
    }

    /**
     *  \brief  Insert at position with incremental key
     *
     *  The function inserts the value to the position as if
     *  the key was prepended by the prefix prepresented by the
     *  position node.
     *  I.e. if the \c pos iterator node represents key \c K,
     *  then the value shall be stored under key \c K.key.
     *
     *  In case of duplicity, the value is NOT overwritten.
     *
     *  \param  key  Key
     *  \param  val  Value
     *  \param  pos  Position hint (e.g. from \ref find)
     *
     *  \return Iterator to entry + indication of success
     */
    inline std::pair<iterator, bool> insert_relative(
        const key_t &   key,
        const T &       val,
        struct_iterator pos)
    {
        return insert_impl(pos->m_node, key, val, 0);
    }

    /**
     *  \brief  Access by key
     *
     *  \param  key  Key
     *
     *  \return Value (by ref.) at key; the entry is created
     *          unless it already exists
     */
    inline T & operator [] (const key_t & key) {
        bool found;
        struct_iterator pos = find(key, found);

        iterator entry = found ? pos : insert(key, T(), pos);

        return *entry;
    }

    /**
     *  \brief  Remove value by key
     *
     *  \param  key  Key
     */
    inline void remove(const key_t & key) {
        node_t * node = m_root.find(key);

        if (NULL != node)
            node->remove();
    }

    /**
     *  \brief  Remove value
     *
     *  Throws an exception if iterator is invalid.
     *
     *  \param  pos  Iterator
     */
    inline void remove(struct_iterator & pos) throw(std::range_error) {
        if (NULL == pos.m_node)
            throw std::range_error(
                "container::trie: invalid remove iterator");

        pos.m_node->remove();
        pos.m_node = NULL;
    }

    private:

    /** Copying is forbidden */
    trie(const trie & orig) {}

    /** Assignment is forbidden */
    void operator = (const trie & rval) {}

};  // end of template class trie

}  // end of namespace container

#endif  // end of #ifndef container__trie_hxx
