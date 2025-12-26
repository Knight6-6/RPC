#pragma once
#include <string>
#include <netinet/in.h>
#include <ctime>

class client
{
public:
       client();
       client(sockaddr_in &sockaddr);
       ~client();
       void init(sockaddr_in &sockaddr);
       void clean();
       std::string get_ip();
       unsigned short get_port();
       time_t get_time();
private:
       std::string ip;
       unsigned short port;
       time_t client_time;
};