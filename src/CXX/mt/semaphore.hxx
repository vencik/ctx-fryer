#ifndef mt__semaphore_hxx
#define mt__semaphore_hxx

#include "mt/mutex.hxx"
#include "mt/condition.hxx"

#include <stdexcept>


namespace mt {

/**
 *  \brief  Semaphore
 *
 *  The class implements semaphore with arbitrary capacity.
 *  Such a sync. primitive is usefull in situations where
 *  you need to block a thread (or process) till a pre-defined
 *  amount of concurrent tasks finish.
 *  In other words, it may be percieved as a mutex generalisation.
 *
 *  This implementations allows to set the semaphore capacity
 *  both upon construction and change it later.
 *  Changing the capacity is NOT typical operation; it may be useful
 *  in certain situations, though (typically when you don't know
 *  the required capacity at the semaphore construction time).
 *
 *  References:
 *  * http://en.wikipedia.org/wiki/Semaphore_(programming)
 */
class semaphore {
    protected:

    int       m_val;    /**< Semaphore current value  */
    mutex     m_mutex;  /**< Protective mutex         */
    condition m_open;   /**< Semaphore open condition */

    public:

    /**
     *  \brief  Constructor
     *
     *  Although zero or negative capacity may not make much sense
     *  at the 1st glance, such construction allows e.g. for
     *  waiting for certain amount of otherwise concurrent tasks.
     *  This implementation is permissive; it's the user's responsibility
     *  to decide whether such setting is sane.
     *
     *  \param  capacity  Semaphore capacity
     */
    semaphore(int capacity): m_val(capacity) {}

    /** Constructor of semaphore with capacity of 1 */
    semaphore(): m_val(1) {}

    /**
     *  \brief  Increase semaphore value
     *
     *  This function increases the semaphore value.
     *  Could be percieved as a posteriori semaphore capacity increase.
     *  Signalises if the resulting value is not negative.
     *
     *  IMPORTANT NOTE:
     *  Use the function with caution and only if you know
     *  what you are doing.
     *
     *  \param  diff  Addition argument
     *
     *  \return Current value
     */
    inline int inc(int diff) {
        lock4scope(m_mutex);

        m_val += diff;

        if (!(m_val < 0))
            m_open.signal();

        return m_val;
    }

    /**
     *  \brief  Decrease semaphore value
     *
     *  This function decreases the semaphore value.
     *  Could be percieved as a posteriori semaphore capacity decrease.
     *
     *  IMPORTANT NOTE:
     *  Use the function with caution and only if you know
     *  what you are doing.
     *
     *  \param  diff  Subtraction argument
     *
     *  \return Current value
     */
    inline int dec(int diff) {
        lock4scope(m_mutex);

        m_val -= diff;

        return m_val;
    }

    /** Pre-increment semaphore value by one (see \ref inc) */
    inline int operator ++ () { return inc(1); }

    /** Pre-decrement semaphore value by one (see \ref dec) */
    inline int operator -- () { return dec(1); }

    /**
     *  \brief  Wait on semaphore
     *
     *  The function first decrements the semaphore value.
     *  The current thread is then suspended (blocked)
     *  while the value is negative.
     *
     *  \return The semaphore current value
     */
    inline int wait() {
        lock4scope(m_mutex);

        --m_val;

        // Re-check the condition (due spurious wake-ups)
        while (0 > m_val)
            m_open.wait(m_mutex);

        return m_val;
    }

    /**
     *  \brief  Signal on semaphore
     *
     *  The signal operation increments the semaphore and
     *  if the resulting value is greater than 0, it signalises
     *  to (one) waiting thread.
     *
     *  NOTE:
     *  It may be observed that the function is equal
     *  to semaphore pre-increment.
     *
     *  \return The semaphore current value
     */
    inline int signal() { return ++(*this); }

    private:

    /** Copying is forbidden */
    semaphore(const semaphore & orig) {
        throw std::logic_error("semaphore copying forbidden");
    }

    /** Assignment is forbidden */
    semaphore & operator = (const semaphore & orig) {
        throw std::logic_error("semaphore assignment forbidden");
    }

};  // end of class semaphore

}  // end of namespace mt

#endif  // end of #ifndef mt__semaphore_hxx
