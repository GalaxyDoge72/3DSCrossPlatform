#include "netscan.h"

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cstring>
#include <string>
#include <sstream>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>

using namespace std;

bool scanIPForPort(const char* ipAddr) {
    if (!ipAddr) return false;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags >= 0) fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TARGET_PORT);

    if (inet_pton(AF_INET, ipAddr, &addr.sin_addr) != 1) {
        ::close(sock);
        return false;
    }

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res == 0) {
        ::close(sock);
        return true;
    }

    if (res < 0 && errno != EINPROGRESS) {
        ::close(sock);
        return false;
    }

    // Wait for write enabled flag //
    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sock, &wfds);

    struct timeval tv;
    tv.tv_sec = SCAN_TIMEOUT_SECONDS;
    tv.tv_usec = 0;

    res = select(sock + 1, NULL, &wfds, NULL, &tv);
    if (res <= 0) {
        ::close(sock);
        return false;
    }

    int soError = 0;
    socklen_t len = sizeof(soError);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &soError, &len) < 0) {
        ::close(sock);
        return false;
    }

    ::close(sock);
    return (soError == 0);
}

bool startSubnetScan(char* foundIPBuffer, size_t bufSize) {
    if (!foundIPBuffer || bufSize == 0) return false;

    foundIPBuffer[0] = '\0';

    queue<string> tasks;
    for (int i = IP_RANGE_START; i <= IP_RANGE_END; i++) {
        ostringstream ss;
        ss << SUBNET_BASE << i;
        tasks.push(ss.str());
    }

    mutex queueMutex;
    condition_variable cv;
    atomic<bool> found(false);
    mutex resultMutex;

    auto worker = [&](void) {
        while (!found.load()) {
            string ip;
            {
                unique_lock<mutex> lk(queueMutex);
                if (tasks.empty()) return;
                ip = move(tasks.front());
                tasks.pop();
            }
            bool open = scanIPForPort(ip.c_str());
            if (open) {
                bool expected = false;
                if (found.compare_exchange_strong(expected, true)) {
                    lock_guard<mutex> g(resultMutex);
                    size_t copyLen = min(bufSize - 1, ip.size());
                    memcpy(foundIPBuffer, ip.c_str(), copyLen);
                    foundIPBuffer[copyLen] = '\0';
                }
                return;
            }
        }
    };
    unsigned int threadCount = MAX_THREADS;
    unsigned int totalTasks = (IP_RANGE_END - IP_RANGE_START + 1);
    if (threadCount == 0) threadCount = 1;
    if (threadCount > totalTasks) threadCount = totalTasks;
    vector<thread> threads;
    threads.reserve(threadCount);
    for (unsigned int t = 0; t < threadCount; ++t) threads.emplace_back(worker);
    for (auto &th : threads) {
        if (th.joinable()) th.join();
    }
    return found.load();
}