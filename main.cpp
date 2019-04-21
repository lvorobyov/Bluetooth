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

using namespace std;
using namespace csoi::bth;
using csoi::net::sockets_set;

// {D3133043-6F35-4FA2-9BC4-D3593430B102}
static const GUID SERVICE_ID =
{ 0xd3133043, 0x6f35, 0x4fa2, { 0x9b, 0xc4, 0xd3, 0x59, 0x34, 0x30, 0xb1, 0x2 } };


int main() {
    int status = EXIT_SUCCESS;
    static volatile sig_atomic_t active = true;
    signal(SIGINT, [](int sig) { active = false; });
    static const int max_len = 80;
    spdlog::create<spdlog::sinks::stderr_sink_st>("stderr_log");
    spdlog::create<spdlog::sinks::basic_file_sink_st>("app_log", "app.log");
    spdlog::set_default_logger(spdlog::get("app_log"));
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
            TCHAR buffer[max_len];
            while (client.recv((char*)buffer, max_len, 0))
                _tprintf(_T("%ls"), buffer);
            _tprintf(_T("\n"));
        } while (active);
        _tprintf(_T("Bye!\n"));
    } catch (exception const& ex) {
        spdlog::error(ex.what());
        status = EXIT_FAILURE;
    }
    return status;
}