/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef AYANE_ALIGNEDMEMORY_H_
#define AYANE_ALIGNEDMEMORY_H_

#include <cstdint>
#include <cstdlib>

namespace Ayane
{
    
    class AlignedMemory
    {
    public:
        
        template< typename T >
        static T *allocate( unsigned int count, size_t alignment );
        
        template< typename T >
        static void deallocate ( T *ptr );
        
        template< typename  T >
        static T *allocate4( unsigned int count );
        
        template< typename T >
        static T *allocate8( unsigned int count );
        
        template< typename T >
        static T *allocate16 ( unsigned int count );
        
        template< typename T >
        static T *allocate32 ( unsigned int count );
        
    };
    
    
    template< typename T >
    T* AlignedMemory::allocate ( unsigned int count, size_t alignment )
    {
        size_t size = sizeof(T) * count;
        
        uintptr_t r = (uintptr_t)malloc(size + --alignment + 2);
        
        if (!r)
            return nullptr;
        
        uintptr_t o = (r + 2 + alignment) & ~(uintptr_t)alignment;
        ((uint16_t*)o)[-1] = (uint16_t)(o-r);
        
        return reinterpret_cast<T*>(o);
    }
    
    template< typename T >
    void AlignedMemory::deallocate ( T* ptr )
    {
        if (ptr == nullptr)
            return;
        
        void *r = reinterpret_cast<void*>(ptr);
        
        free((void*)((uintptr_t)r-((uint16_t*)r)[-1]));
    }
    
    template< typename  T >
    inline T *AlignedMemory::allocate4( unsigned int count )
    {
        return AlignedMemory::allocate<T>(count, 4);
    }
    
    template< typename T >
    inline T *AlignedMemory::allocate8( unsigned int count )
    {
        return AlignedMemory::allocate<T>(count, 8);
    }
    
    template< typename T >
    inline T *AlignedMemory::allocate16 ( unsigned int count )
    {
        return AlignedMemory::allocate<T>(count, 16);
    }
    
    template< typename T >
    inline T *AlignedMemory::allocate32 ( unsigned int count )
    {
        return AlignedMemory::allocate<T>(count, 32);
    }
    
    
}

#endif
