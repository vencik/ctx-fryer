#ifndef container__heap_hxx
#define container__heap_hxx

/**
 *  \brief  Heap
 *
 *  Heap is a container that allows for efficient insert/delete
 *  operations of items from a well-ordered set, keeping track
 *  of the minimum.
 *
 *  The most used heap flavours are:
 *  * Binary heap
 *  * Binomial heap
 *  * Fibonacci heap
 *
 *  Binary heap is only used for continuous array sorting in practice
 *  (since it can be easily implemented over such an array).
 *  The resulting \c heapsort sorting algorithm has _worst case_ time complexity
 *  of O(n log n).
 *  Unlike commonly used \c quicksort, \c heapsort therefore guarrantees
 *  to sort _any_ array of length \c n in O(n log n) time (wihle \c quicksort
 *  may actually fall to O(n^2) complexity in certain cases).
 *  However, in average case, \c quicksort is faster than \c heapsort; therefore,
 *  \c heapsort is only used in situations where each sorted instance requires
 *  optimal speed guarrantee.
 *
 *  Binomial heap provides dynamic size and O(log n) guarranteed operations
 *  time complexity.
 *  If, however, insert operations notably prevail over the other operations,
 *  Fibonacci heap may be better choice as it allows for better ammortised
 *  time complexity in such situations (in exchange for potentially high
 *  complexity of certain operations as the heap may need consolidation
 *  beforehand).
 *  For instance, binomial heap is more suitable for implementation of
 *  priority queue, since typically, you want to perform the same amount of
 *  push and pull operations over it as well as you expect the operations
 *  to be done in guarranteed time.
 *  However, Fibonacci heap serves better e.g. for implementation of accumulators
 *  (multi-dimensional counters, see Hough transform for an example of use)
 *  where you typically wish to fill the accumulator and then search
 *  for peaks, so inserts greatly outnumber minimum searches and removals.
 *
 *  References:
 *  * http://en.wikipedia.org/wiki/Heap_(data_structure)
 *  * http://en.wikipedia.org/wiki/Priority_queue
 *  * http://en.wikipedia.org/wiki/Hough_transform
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

#include <cstdlib>
#include <cassert>
#include <stdexcept>


namespace container {

namespace impl {

/** Default implementation of decrease key (using -= operator) */
template <typename T>
class dec_key_default {
    public:

    /** Implementation */
    inline void operator () (T & key, const T & dec) const {
        key -= dec;
    }

};  // end of template class dec_key_default


/** Default implementation of Less comparator (using < operator) */
template <typename T>
class less_default {
    public:

    /** Implementation */
    inline bool operator () (const T & left, const T & right) const {
        return left < right;
    }

};  // end of template class less_default

}  // end of namespace impl


/**
 *  \brief  Binomial heap
 *
 *  Binomial heap is a forrest of binomial trees of mutually different orders
 *  that each fulfil the so-called minimum heap property:
 *  the key of a node is greater or equal to the key of its parent.
 *
 *  For more information, see http://en.wikipedia.org/wiki/Binomial_heap
 *  Note however that this implementation uses a slightly different definition
 *  of binomial tree for practical reasons (see \ref impl::Btree).
 */
template <typename T, class DecKey = impl::dec_key_default<T>, class Less = impl::less_default<T> >
class binomial_heap;


/**
 *  \brief  Fibonacci heap
 *
 *  TODO
 */
//template <typename T>
//class fibonacci_heap;


