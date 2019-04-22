#include <cstdio>
#include <bth/server_socket.h>
#include <net/sockets_set.h>
#include <csignal>
#include <mingw.thread.h>
#include <mingw.mutex.h>
#include <mingw.condition_variable.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fcntl.h>
#include <sstream>

using namespace std;
using namespace csoi::bth;
using csoi::net::sockets_set;

// {D3133043-6F35-4FA2-9BC4-D3593430B102}
static const GUID SERVICE_ID =
{ 0xd3133043, 0x6f35, 0x4fa2, { 0x9b, 0xc4, 0xd3, 0x59, 0x34, 0x30, 0xb1, 0x2 } };

#ifdef _WIN32
#define mbstowcs_s(ret,dst,sz,src,len) \
    ret = MultiByteToWideChar(CP_UTF8, 0, src, len, dst, sz)
#endif

int main() {
    int status = EXIT_SUCCESS;
    static volatile sig_atomic_t active = true;
    signal(SIGINT, [](int sig) { active = false; });
    static const int max_len = 80;
    _setmode(_fileno(stdout), _O_U8TEXT);
    spdlog::create<spdlog::sinks::stderr_sink_st>("stderr_log");
    spdlog::create<spdlog::sinks::basic_file_sink_st>("app_log", "app.log");
    spdlog::set_default_logger(spdlog::get("app_log"));
    vector<TCHAR> message;
    message.reserve(max_len);
    try {
        server_socket bth_socket(L"Text service via Bluetooth", &SERVICE_ID);
        signal(SIGINT, [](int sig) { active = false; });
        bth_socket.set_blocking(false);
        sockets_set socks;
        TIMEVAL timeout{0,15000};
        do {
            socks.set(bth_socket);
            int s = select(0, &socks, nullptr, nullptr, &timeout);
            if (s == SOCKET_ERROR)
                throw logic_error("select error");
            if (s == 0)
                continue;
            socks.clear();
            client_socket client = bth_socket.accept();
            spdlog::info("connected {:12x}", client.get_address().btAddr);
            message.clear();
            char buffer[max_len];
            int len, c, bias = 0;
            while ((len = bias + client.recv(buffer + bias, max_len - bias, 0)) > 0) {
                mbstowcs_s(c, nullptr, 0, buffer, len);
                int sz = message.size();
                message.resize(sz + c);
                mbstowcs_s(c, &message[sz], c, buffer, len);
                bias = 0;
                if (message.back() == 0xFFFD) {
                    message.pop_back();
                    int i = len;
                    char ch;
                    do {
                        ch = buffer[--i];
                        if (ch > 0)
                            break;
                        bias ++;
                    } while (ch < -64);
                    memcpy(buffer, buffer + len - bias, bias);
                }
            }
            message.push_back(L'\0');
            _tprintf(_T("%ls\n"), message.data());
        } while (active);
        _tprintf(_T("Bye!\n"));
    } catch (exception const& ex) {
        spdlog::error(ex.what());
        status = EXIT_FAILURE;
    }
    return status;
}