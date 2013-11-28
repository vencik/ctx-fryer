#ifndef mt__rwlock_hxx
#define mt__rwlock_hxx

#include "mt/mutex.hxx"
#include "mt/condition.hxx"

#include <stdexcept>
#include <cassert>


/**
 *  \brief  Read-lock scope
 *
 *  The macro locks a RW lock for reading till end of scope.
 *
 *  \param  rwlock  RW lock
 */
#define rlock4scope(rwlock) mt::scoperlock __scope_rlock_ # __LINE__(rwlock)

/**
 *  \brief  Read-unlock scope
 *
 *  The macro unlocks a RW lock for reading till end of scope.
 *
 *  \param  rwlock  RW lock
 */
#define runlock4scope(rwlock) mt::scoperunlock __scope_runlock_ # __LINE__(rwlock)

/**
 *  \brief  Write-lock scope
 *
 *  The macro locks a RW lock for writing till end of scope.
 *
 *  \param  rwlock  RW lock
 */
#define wlock4scope(rwlock) mt::scopewlock __scope_wlock_ # __LINE__(rwlock)

/**
 *  \brief  Write-unlock scope
 *
 *  The macro unlocks a RW lock for writing till end of scope.
 *
 *  \param  rwlock  RW lock
 */
#define wunlock4scope(rwlock) mt::scopewunlock __scope_wunlock_ # __LINE__(rwlock)


namespace mt {

/**
 *  \brief  RW lock
 *
 *  RW lock allows for more flexible locking of a critical section (CS)
 *  than mutex.
 *  It allows concurrent access of the CS for (self-claimed) readers;
 *  only writer has exclusive access.
 *
 *  By default, the RW lock favourises writers; i.e. as soon as a writer
 *  waits for the lock, all newcomming readers are suspended.
 *  This protects the writers from starving, being naturally handicapped
 *  by the need of exclusivity of their lock.
 *  If, however, the user wants to favourise readers, the implementation
 *  supports an alternative constructor.
 */
class rwlock {
    private:

    bool      m_r_fav;  /**< Readers are favoured        */
    int       m_r_cnt;  /**< CS readers count            */
    int       m_w_cnt;  /**< Writers count (<0 if in CS) */
    mutex     m_mutex;  /**< Members' mutex              */
    condition m_rcond;  /**< Reading allowed             */
    condition m_wcond;  /**< Writing allowed             */

    public:

    typedef enum { favourise_readers } fav_read_t;  /**< Readers favourised flag */

    /** Constructor (default) */
    rwlock(): m_r_fav(false), m_r_cnt(0), m_w_cnt(0) {}

    /** Constructor (readers' favourising) */
    rwlock(fav_read_t ): m_r_fav(true), m_r_cnt(0), m_w_cnt(0) {}

    /** Lock for reading */
    inline void rlock() {
        lock4scope(m_mutex);

        // Wait for writer in CS or any writer if favoured
        while (0 > m_w_cnt || (m_w_cnt && !m_r_fav))
            m_rcond.wait(m_mutex);

        ++m_r_cnt;  // going to read...
    }

    /**
     *  \brief  Try to lock for reading
     *
     *  \retval \c true  if locked
     *  \retval \c false if not locked
     */
    inline bool tryrlock() {
        bool locked = m_mutex.trylock();

        if (locked) {
            // Writer in CS or favoured
            if (0 > m_w_cnt || (m_w_cnt && !m_r_fav))
                locked = false;
            else
                ++m_r_cnt;  // going to read...

            m_mutex.unlock();
        }

        return locked;
    }

    /** Unlock for reading */
    inline void runlock() throw(std::logic_error) {
        lock4scope(m_mutex);

        if (0 == m_r_cnt)
            throw std::logic_error("RW lock readers underflow");

        --m_r_cnt;  // done reading

        // Last reader signalises to waiting writer
        if (0 == m_r_cnt && m_w_cnt)
            m_wcond.signal();
    }

