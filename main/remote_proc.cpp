#include "remote_proc.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "esp_log.h"


const char *TAG = "REMOTE_PROC";

RemoteProc::RemoteProc(int port)
{
    int ip_protocol = 0;
    int addr_family = AF_INET;
    struct sockaddr_in6 dest_addr;
    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(port);
    ip_protocol = IPPROTO_IP;
    m_sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (m_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket created");
    int err = bind(m_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
    }
    ESP_LOGI(TAG, "Socket bound, port %d", port);
}

RemoteProc::~RemoteProc()
{
    ESP_LOGE(TAG, "Shutting down socket and restarting...");
    shutdown(m_sock, 0);
    close(m_sock);
}

bool RemoteProc::SendBytes(const std::string& bytes, uint8_t ip[4], uint16_t port)
{
    struct sockaddr_in6 source_addr;
    struct sockaddr_in *source_addr_ip4 = (struct sockaddr_in *)&source_addr;
    source_addr_ip4->sin_addr.s_addr = htonl(ip[0] << 24 | ip[1] << 16 | ip[2] << 8 | ip[3]);
    source_addr_ip4->sin_family = AF_INET;
    source_addr_ip4->sin_port = htons(port);
    int err = sendto(
        m_sock, bytes.data(), bytes.size(), 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        return false;
    }
    return true;
}