namespace impl {

/**
 *  \brief  Binomial tree
 *
 *  Definition (inductive):
 *  * B_k be binomial tree of order k
 *  * B_0 is a single node
 *  * B_k is a tree which root's children are {B_(0), B_(1), ... , B_(k-1)}
 *    (left to right) for k > 0.
 *
 *  Note that B_k tree size (number of nodes) is 2^k
 *  Proof (by math. induction):
 *  *Induction base*: |B_0| = 1 = 2^0  (obviously)
 *  *Induction premise*: |B_k| = 2^k
 *  *Induction step*:
 *    |B_(k+1)| = 1 + sum_{i=0}^{k} |B_i|  (from definition)
 *    |B_(k+1)| = 1 + sum_{i=0}^{k} 2^i    (from induct. premise)
 *    |B_(k+1)| = 1 + (sum_{i=0}^{k-1} |B_i| + 2^k)
 *    Now note again that
 *      |B_k| = 2^k = 1 + sum_{i=0}^{k-1} |B_i|  (from above)
 *    |B_(k+1)| = (1 + sum_{i=0}^{k-1} |B_i|) + |B_k|  (assoc.)
 *    |B_(k+1)| = |B_k| + |B_k| = 2 * 2^k = 2^(k+1)  QED
 *
 *  The trees are ment to always be dynamic.
 *  Construction is only allowed to friends.
 *
 *  Note that typical definition uses reversed order of the children.
 *  However, the above order is more practical for binomial heap implementation.
 *
 *  Also note that for practical reasons, the 1st child's \c m_prev (i.e. pointer
 *  to the previous sibling) is set to point at the last child to enable easy
 *  obtaining of it.
 *  The last child's \c m_next pointer is however \c NULL to enable for easy
 *  detection of the sibling list end.
 *  This makes the sibling list one-way cyclic bi-directional linked list.
 *
 *  The siblings list is also used for creation of tree lists on root level.
 *
 *  Last but not least, note that each node of a tree has pointer \c m_super
 *  that points to what shall hereafter be called "superior node".
 *  Superior node of any 1st child is its parent.
 *  Superior node of any other child is the 1st child.
 *  This mechanism allows for easy movement over the tree structure upwards
 *  without having to traverse the children list (finding parent is matter
 *  of at most 2 dereferences), while allowing for simple node position swapping
 *  without having to change more than constant number of pointers.
 *  Roots of trees have the \c m_super pointer set to \c NULL, regardless of
 *  being or not in any tree lists.
 *  In condensed language, the sentence above reads simply:
 *  Trees in a forest are peers.
 */
template <typename T>
class Btree {
    friend class binomial_heap<T>;

    private:

    size_t  m_order;  /**< Tree order       */
    Btree * m_super;  /**< Superior node    */
    Btree * m_child;  /**< 1st child        */
    Btree * m_prev;   /**< Previous sibling */
    Btree * m_next;   /**< Next sibling     */

    public:

    T value;  /**< Payload */

    protected:

    /**
     *  \brief  Remove the tree from a tree list
     *
     *  The operation is done in constant time.
     *
     *  USE WITH CAUTION!
     *
     *  It's responsibility of the caller to make sure that this tree
     *  indeed belongs to the provided list.
     *  The function only check this if the tree is the list head.
     *
     *  \param  list  Tree list
     *
     *  \return The resulting list head item
     */
    Btree * unlist_from(Btree * list) {
        assert(NULL != list);
        assert(NULL != m_prev);  // this must always be valid pointer

        // The tree is on the list head
        if (NULL == m_prev->m_next) {
            assert(this == list);

            list = m_next;
        }
        else
            m_prev->m_next = m_next;

        // The tree isn't last in the list
        if (NULL != m_next)
            m_next->m_prev = m_prev;

        // The tree was last in still non-empty list
        else if (NULL != list)
            list->m_prev = m_prev;

        // Make the tree a stand-alone one
        m_prev = this;
        m_next = NULL;

        return list;
    }

    /**
     *  \brief  Make this left neighbour of the supplied tree
     *
     *  The operation is done in constant time.
     *
     *  USE WITH CAUTION!
     *
     *  The function DOESN'T check if this object is a stand-alone
     *  tree.
     *  If it isn't then the list it belongs to will still contain it
     *  (yielding a non-empty intersection of the 2 lists).
     *  Although that isn't generally wrong, it's definitely something
     *  to point out.
     *
     *  \param  btree  The right sibling-to-be (of this)
     */
    void enlist_left_of(Btree * btree) {
        assert(NULL != btree);
        assert(NULL != btree->m_prev);  // this must always be valid pointer

        // The tree isn't a list head
        if (NULL != btree->m_prev->m_next)
            btree->m_prev->m_next = this;

        m_prev = btree->m_prev;
        btree->m_prev = this;
        m_next = btree;
    }