    /** Lock for writing */
    inline void wlock() {
        lock4scope(m_mutex);

        // Declare writing intention
        m_w_cnt += 0 < m_w_cnt ? 1 : -1;

        // Wait for readers and writers in CS
        while (m_r_cnt || 0 > m_w_cnt)
            m_wcond.wait(m_mutex);

        assert(0 < m_w_cnt);

        m_w_cnt = -m_w_cnt;  // going to write...
    }

    /**
     *  \brief  Try to lock for writing
     *
     *  \retval \c true  if locked
     *  \retval \c false if not locked
     */
    inline bool trywlock() {
        bool locked = m_mutex.trylock();

        if (locked) {
            // Reader(s) or writers in CS
            if (m_r_cnt || 0 > m_w_cnt) {
                m_mutex.unlock();
                locked = false;
            }

            // Going to write...
            else {
                ++m_w_cnt;

                m_w_cnt = -m_w_cnt;
            }

            m_mutex.unlock();
        }

        return locked;
    }

    /** Unlock for writing */
    inline void wunlock() throw(std::logic_error) {
        lock4scope(m_mutex);

        if (0 == m_w_cnt)
            throw std::logic_error("RW lock writers underflow");

        assert(0 > m_w_cnt);

        m_w_cnt = -m_w_cnt;
        --m_w_cnt;

        // Signal to next writer
        if (m_w_cnt)
            m_wcond.signal();

        // Last writer broadcasts to waiting readers
        else
            m_rcond.broadcast();
    }

    private:

    /** Copying forbidden */
    rwlock(const rwlock & orig) {}

    /** Assignment forbidden */
    rwlock & operator = (const rwlock & orig) {}

};  // end of class rwlock


/**
 *  \brief  Scope read-lock
 *
 *  Simmilar to \ref scopelock, for RW lock on reading.
 *
 *  See \ref rlock4scope macro, too.
 */
class scoperlock {
    private:

    rwlock & m_rwlock;  /**< Associated RW lock */

    public:

    /** Read-lock RW lock */
    scoperlock(rwlock & rwl): m_rwlock(rwl) {
        m_rwlock.rlock();
    }

    /** Read-unlock RW lock */
    ~scoperlock() {
        m_rwlock.runlock();
    }

};  // end of class scoperlock

/**
 *  \brief  Scope read-unlock
 *
 *  Simmilar to \ref scopeunlock, for RW lock on reading.
 *
 *  See \ref runlock4scope macro, too.
 */
class scoperunlock {
    private:

    rwlock & m_rwlock;  /**< Associated RW lock */

    public:

    /** Read-unlock RW lock */
    scoperunlock(rwlock & rwl): m_rwlock(rwl) {
        m_rwlock.runlock();
    }

    /** Read-lock RW lock */
    ~scoperunlock() {
        m_rwlock.rlock();
    }

};  // end of class scoperunlock

/**
 *  \brief  Scope write-lock
 *
 *  Simmilar to \ref scopelock, for RW lock on writing.
 *
 *  See \ref wlock4scope macro, too.
 */
class scopewlock {
    private:

    rwlock & m_rwlock;  /**< Associated RW lock */

    public:

    /** Write-lock RW lock */
    scopewlock(rwlock & rwl): m_rwlock(rwl) {
        m_rwlock.wlock();
    }

    /** Write-unlock RW lock */
    ~scopewlock() {
        m_rwlock.wunlock();
    }

};  // end of class scopewlock

/**
 *  \brief  Scope write-unlock
 *
 *  Simmilar to \ref scopeunlock, for RW lock on writing.
 *
 *  See \ref wunlock4scope macro, too.
 */
class scopewunlock {
    private:

    rwlock & m_rwlock;  /**< Associated RW lock */

    public:

    /** Write-unlock RW lock */
    scopewunlock(rwlock & rwl): m_rwlock(rwl) {
        m_rwlock.wunlock();
    }

    /** Write-lock RW lock */
    ~scopewunlock() {
        m_rwlock.wlock();
    }

};  // end of class scopewunlock

}  // end of namespace mt

#endif  // end of #ifndef mt__rwlock_hxx
