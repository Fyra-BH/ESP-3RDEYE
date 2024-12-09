#ifndef REMOTE_PROC_H
#define REMOTE_PROC_H

#include <functional>
#include <string>
#include <cstdint>

constexpr int RX_BUF_SIZE = 256;

class RemoteProc
{
public:
    RemoteProc(int port);
    bool SendBytes(const std::string& bytes, uint8_t ip[4], uint16_t port);
    ~RemoteProc();
private:
    char m_rxBuff[RX_BUF_SIZE];
    int m_sock;
};

#endif // REMOTE_PROC_H
