#pragma once

#include <stddef.h>

#include "atomicoperations.h"

namespace Ayane
{

  template< typename T, size_t N >
  class LocklessQueue
  {
    public:

      LocklessQueue();

      bool enqueue ( T *elem );
      bool dequeue ( T **elem );

    private:

      volatile unsigned int m_front;
      volatile unsigned int m_back;

      T *m_elems[N + 1];

  };

  template< typename T, size_t N >
  LocklessQueue< T, N >::LocklessQueue() : m_front ( 0 ), m_back ( 0 )
  {

  }

  template< typename T, size_t N >
  bool LocklessQueue<T, N>::enqueue ( T* elem )
  {
    int back = m_back;
    int front = m_front;
    int nBack = ( back + 1 ) % ( N + 1 );

    if ( nBack == front )
      return false;

    m_elems[back] = elem;

    // Atomically update m_back. This should not fail since m_back is never written
    // to anywhere else
    AtomicOperations::tryCompareExchange ( &m_back, back, nBack );

    return true;
  }

  template< typename T, size_t N >
  bool LocklessQueue<T, N>::dequeue ( T** elem )
  {
    int back = m_back;

    int front = m_front;
    int nFront = ( front + 1 ) % ( N + 1 );

    if ( front == back )
      return false;

    *elem = m_elems[front];

    // Atomically update m_front. This should not fail since m_front is never written to
    // anywhere else.
    AtomicOperations::tryCompareExchange ( &m_front, front, nFront );

    return true;
  }

}
