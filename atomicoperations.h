#pragma once

#include <stddef.h>

namespace Ayane
{

  class AtomicOperations
  {
    public:
      static inline bool tryCompareExchange ( volatile unsigned int *reg, unsigned int expected, unsigned int value );

      static inline void doCompareExchange( volatile unsigned int *reg, unsigned int value );
      
  };

  inline bool AtomicOperations::tryCompareExchange ( volatile unsigned int* reg, unsigned int expected, unsigned int value )
  {
    return __sync_bool_compare_and_swap ( reg, expected, value );
  }
  
  inline void AtomicOperations::doCompareExchange ( volatile unsigned int* reg, unsigned int value )
  {
    unsigned int old = *reg;
   
    while( !tryCompareExchange( reg, old, value ) )
    {
      old = *reg;
    }
  }


}