    /**
     *  \brief  Join tree lists
     *
     *  USE WITH CAUTION!
     *
     *  The function treats this as a tree list (checked), and joins
     *  the provided one to its tail.
     *  The function DOESN'T check orders of the trees in the lists.
     *  The operation is done in constant time.
     *
     *  \param  list  Tree list head
     */
    void join_list(Btree * list) {
        if (NULL == list) return;

        Btree * last = m_prev;

        assert(NULL != last);
        assert(NULL == last->m_next);  // check that this is tree list head

        assert(NULL != list);
        assert(NULL != list->m_prev);
        assert(NULL == list->m_prev->m_next);  // check list is valid, too

        // Fix last item pointer
        m_prev = list->m_prev;

        // Join lists
        last->m_next = list;
        list->m_prev = last;
    }

    /**
     *  \brief  Swap 2 node pointers
     *
     *  \param  btree1  1st pointer
     *  \param  btree2  2nd pointer
     */
    static inline void swap_ptrs(Btree * & btree1, Btree * & btree2) {
        Btree * tmp = btree1;
        btree1 = btree2;
        btree2 = tmp;
    }

    /**
     *  \brief  Swap nodes of tree(s)
     *
     *  The function interchanges 2 different nodes.
     *  The operation is done in constant time.
     *
     *  IMPORTANT NOTE:
     *  The function assumes that the nodes are different.
     *
     *  \param  btree1  Tree node
     *  \param  btree2  Tree node
     */
    static void swap_nodes(Btree * btree1, Btree * btree2) {
        assert(NULL != btree1);
        assert(NULL != btree2);

        assert(btree1 != btree2);  // swapping with itself doesn't make sense

        // Swap orders
        size_t tmp_order = btree1->m_order;
        btree1->m_order  = btree2->m_order;
        btree2->m_order  = tmp_order;

        // Swap superior nodes
        swap_ptrs(btree1->m_super, btree2->m_super);

        // Swap chilren
        if (NULL != btree1->m_child)
            btree1->m_child->m_super = btree2;

        if (NULL != btree2->m_child)
            btree2->m_child->m_super = btree1;

        swap_ptrs(btree1->m_child, btree2->m_child);

        // Swap previous siblings
        if (NULL != btree1->m_prev && NULL != btree1->m_prev->m_next)
            btree1->m_prev->m_next = btree2;

        if (NULL != btree2->m_prev && NULL != btree2->m_prev->m_next)
            btree2->m_prev->m_next = btree1;

        swap_ptrs(btree1->m_prev, btree2->m_prev);

        // Swap next siblings
        if (NULL != btree1->m_next)
            btree1->m_next->m_prev = btree2;

        if (NULL != btree2->m_next)
            btree2->m_next->m_prev = btree1;

        swap_ptrs(btree1->m_next, btree2->m_next);
    }

    /**
     *  \brief  B_0 constructor
     *
     *  \param  val  Payload
     */
    Btree(const T & val):
        m_order(0),
        m_super(NULL),
        m_child(NULL),
        m_prev(this),
        m_next(NULL),
        value(val) {}

    /**
     *  \brief  B_k constructor
     *
     *  Note that the root's children must already
     *  be chained (see \ref set_sibling).
     *  The constructor doesn't check the binomial tree
     *  property; it's responsability of the caller to keep it.
     *
     *  \param  val    Root payload
     *  \param  child  1st child
     */
    Btree(const T & val, Btree * child):
        m_order(child->m_order + 1),
        m_super(NULL),
        m_child(child),
        m_prev(this),
        m_next(NULL),
        value(val)
    {
        child->m_super = this;
    }

