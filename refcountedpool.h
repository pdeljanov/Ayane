#ifndef STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_

#include <core/macros.h>
#include <mutex>
#include <memory>
#include <stack>

#include <iostream>

namespace Stargazer {
    namespace Audio {

        template< typename Type, typename Allocator >
        class RefCountedPoolImpl;
        
        //template< typename Type, typename Allocator >
        class RefCountedPoolFactory;
        
        
        template< typename Type, typename Allocator >
        class RefCountedPoolDeallocator
        {
            friend class RefCountedPoolImpl<Type, Allocator>;

            typedef RefCountedPoolImpl<Type, Allocator> PoolType;

        public:

            void operator() ( Type *object ) const
            {

                // If the ownership pool is still present, let the pool reclaim the object.
                // Otherwise, it's time to put the object down.
                if ( std::shared_ptr<PoolType> pool = m_pool.lock() )
                {
                    std::cout << "[PooledRefCountDeallocator] TryReclaim: " << *object << " @ " << object << std::endl;
                    pool->reclaim( object );
                }
                else
                {
                    std::cout << "[PooledRefCountDeallocator] Delete: " << object << std::endl;
                    delete object;
                }
            }
            
        private:
            
            RefCountedPoolDeallocator( std::weak_ptr<PoolType> &pool ) : m_pool(pool)
            {}
            
            std::weak_ptr<PoolType> m_pool;
        };
       
        
        /**
         *  PooledRefCount is a wrapper to add pooled reference counting to an object.
         *  This object can only be created by a Pool object.
         */
        template<typename Type>
        using PooledRefCount = std::shared_ptr<Type>;
        
        /**
         *  RefCountedPool implementation. Instances of RefCountedPoolImpl must be wrapped by a std::shared_ptr, 
         *  therefore, this class cannot be instantiated publically. Use RefCountedPoolFactory to create new 
         *  RefCountedPools.
         */
        template< typename Type, typename Allocator >
        class RefCountedPoolImpl : public std::enable_shared_from_this< RefCountedPoolImpl< Type, Allocator > >
        {
            friend class RefCountedPoolDeallocator<Type, Allocator>;
            friend class RefCountedPoolFactory;

            typedef RefCountedPoolImpl<Type, Allocator> PoolType;
            typedef RefCountedPoolDeallocator<Type, Allocator> PooledDeallocatorType;
            
        public:
            
            /** 
             *  Destroys the RefCountedPool. Any free objects in the pool will be deallocated.
             *  Objects owned by the pool, will be freed once their reference count reaches 0.
             */
            ~RefCountedPoolImpl(){}

            /**
             *  Acquires an object from the pool. If there are no free objects, acquire will
             *  allocate a new object using the creator functor given during Pool construction.
             */
            PooledRefCount<Type> acquire()
            {
                PooledRefCount<Type> ret;

                // Lock the pool.
                std::lock_guard<std::mutex> lock(m_poolMutex);
                
                if( m_pool.empty() )
                {
                    std::cout << "[Pool] Acquire -> ";
                    std::weak_ptr<PoolType> self = this->shared_from_this();
                    ret = std::shared_ptr<Type>(m_allocator(), PooledDeallocatorType(self));
                }
                else
                {
                    ret = m_pool.top();
                    m_pool.pop();
                    std::cout << "[Pool] Acquire -> Pop: " << *ret << " @ " << ret.get() << std::endl;
                }
                
                return ret;
            }
            
        private:
            STARGAZER_DISALLOW_COPY_AND_ASSIGN(RefCountedPoolImpl);

            RefCountedPoolImpl( const Allocator &allocator ) :
                m_allocator(allocator)
            {
            }
            
            /**
             *  Preallocates count objects and pushes them into the pool.
             */
            void preallocate( int count )
            {
                std::weak_ptr< PoolType > self = this->shared_from_this();

                // Lock the pool.
                std::lock_guard<std::mutex> lock(m_poolMutex);
                
                for( int i = 0; i < count; ++i )
                    m_pool.push( std::shared_ptr<Type>(m_allocator(), PooledDeallocatorType(self)) );
            }
            
            /**
             *  Reclaims an object that is owned by the pool. Reclaimed objects will be pushed
             *  back into the pool's free list.
             */
            void reclaim( Type *object )
            {
                // Lock the pool.
                std::lock_guard<std::mutex> lock(m_poolMutex);
                
                std::cout << "[Pool] Reclaimed: " << *object << " @ " << object << std::endl;

                std::weak_ptr< PoolType > self = this->shared_from_this();
                m_pool.push( std::shared_ptr<Type>(object, PooledDeallocatorType(self)) );
            }
            
            // Mutex for protecting pool's free stack.
            std::mutex m_poolMutex;
            
            // The pool's free stack.
            std::stack< PooledRefCount<Type> > m_pool;
            
            // Pool object allocator.
            Allocator m_allocator;
        };
        
        class RefCountedPoolFactory
        {
        public:
            
            /**
             *  Creates a RefCountedPool that uses the specified allocator for pool object
             *  creation.
             */
            template<typename Type, typename Allocator>
            static std::shared_ptr<RefCountedPoolImpl<Type, Allocator>> create( const Allocator &allocator )
            {
                typedef RefCountedPoolImpl<Type, Allocator> PoolType;
                return std::shared_ptr<PoolType>( new PoolType( allocator ) );
            }
            
            /**
             *  Creates a RefCountedPool with a specified number of preallocated objects.
             *  The specified allocator is used for pool object creation.
             */
            template<typename Type, typename Allocator>
            static std::shared_ptr<RefCountedPoolImpl<Type, Allocator>> create( const Allocator &allocator, int count )
            {
                typedef RefCountedPoolImpl<Type, Allocator> PoolType;
                std::shared_ptr<PoolType> pool( new PoolType( allocator ) );
                pool->preallocate( count );
                return pool;
            }
            
        private:
            STARGAZER_DISALLOW_DEFAULT_CTOR_COPY_AND_ASSIGN(RefCountedPoolFactory);
        };
        
        
        /**
         *  RefCountedPool is a generic object pool with automatic object reclaimation.
         *  Objects in the pool are automatically allocated via the specified allocator functor.
         *  If the pool is empty, the pool will allocate a new object. If the pool is not empty,
         *  the caller requesting the object will receive a reused object. When objects are destroyed
         *  the pool will automatically reclaim the object for reuse. If the pool is destroyed before
         *  an object can be reclaimed, the object will not be reclaimed but destroyed.
         */
        template< typename Type, typename Allocator >
        using RefCountedPool = std::shared_ptr< RefCountedPoolImpl<Type, Allocator> >;
        


        
    }
}

#endif
