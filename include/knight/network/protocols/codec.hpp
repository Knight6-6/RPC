#pragma once
#include "knight/network/core/net_session.hpp"
#include "unordered_map"

namespace knight::network
{

class codec
{
public:
    void codecin(std::unordered_map<uint64_t,std::unique_ptr<netsession>>::iterator it , int ready_fd);
    bool codecout();
    void codecrdhup();
private:
};

}