    /**
     *  \brief  B_k copy constructor
     *
     *  The constructor creates copy of the whole tree.
     *  If the tree is part of a list, neighbours are NOT created, though.
     *
     *  \param  orig  Original tree
     */
    Btree(const Btree * orig):
        m_order(orig->m_order),
        m_super(NULL),
        m_child(NULL),
        m_prev(this),
        m_next(NULL),
        value(orig->value)
    {
        if (NULL == orig->m_child) return;

        // Copy 1st child
        m_child = new Btree(orig->m_child);
        m_child->m_super = this;

        // Copy the rest of the children
        Btree * btree = orig->m_child->m_next;

        for (; NULL != btree; btree = btree->m_next) {
            Btree * copy  = new Btree(btree);
            copy->m_super = m_child;
            copy->m_prev  = m_child->m_prev;

            m_child->m_prev->m_next = copy;
            m_child->m_prev         = copy;
        }
    }

    public:

    /** Tree order getter */
    inline size_t order() const { return m_order; }

    /** Tree size getter */
    inline size_t size() const { return 1 << m_order; }

    /** Root check */
    inline bool is_root() const { return NULL == m_super; }

    /** 1st sibling (or in list) check */
    inline bool is_1st() const { return NULL == m_prev->m_next; }

    /** Last sibling (or in list) check */
    inline bool is_last() const { return NULL == m_next; }

    /** Parent getter */
    inline Btree * get_parent() const {
        if (NULL == m_super) return NULL;  // root has no parent

        return is_1st() ? m_super : m_super->m_super;
    }

    /** Child getter */
    inline Btree * get_child() const { return m_child; }

    /** Last child getter */
    inline Btree * get_last_child() const {
        return NULL != m_child ? m_child->m_prev : NULL;
    }

    /** Previous sibling getter */
    inline Btree * get_prev() const { return is_1st() ? NULL : m_prev; }

    /** Next sibling getter */
    inline Btree * get_next() const { return m_next; }

    /**
     *  \brief  Merge with another tree of the same order
     *
     *  Any two B_k trees may be merged to form B_(k+1) tree.
     *  The method makes the parameter to be last child of \c this.
     *  The operation is done in constant time.
     *
     *  \param  tree
     *
     *  \return \c this
     */
    void merge(Btree * tree) throw(std::logic_error) {
        assert(NULL != tree);

        if (m_order != tree->m_order)
            throw std::logic_error("attempt to merge incompatible binomial trees");

        if (m_order) {  // there is at least 1 child
            tree->m_super = m_child;
            tree->m_prev = m_child->m_prev;
            m_child->m_prev->m_next = tree;
            m_child->m_prev = tree;
        }
        else {  // merging B_0 trees
            tree->m_super = this;
            m_child = tree;
        }

        ++m_order;
    }

    /**
     *  \brief  Split tree to its root's children list
     *
     *  Make root of the tree B_0 and return its children list.
     *  The operation is done in O(\c m_order ) time since root's children's
     *  \c m_super pointer has to be cleared.
     *
     *  \return Children list
     */
    inline Btree * split() {
        Btree * child = m_child;

        for (; child != NULL; child = child->m_next)
            child->m_super = NULL;

        child   = m_child;
        m_order = 0;
        m_child = NULL;

        return child;
    }

    private:

    /**
     *  \brief  Deep destruction of a tree list
     *
     *  \param  btree  Tree (list)
     */
    static void delete_deep(Btree * btree) {
        while (NULL != btree) {
            Btree * next = btree->m_next;

            // Make the tree stand-alone
            btree->m_prev = btree;
            btree->m_next = NULL;

            // Destroy node
            delete btree;

            btree = next;
        }
    }

    public:

    /**
     *  \brief  Destructor
     *
     *  The destructor destroys whole tree to depth.
     *  The tree MUST be a stand-alone one; exception is
     *  thrown, otherwise.
     */
    ~Btree() throw(std::logic_error) {
        if (m_next != NULL || m_prev != this)
            throw std::logic_error("destruction of stand-alone binomial trees allowed, only");

        delete_deep(m_child);
    }

};  // end of template class Btree

}  // end of namespace impl


