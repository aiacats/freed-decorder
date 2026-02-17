#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>

class WinsockInit {
public:
    WinsockInit();
    ~WinsockInit();
    bool ok() const { return m_ok; }

    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;

private:
    bool m_ok = false;
};
