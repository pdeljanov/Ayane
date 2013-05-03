#include "memoryoperations.h"

#include <stdint.h>
#include <stdlib.h>

namespace Ayane
{

  MemoryOperations& MemoryOperations::getMemoryOperations()
  {
    static MemoryOperations memOps;
    return memOps;
  }

  void* MemoryOperations::malloc4ByteAligned ( size_t size )
  {
    return mallocAligned( size, 4 );
  }

  void* MemoryOperations::malloc8ByteAligned ( size_t size )
  {
    return mallocAligned( size, 8 );
  }

  void* MemoryOperations::malloc16ByteAligned ( size_t size )
  {
    return mallocAligned( size, 16 );
  }
  
  void* MemoryOperations::mallocAligned ( size_t size, size_t alignment )
  {   
    uintptr_t r = (uintptr_t)malloc(size + --alignment + 2);
    uintptr_t o = (r + 2 + alignment) & ~(uintptr_t)alignment;
    
    if (!r) 
      return NULL;
    
    ((uint16_t*)o)[-1] = (uint16_t)(o-r);
    
    return (void*)o;
  }
  
  void MemoryOperations::freeAligned ( void* ptr )
  {
    if (!ptr) 
      return;
    
    free((void*)((uintptr_t)ptr-((uint16_t*)ptr)[-1]));
  }


}