// Binomial heap
//template <typename T, class DecKey = impl::dec_key_default<T>, class Less = impl::less_default<T> >
template <typename T, class DecKey, class Less>
class binomial_heap {
    private:

    size_t           m_size;    /**< Number of nodes    */
    impl::Btree<T> * m_btrees;  /**< Binomial tree list */
    impl::Btree<T> * m_min;     /**< Ref. to minimum    */

    static const DecKey s_dec_key;  /**< Decrease key functor */
    static const Less   s_less;     /**< Comparator           */

    /**
     *  \brief  Merge 2 B_k trees, keeping binomial heap property
     *
     *  Root with lower value becomes the merge tree root.
     *  If both merged trees have root of the same value then the 1st argument
     *  is priviledged in the selection (and becomes the merge tree root).
     *
     *  Time complexity: O(1)
     *
     *  \param  t1  B_k tree
     *  \param  t2  B_k tree
     *
     *  \return root of the resulting B_(k+1) tree
     */
    static inline impl::Btree<T> * mergeTrees(impl::Btree<T> * t1, impl::Btree<T> * t2) {
        if (s_less(t2->value, t1->value)) {
            t2->merge(t1);

            return t2;
        }

        t1->merge(t2);

        return t1;
    }

    /**
     *  \brief  Join trees of the provided heap to this
     *
     *  Time complexity: O(1)
     *
     *  USE WITH CAUTION!
     *  MOST NOTABLY, MIND THE FOLLOWING:
     *
     *  The joined trees MUST have greater order than those of this object;
     *  otherwise, logical exception is thrown.
     *  Also note that minimum is NOT fixed by the function; if the provided
     *  heap's minimum is lower than the one of this object, the result
     *  shall be inconsistent.
     *  It also doesn't change the size counters.
     *  Caller is responsible to ensure the above.
     *
     *  The provided heap is emptied by the operation.
     *
     *  \param  heap  Source of the trees joined in
     */
    void join_in(binomial_heap & heap) throw(std::logic_error) {
        // Our heap is empty
        if (NULL == m_btrees)
            m_btrees = heap.m_btrees;

        // Join lists
        else
            m_btrees->join_list(heap.m_btrees);

        // Empty the argument
        heap.m_btrees = NULL;
        heap.m_min    = NULL;
    }

    /**
     *  \brief  Remove tree form the heap list
     *
     *  Time complexity: O(1)
     *
     *  USE WITH EXTRA CAUTION!
     *  MOST NOTABLY, MIND THE FOLLOWING:
     *
     *  It's responsibility of the caller to make sure that the tree
     *  indeed belongs to the heap.
     *  Also note that the function DOESN'T take care about
     *  minimum fixing, so if the tree root is the heap minimum
     *  and if it's removed from the heap for good then the heap
     *  is no longer consistent.
     *  It also doesn't change the size counter.
     *
     *  \param  btree  Tree
     */
    inline void unlist(impl::Btree<T> * btree) {
        assert(NULL != btree);

        m_btrees = btree->unlist_from(m_btrees);
    }

    /**
     *  \brief  Enlist a tree before another one
     *
     *  Time complexity: O(1)
     *
     *  USE WITH EXTRA CAUTION!
     *  MOST NOTABLY, MIND THE FOLLOWING:
     *
     *  It's responsibility of the caller to make sure that the tree
     *  that is provided as the right sibling-to-be indeed belongs
     *  to the heap.
     *  Also note that the function DOESN'T take care about
     *  minimum fixing, so if the root of the inserted tree has
     *  lower value than the current heap minimum then the heap
     *  is no longer consistent.
     *  It also doesn't change the size counter.
     *
     *  \param  btree_my       The inserted node's right sibling-to-be
     *  \param  btree_foreign  The inserted node
     */
    inline void enlist_left_of(impl::Btree<T> * btree_my, impl::Btree<T> * btree_foreign) {
        assert(NULL != btree_my);
        assert(NULL != btree_foreign);

        btree_foreign->enlist_left_of(btree_my);

        if (btree_foreign->is_1st())
            m_btrees = btree_foreign;
    }

