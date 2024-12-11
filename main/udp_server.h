#ifndef REMOTE_PROC_H
#define REMOTE_PROC_H

#include <functional>
#include <string>
#include <cstdint>
#include <atomic>
#include <memory>

#include "idata_proc.h"

constexpr int RX_BUF_SIZE = 256;

class UdpServer {
public:
    UdpServer(int port);
    ~UdpServer();
    bool SendBytes(const std::string& bytes, uint8_t ip[4], uint16_t port);
    void StartListening();
    void StopListening();
private:
    char m_rxBuff[RX_BUF_SIZE];
    int m_sock;
    std::atomic<bool> m_running{false};
    std::shared_ptr<IDataProc> m_dataProc = nullptr;
};

#endif // REMOTE_PROC_H
