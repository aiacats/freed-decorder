#include "network_utils.h"

WinsockInit::WinsockInit() {
    WSADATA wsa_data;
    m_ok = (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0);
}

WinsockInit::~WinsockInit() {
    if (m_ok) {
        WSACleanup();
    }
}
