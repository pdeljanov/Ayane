#pragma once

#include <stddef.h>

#include "locklessqueue.h"

namespace Ayane
{

  /** Implements a recycling queue without locks.
    *
    * A recycling queue is one in which objects that are dequeued can be reused and enqueued again.
    * The lifecycle of an object is as follows:
    *
    *  i.   Object in free pool (Ownership: Queue)
    *  ii.  Object acquired from free pool by caller (Ownership: Caller)
    *  iii. Object enqueued (Ownership: Queue)
    *  iv.  Object dequeued (Ownership: Caller)
    *  v.   Object released back into free pool (Ownership: Queue)
    *
    * Through this mechanism a fixed set of objects can be cycled between the queue and the caller.
    *
    * This class is implemented without locks and is thread-safe for single-producer, single-consumer use cases.
    * Producers should only call obtain, and enqueue, while consumers should only call dequeue and release.
    * All calls are non-blocking. Calls which may fail (such as dequeuing from an empty queue) will return a status.
  **/
  template< typename T, size_t N >
  class LocklessRecyclingQueue
  {
    public:

      /** Trys to retrieve a pointer to a free object.
        *
        * \param elem The pointer to the free object. NULL if there are no free objects.
        * \returns Returns true if a free object could be returned. False if there are
        *          no free objects.
      **/
      bool obtain ( T **elem );

      /** Enqueues an object onto the queue.
        *
        * \param elem A pointer to the object to be enqueued.
      **/
      void enqueue ( T *elem );

      /** Trys to dequeue an object from the queue.
        *
        * \param elem The pointer to the dequeued object. NULL if there are no objects in the queue.
        * \returns Returns true if an object could be dequeued. False otherwise.
      **/
      bool dequeue ( T **elem );

      /** Releases an object and allow it to be reused again.
        *
        * \param elem The pointer to the object to be reused.
      **/
      void release ( T *elem );

      /** Releases all objects in the queue.
        *
        * This function is not thread-safe.
      **/ 
      void freeAll( );

  private:

      LocklessQueue<T, N> m_free;
      LocklessQueue<T, N> m_queue;

  };

  template< typename T, size_t N >
  bool LocklessRecyclingQueue<T, N>::obtain ( T** elem )
  {
    return m_free.dequeue ( elem );
  }

  template< typename T, size_t N >
  void LocklessRecyclingQueue<T, N>::enqueue ( T* elem )
  {
    m_queue.enqueue ( elem );
  }

  template< typename T, size_t N >
  bool LocklessRecyclingQueue<T, N>::dequeue ( T** elem )
  {
    return m_queue.dequeue ( elem );
  }

  template< typename T, size_t N >
  void LocklessRecyclingQueue<T, N>::release ( T* elem )
  {
    m_free.enqueue ( elem );
  }
  
  template< typename T, size_t N >
  void LocklessRecyclingQueue<T, N>::freeAll()
  {
    T *elem = NULL;
    while( m_free.dequeue( &elem ) ) delete elem;
    while( m_queue.dequeue( &elem ) ) delete elem;
  }


}