    /**
     *  \brief  Merge implementation
     *
     *  Note that we know that both \c this and \c heap are not empty.
     *  ALSO NOTE that the function DOES NOT fix the size counter.
     *
     *  \param  heap  Heap merged in
     */
    void merge_impl(binomial_heap & heap) {
        assert(NULL != m_btrees);
        assert(NULL != m_min);
        assert(NULL != heap.m_btrees);
        assert(NULL != heap.m_min);

        // Fix minimum
        if (s_less(heap.m_min->value, m_min->value))
            m_min = heap.m_min;

        impl::Btree<T> * btree = m_btrees;

        while (NULL != btree) {
            impl::Btree<T> * heap_btree_1st = heap.m_btrees;

            if (NULL == heap_btree_1st) break;  // nothing more to do

            // Order too low to merge trees
            if (btree->order() < heap_btree_1st->order()) {
                btree = btree->get_next();
            }

            // No need to merge trees
            else if (btree->order() > heap_btree_1st->order()) {
                // Move tree from heap to this before current btree
                heap.unlist(heap_btree_1st);

                enlist_left_of(btree, heap_btree_1st);
            }

            else {  // Merge trees of the same order
                impl::Btree<T> * next = btree->get_next();

                // Remove trees from their respective heaps
                unlist(btree);
                heap.unlist(heap_btree_1st);

                // Merge trees
                btree = mergeTrees(btree, heap_btree_1st);

                // Merge the resulting tree to heap
                binomial_heap<T> tmp(btree);

                heap.merge_from(tmp);

                // Fix minimum if necessary
                // Minimum may cease to be a tree root if the minimal value
                // is born by multiple items.
                // Since merging generally changes the heap minimum,
                // the handle doesn't need to be preserved, so just switching
                // to the tree root is enough (the former min. parents can't be
                // greater).
                for (;;) {
                    impl::Btree<T> * parent = m_min->get_parent();

                    if (NULL == parent) break;

                    m_min = parent;
                }

                btree = next;  // next tree
            }
        }

        // Append rest of trees in heap to this tree list end
        join_in(heap);

        // Minimum should always be in a tree root
        assert(NULL != m_min);
        assert(m_min->is_root());
    }

    /** Find & set minimum of non-empty heap */
    inline void set_min() {
        m_min = m_btrees;

        assert(NULL != m_min);

        impl::Btree<T> * btree = m_min->get_next();

        for (; NULL != btree; btree = btree->get_next())
            if (!s_less(m_min->value, btree->value))
                m_min = btree;
    }

    /**
     *  \brief  Create heap from tree list
     *
     *  IMPORTANT NOTE:
     *  The constructor DOES NOT set the size counter.
     *
     *  \param  btrees  Tree list
     */
    binomial_heap(impl::Btree<T> * btrees):
        m_size(0),
        m_btrees(btrees),
        m_min(btrees)
    {
        assert(NULL != m_btrees);

        set_min();  // fix minimum
    }

    public:

    /**
     *  \brief  Item handle
     *
     *  Items in the heap are accessible via their handle returned by
     *  the \ref add method.
     */
    typedef impl::Btree<T> * item_handle;

    /** Constructor of empty heap */
    binomial_heap():
        m_size(0),
        m_btrees(NULL),
        m_min(NULL) {}

