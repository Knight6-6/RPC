#include "client.hpp"
#include <arpa/inet.h>

client::client(){}
client::~client(){}
client::client(sockaddr_in& sockaddr)
{
   char ip_buf[INET_ADDRSTRLEN];
   port=ntohs(sockaddr.sin_port);
   inet_ntop(AF_INET , &(sockaddr.sin_addr) , ip_buf , sizeof(ip_buf));
   ip=ip_buf;
   client_time =time(nullptr);
}
void client::init(sockaddr_in &sockaddr)
{
   char ip_buf[INET_ADDRSTRLEN];
   port=ntohs(sockaddr.sin_port);
   inet_ntop(AF_INET , &(sockaddr.sin_addr) , ip_buf , sizeof(ip_buf));
   ip=ip_buf;
   client_time =time(nullptr);
}

void client::clean()
{
    ip.clear();
    port=0;
    client_time=0;
}

std::string client:: get_ip()
{
    return ip;
}

unsigned short client::get_port()
{
    return port;
}

time_t client::get_time()
{
    return client_time;
}             