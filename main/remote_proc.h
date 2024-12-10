#ifndef REMOTE_PROC_H
#define REMOTE_PROC_H

#include <functional>
#include <string>
#include <cstdint>
#include <atomic>

constexpr int RX_BUF_SIZE = 256;

class RemoteProc
{
public:
    RemoteProc(int port);
    ~RemoteProc();
    bool SendBytes(const std::string& bytes, uint8_t ip[4], uint16_t port);
    void StartListening();
    void StopListening();
    std::string ProcessMessage(const std::string& message); // Process the message and return the response
private:
    char m_rxBuff[RX_BUF_SIZE];
    int m_sock;
    std::atomic<bool> m_running{false};
};

#endif // REMOTE_PROC_H
