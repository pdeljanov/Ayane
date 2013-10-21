#ifndef STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_
#define STARGAZER_STDLIB_AUDIO_BUFFERPOOL_H_

#include <core/macros.h>
#include <memory>
#include <stack>

#include <iostream>

namespace Stargazer {
    namespace Audio {

        template< typename BufferType, typename Allocator >
        class RefCountedPool;
        
        
        template< typename Type, typename Allocator >
        class RefCountedPoolDeallocator
        {
            friend class RefCountedPool<Type, Allocator>;

            typedef RefCountedPool<Type, Allocator> PoolType;

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
        
        
       
        
        template< typename Type, typename Allocator >
        class RefCountedPool : public std::enable_shared_from_this< RefCountedPool< Type, Allocator > >
        {
            friend class RefCountedPoolDeallocator<Type, Allocator>;

            typedef RefCountedPool<Type, Allocator> PoolType;
            typedef RefCountedPoolDeallocator<Type, Allocator> PooledDeallocatorType;
            
        public:
            
            static std::shared_ptr<PoolType> create( const Allocator &allocator )
            {
                return std::shared_ptr<PoolType>( new RefCountedPool( allocator ) );
            }
            
            ~RefCountedPool()
            {
                // Raise the destructing flag. This is required so that when m_pool is
                // deallocated, the objects won't be reclaimed.
                m_destroying = true;
            }

            /**
             *  Acquires an object from the pool. If there are no free objects, it acquire() will
             *  allocate a new object using the creator functor given during Pool construction.
             */
            PooledRefCount<Type> acquire()
            {
                PooledRefCount<Type> ret;
                
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

            RefCountedPool( const Allocator &allocator ) :
                m_destroying(false),
                m_pool(),
                m_allocator(allocator)
            {
            }
            
            void reclaim( Type *object )
            {
                // Reclaim the buffer only if the Pool is not destructing, otherwise,
                // delete it.
                if( !m_destroying )
                {
                    std::cout << "[Pool] Reclaimed: " << *object << " @ " << object << std::endl;

                    std::weak_ptr< PoolType > self = this->shared_from_this();
                    m_pool.push( std::shared_ptr<Type>( object, PooledDeallocatorType(self) ) );
                }
                else
                {
                    std::cout << "[Pool] Delete: " << object << std::endl;
                    delete object;
                }
            }
            
            bool m_destroying;
            std::stack< PooledRefCount<Type> > m_pool;
            Allocator m_allocator;
            
        };
        
        
        
        
        
    }
}

#endif