    /**
     *  \brief  Copy constructor
     *
     *  \param  orig  Original heap
     */
    binomial_heap(const binomial_heap<T> & orig):
        m_size(orig.m_size),
        m_btrees(NULL),
        m_min(NULL)
    {
        impl::Btree<T> * btree = orig.m_btrees;

        if (NULL == btree) return;

        // Copy 1st tree
        m_btrees = m_min = new impl::Btree<T>(btree);

        // Copy the other trees
        btree = btree->get_next();

        for (; NULL != btree; btree = btree->get_next()) {
            impl::Btree<T> * copy = new impl::Btree<T>(btree);

            m_btrees->join_list(copy);

            if (orig.m_min == btree)
                m_min = copy;
        }
    }

    /** Check whether the heap is empty */
    inline bool empty() const { return 0 == m_size; }

    /** Get heap size */
    inline size_t size() const { return m_size; }

    /** Minimum getter */
    inline const T & get_min() const throw(std::logic_error) {
        if (NULL == m_btrees)
            throw std::logic_error("empty heap min. access");

        assert(NULL != m_min);

        return m_min->value;
    }

    /**
     *  \brief  Swap objects
     *
     *  \param  heap  Another heap
     */
    void swap(binomial_heap & heap) {
        impl::Btree<T> * tmp_btree;

        // Swap sizes
        size_t tmp_size = heap.m_size;
        heap.m_size     = m_size;
        m_size          = tmp_size;

        // Swap tree lists
        tmp_btree     = heap.m_btrees;
        heap.m_btrees = m_btrees;
        m_btrees      = tmp_btree;

        // Swap minimums
        tmp_btree  = heap.m_min;
        heap.m_min = m_min;
        m_min      = tmp_btree;
    }

    /**
     *  \brief  Merge heaps
     *
     *  This method merges the argument to this object (directly, no copying).
     *  The argument becomes empty.
     *  The operation is done in O(log n+m) time (n and m being item
     *  counts of the heaps merged).
     *
     *  Note that the operation changes the heap minimum in general.
     *  Minimum may NOT be represented by the same item (and therefore handle)
     *  after the operation even if the merged heap's minimum is greater than
     *  the one of this.
     *  Such a situation may happen if the min. value has multiplicity
     *  in the heap(s).
     *
     *  \param  heap  Binomial heap
     */
    void merge_from(binomial_heap & heap) {
        // Nothing to do for empty argument
        if (NULL == heap.m_btrees) return;

        // Only swap required if this is empty
        if (NULL == m_btrees)
            swap(heap);

        // Well, let's have a go
        else {
            merge_impl(heap);

            m_size += heap.m_size;
        }
    }

    /**
     *  \brief  Merge heaps
     *
     *  Merge copy of the argument into the heap.
     *  The operation is done in O(m + log n+m) time (n and m being item
     *  counts of the heaps merged).
     *  The O(m) time is spent on creation of the argument copy.
     *  If you don't need the argument any more, use \ref merge_from
     *  which uses its content, directly, and therefore reaches asymptotically
     *  lower complexity.
     *
     *  \param  heap  Merged heap
     */
    inline void merge_in(const binomial_heap & heap) {
        binomial_heap copy(heap);

        merge_from(copy);
    }

    /**
     *  \brief  Heaps merge constructor
     *
     *  Create merge of the arguments.
     *  The operation is done in O(n + m + log n+m) time (n and m being item
     *  counts of the heaps merged).
     *  The O(n + m) time is spent on creation of the arguments copies.
     *  If you don't need (one of) the arguments any more, use \ref merge_from
     *  (or \ref merge_in) which use their content, directly, and therefore reach
     *  asymptotically lower complexity.
     *
     *  \param  heap1  Merged heap
     *  \param  heap2  Merged heap
     */
    binomial_heap(const binomial_heap & heap1, const binomial_heap & heap2):
        m_btrees(NULL),
        m_min(NULL)
    {
        binomial_heap copy(heap1);

        swap(copy);

        merge_in(heap2);
    }

