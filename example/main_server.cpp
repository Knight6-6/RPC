#include <knight/rpc/rpc_server.hpp>
#include <iostream>
#include <string>
#include <cstdio> // 使用 printf 更稳
#include "search.pb.h"

using namespace knight::rpc;

class SearchServiceImpl : public knight::rpc::test::SearchService {
public:
    void Search(google::protobuf::RpcController* controller,
                const knight::rpc::test::SearchRequest* request,
                knight::rpc::test::SearchResponse* response,
                google::protobuf::Closure* done) override {
        
        // --- 核心补丁：打印请求来源 ---
        printf("\n[Server:%d] >>> 收到 Search 请求: %s\n", my_port, request->query().c_str());
        fflush(stdout); 

        response->add_results("Response from port " + std::to_string(my_port));
        if (done) done->Run();
    }

    void GetDetail(google::protobuf::RpcController* controller,
                   const knight::rpc::test::DetailRequest* request,
                   knight::rpc::test::DetailResponse* response,
                   google::protobuf::Closure* done) override {
        
        printf("[Server:%d] >>> 收到 GetDetail, ID: %u\n", my_port, request->id());
        fflush(stdout);

        response->set_title("Result from " + std::to_string(my_port));
        response->set_content("Success");
        if (done) done->Run();
    }

    void GetStatus(google::protobuf::RpcController* controller,
                   const knight::rpc::test::Empty* request,
                   knight::rpc::test::StatusResponse* response,
                   google::protobuf::Closure* done) override {
        
        printf("[Server:%d] >>> 收到 GetStatus 心跳检查\n", my_port);
        fflush(stdout);

        response->set_status("OK");
        response->set_load(20);
        if (done) done->Run();
    }

    // 用来区分是谁在打印
    void set_port(uint16_t p) { my_port = p; }

private:
    uint16_t my_port = 0;
};

int main(int argc, char* argv[]) {
    uint16_t port = 9000;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    rpcserver server("http://127.0.0.1:2379");

    SearchServiceImpl search_service;
    search_service.set_port(port); // 让 Service 知道自己在哪跑
    server.register_service(&search_service); 

    printf("========================================\n");
    printf("  RPC Server 正在启动 | 端口: %d\n", port);
    printf("========================================\n");
    fflush(stdout);

    server.run("127.0.0.1", port);

    return 0;
}