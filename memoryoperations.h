#pragma once

#include <stddef.h>
#include <map>

namespace Ayane
{

  class MemoryOperations
  {
    public:
      static MemoryOperations &getMemoryOperations();

      void *malloc4ByteAligned( size_t size );
      void *malloc8ByteAligned( size_t size );
      void *malloc16ByteAligned ( size_t size );
      
      void *mallocAligned( size_t size, size_t alignment );
      
      void freeAligned ( void *ptr );

  };

}