    /**
     *  \brief  Add value
     *
     *  The method creates new heap item with the value
     *  and adds it to the heap.
     *  The item handle is returned and may be passed to
     *  methods like \ref decrease_key or \ref delete_item.
     *  It is guarranteed that the handle will not change
     *  throughout the life of the item.
     *
     *  \param  val  Value
     *
     *  \return Item handle
     */
    item_handle add(const T & val) {
        impl::Btree<T> * handle = new impl::Btree<T>(val);

        binomial_heap single(handle);
        single.m_size = 1;

        merge_from(single);

        return handle;
    }

    /**
     *  \brief  Decrease key
     *
     *  Note that the method prefers the item with the decreased key
     *  over the other items in ordering; meaning that the item
     *  gets higher in its binomial tree than any other item of the same
     *  key.
     *  If the item's key becomes equal to the heap's minimum then the item
     *  becomes the heap's claimed minimum.
     *
     *  IMPORTANT NOTE:
     *  The item handle MAY CHANGE by the operation!
     *
     *  \param  handle  Item handle
     *  \param  dec     Decrement
     */
    void decrease_key(item_handle handle, const T & dec) {
        assert(NULL != handle);
        assert(NULL != m_btrees);
        assert(NULL != m_min);

        s_dec_key(handle->value, dec);

        // Find new position in binomial tree
        for (;;) {
            impl::Btree<T> * parent = handle->get_parent();

            if (NULL == parent) break;

            // Due place found
            if (s_less(parent->value, handle->value)) break;

            impl::Btree<T>::swap_nodes(handle, parent);
        }

        // Fix minimum
        if (!s_less(m_min->value, handle->value))
            m_min = handle;
    }

    /**
     *  \brief  Decrease key of minimum
     *
     *  The method calls \ref decrease_key for the heap minimum.
     *
     *  \param  dec     Decrement
     */
    inline void decrease_min(const T & dec) { decrease_key(m_min, dec); }

    /**
     *  \brief  Delete an item
     *
     *  After the operation, the handle is invalidated.
     *
     *  \param  handle  Item handle
     */
    void delete_item(item_handle & handle) {
        assert(NULL != handle);
        assert(NULL != m_btrees);
        assert(NULL != m_min);

        // Move item to its binomial tree root
        for (;;) {
            impl::Btree<T> * parent = handle->get_parent();

            if (NULL == parent) break;

            impl::Btree<T>::swap_nodes(handle, parent);
        }

        unlist(handle);  // Remove the tree from the heap

        // Create new heap from the item children
        impl::Btree<T> * children = handle->split();

        if (NULL != children) {
            // Fix minimum
            if (m_min == handle && NULL != m_btrees)
                set_min();

            binomial_heap heap(children);

            // Merge from the heap
            if (NULL == m_btrees) {
                // Pre-fix sizes
                m_size = 0;
                heap.m_size = m_size - 1;

                swap(heap);
            }
            else {
                merge_impl(heap);

                --m_size;  // fix size
            }
        }

        // B_0 was removed and bears the minimum of non-empty heap
        else if (m_min == handle) {
            // Fix minimum
            if (NULL != m_btrees)
                set_min();
            else
                m_min = NULL;

            // Fix size
            --m_size;
        }

        // Remove item and invalidate handle
        delete handle;
        handle = NULL;
    }

    /**
     *  \brief  Delete minimum
     *
     *  The method calls \ref delete_item for the heap minimum.
     */
    inline void delete_min() {
        impl::Btree<T> * min = m_min;

        delete_item(min);
    }

    /**
     *  \brief  Destructor
     *
     *  Non-empty heap's trees shall be destroyed.
     */
    ~binomial_heap() {
        while (NULL != m_btrees) {
            impl::Btree<T> * btree = m_btrees;

            unlist(btree);

            delete btree;
        }
    }

};  // end of template class binomial_heap

// Template class binomial_heap static members initialisations
template <typename T, class DecKey, class Less>
const DecKey binomial_heap<T, DecKey, Less>::s_dec_key;
template <typename T, class DecKey, class Less>
const Less binomial_heap<T, DecKey, Less>::s_less;

}  // end of namespace container

#endif  // end of #ifndef container__heap_hxx
