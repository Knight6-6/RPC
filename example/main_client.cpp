#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <atomic>
#include <knight/rpc/rpc_client.hpp>
#include "search.pb.h"

using namespace knight::rpc;

int main() {
    // 1. 初始化
    rpcclient client("127.0.0.1", 8888);
    int thread_num = 20;           // 开启 10 个并发线程
    int req_per_thread = 50000;     // 每个线程跑 2000 次，总共 2万次请求
    std::atomic<int> success_count{0};

    std::cout << ">>> [并发压测启动] 线程数: " << thread_num 
              << " | 总请求量: " << thread_num * req_per_thread << std::endl;

    // 2. 准备线程池
    std::vector<std::thread> workers;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < thread_num; ++i) {
        workers.emplace_back([&client, req_per_thread, &success_count]() {
            // 每个线程必须有独立的 req 和 res，避免 Protobuf 内部竞争
            knight::rpc::test::SearchRequest local_req;
            local_req.set_query("Heavy Load Test");
            knight::rpc::test::SearchResponse local_res;

            for (int j = 0; j < req_per_thread; ++j) {
                local_res.Clear();
                // 调用 RPC
                client.invoke("SearchService", local_req, local_res);
                
                if (local_res.results_size() > 0) {
                    success_count.fetch_add(1, std::memory_order_relaxed);
                }
            }
        });
    }

    // 3. 等待所有战线收工
    for (auto& t : workers) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    // 4. 结果统计
    std::chrono::duration<double, std::milli> total_ms = end - start;
    double total_requests = thread_num * req_per_thread;
    double qps = (total_requests / total_ms.count()) * 1000.0;
    double avg_lat = total_ms.count() / total_requests;

    std::cout << "\n================ 并发性能报告 ================" << std::endl;
    std::cout << ">>> 成功请求数  : " << success_count.load() << std::endl;
    std::cout << ">>> 总吞吐耗时  : " << total_ms.count() << " ms" << std::endl;
    std::cout << ">>> 平均单次延迟 : " << avg_lat << " ms" << std::endl;
    std::cout << ">>> 极限 QPS    : " << (int)qps << std::endl;
    std::cout << "==============================================" << std::endl;

    return 0;
}