/*
 *
 * Copyright (c) 2013 Philip Deljanov. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef STARGAZER_STDLIB_AUDIO_POOL_H_
#define STARGAZER_STDLIB_AUDIO_POOL_H_

#include <core/macros.h>
#include <mutex>
#include <memory>
#include <stack>

#include <iostream>

namespace Stargazer {
    namespace Audio {

        template< typename Type, typename Allocator >
        class PoolImpl;
        
        //template< typename Type, typename Allocator >
        class PoolFactory;
        

        
        template< typename Type, typename Allocator >
        class PoolDeallocator
        {
            friend class PoolImpl<Type, Allocator>;

            typedef PoolImpl<Type, Allocator> PoolType;

        public:

            void operator() (Type *object) const
            {

                // If the ownership pool is still present, let the pool reclaim the object.
                // Otherwise, it's time to put the object down.
                if ( std::shared_ptr<PoolType> pool = mPool.lock() )
                {
                    
                    std::cout << "[PooledRefCountDeallocator] TryReclaim: "
                    << object << std::endl;
                    
                    pool->reclaim(object);
                }
                else
                {
                    std::cout << "[PooledRefCountDeallocator] Delete: "
                    << object << std::endl;
                    
                    delete object;
                }
            }
            
        private:
            
            PoolDeallocator( std::weak_ptr<PoolType> &pool ) : mPool(pool)
            {
            }
            
            std::weak_ptr<PoolType> mPool;
        };
        
        
        
        
        
        /**
         *  Pooled is a wrapper to add pooled reference counting to an object.
         *  This object can only be created by a Pool object.
         */
        template<typename Type, typename DeallocatorType>
        using Pooled = std::unique_ptr<Type, DeallocatorType>;
        
        
        /**
         *  Pool implementation. Instances of PoolImpl must be wrapped by a std::shared_ptr, 
         *  therefore, this class cannot be instantiated publically. Use PoolFactory to create new 
         *  Pools.
         */
        template< typename Type, typename Allocator >
        class PoolImpl : public std::enable_shared_from_this<PoolImpl<Type, Allocator>>
        {
            friend class PoolDeallocator<Type, Allocator>;
            friend class PoolFactory;

            typedef PoolImpl<Type, Allocator> PoolType;
            typedef PoolDeallocator<Type, Allocator> PooledDeallocatorType;
            
        public:
            
            /** 
             *  Destroys the Pool. Any free objects in the pool will be deallocated.
             *  Objects owned by the pool, will be freed once their reference count reaches 0.
             */
            ~PoolImpl(){}

            /**
             *  Acquires an object from the pool. If there are no free objects, acquire will
             *  allocate a new object using the creator functor given during Pool construction.
             */
            Pooled<Type, PooledDeallocatorType> acquire()
            {
                std::unique_ptr<Type, PooledDeallocatorType> ret;
                
                // Lock the pool.
                std::lock_guard<std::mutex> lock(mPoolMutex);
                
                if( mPool.empty() )
                {
                    std::cout << "[Pool] Acquire -> ";
                    
                    std::weak_ptr<PoolType> self = this->shared_from_this();
                    ret = std::unique_ptr<Type, PooledDeallocatorType>(mAllocator(),
                                                                       PooledDeallocatorType(self));
                }
                else
                {
                    ret = std::move(mPool.top());
                    mPool.pop();
                    
                    std::cout << "[Pool] Acquire -> Pop: " << ret.get() << std::endl;
                }
                
                return ret;
            }
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(PoolImpl);

            PoolImpl( const Allocator &allocator ) :
                mAllocator(allocator)
            {
            }
            
            /**
             *  Preallocates count objects and pushes them into the pool.
             */
            void preallocate( int count )
            {
                std::weak_ptr<PoolType> self = this->shared_from_this();

                // Lock the pool.
                std::lock_guard<std::mutex> lock(mPoolMutex);
                
                for( int i = 0; i < count; ++i ) {
                    mPool.push(Pooled<Type, PooledDeallocatorType>(mAllocator(),
                                                                   PooledDeallocatorType(self)));
                }
            }
            
            /**
             *  Reclaims an object that is owned by the pool. Reclaimed objects will be pushed
             *  back into the pool's free list.
             */
            void reclaim(Type *object)
            {
                // Lock the pool.
                std::lock_guard<std::mutex> lock(mPoolMutex);
                
                std::cout << "[Pool] Reclaimed: " << object << std::endl;

                std::weak_ptr<PoolType> self = this->shared_from_this();
                mPool.push(Pooled<Type, PooledDeallocatorType>(object,
                                                               PooledDeallocatorType(self)));
            }
            
            // Mutex for protecting pool's free stack.
            std::mutex mPoolMutex;
            
            // The pool's free stack.
            std::stack<Pooled<Type, PooledDeallocatorType>> mPool;
            
            // Pool object allocator.
            Allocator mAllocator;
        };
        
        class PoolFactory
        {
        public:
            
            /**
             *  Creates a Pool that uses the specified allocator for pool object
             *  creation.
             */
            template<typename Type, typename Allocator>
            static std::shared_ptr<PoolImpl<Type, Allocator>> create( const Allocator &allocator )
            {
                typedef PoolImpl<Type, Allocator> PoolType;
                return std::shared_ptr<PoolType>( new PoolType( allocator ) );
            }
            
            /**
             *  Creates a Pool with a specified number of preallocated objects.
             *  The specified allocator is used for pool object creation.
             */
            template<typename Type, typename Allocator>
            static std::shared_ptr<PoolImpl<Type, Allocator>> create( const Allocator &allocator, int count )
            {
                typedef PoolImpl<Type, Allocator> PoolType;
                std::shared_ptr<PoolType> pool( new PoolType( allocator ) );
                pool->preallocate( count );
                return pool;
            }
            
        private:
            STARGAZER_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(PoolFactory);
        };
        
        
        
        
        /**
         *  Pool is a generic object pool with automatic object reclaimation.
         *  Objects in the pool are automatically allocated via the specified allocator functor.
         *  If the pool is empty, the pool will allocate a new object. If the pool is not empty,
         *  the caller requesting the object will receive a reused object. When objects are destroyed
         *  the pool will automatically reclaim the object for reuse. If the pool is destroyed before
         *  an object can be reclaimed, the object will not be reclaimed but destroyed.
         */
        template<typename Type, typename Allocator>
        using Pool = std::shared_ptr<PoolImpl<Type, Allocator>>;
        


        
    }
}

#endif
