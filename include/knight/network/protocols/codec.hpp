#pragma once
#include "knight/network/core/net_session.hpp"
#include "unordered_map"

namespace knight::network
{

class codec
{
public:
    codec() =default;
    void codecin(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it , int ready_fd);
    bool codecout(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd );
    void codecrdhup(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it_se ,int ready_fd );
    void codecudpin(std::shared_ptr<netsession> se, int fd);
private:
};

}

