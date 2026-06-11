#include "PoolThread.hpp"

void PoolThread::erease_thread(size_t size_remove_thread)
{
    if(size_remove_thread == 0) return;
    if(size_remove_thread >= size)
    {
        remove_all_thread();
        return;
    } 

    io.stop();
    for(std::thread& temp_thread : pool)
    {
        if(temp_thread.joinable())
        temp_thread.join();
    }
    
    size -= size_remove_thread;

   
    if(io.stopped()) io.restart();

    std::vector<std::thread> new_pool;
    new_pool.reserve(size);
    for(size_t i = 0; i < size; ++i)
    {
        new_pool.emplace_back([this](){this->io.run();});
    }

    pool = std::move(new_pool);
}
void PoolThread::add_thread(size_t size_add_thread)
{
    for(size_t i = 0; i < size_add_thread;++i)
    {
        pool.emplace_back([this](){this->io.run();});
    }
   
    size += size_add_thread;
}
void PoolThread::remove_all_thread()
{
    io.stop();
    for(std::thread& temp_thread : pool)
    {
        if(temp_thread.joinable())
        temp_thread.join();
    }
    pool.clear();
    pool.shrink_to_fit();
    size = 0;
}
void PoolThread::stop()
{
    remove_all_thread();
}

PoolThread::PoolThread(boost::asio::io_context& io,size_t size_pool = std::hardware_destructive_interference_size) : io(io)
{
    if(size_pool == 0) size_pool = 2;

    pool.reserve(size_pool);

    for(size_t iterator = 0; iterator < size_pool ;++iterator)
    {
        pool.emplace_back([this](){this->io.run();});
    }
    size = size_pool;
}
PoolThread::~PoolThread()
{
    if(size != 0) remove_all_thread();
}