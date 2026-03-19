#include <knight/naming/naming_server.hpp>
#include <iostream>
#include <unistd.h>

int main() {
    // 监听 8888 端口
    knight::naming::namingserver ns(8888);
    std::cout << "[NamingServer] 启动在 8888 端口..." << std::endl;
    ns.start(); 
    // 防止主线程退出
    while(true) {
        sleep(10);
    }
    return 0;
}