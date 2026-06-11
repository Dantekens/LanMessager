#pragma once

#include <vector>
#include <thread>
#include <boost/asio.hpp>



class PoolThread 
{
    private:

    std::vector<std::thread> pool;
    boost::asio::io_context& io;
    size_t size;

    public:

    void erease_thread(size_t size_remove_thread);
    void add_thread(size_t size_add_thread);
    void remove_all_thread();
    PoolThread(boost::asio::io_context& io,size_t size_pool = std::hardware_destructive_interference_size);
    ~PoolThread();
    void stop();
    
};