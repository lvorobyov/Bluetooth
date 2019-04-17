#include <iostream>
#include <bth/server_socket.h>
#include <csignal>

using namespace std;
using namespace csoi::bth;

// {D3133043-6F35-4FA2-9BC4-D3593430B102}
static const GUID SERVICE_ID =
{ 0xd3133043, 0x6f35, 0x4fa2, { 0x9b, 0xc4, 0xd3, 0x59, 0x34, 0x30, 0xb1, 0x2 } };


int main() {
    int status = EXIT_SUCCESS;
    static volatile sig_atomic_t active = true;
    try {
        server_socket bth_socket(L"Text service via Bluetooth", &SERVICE_ID);
        signal(SIGINT, [](int sig) { active = false; });
        bth_socket.set_blocking(false);
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(bth_socket.raw_socket(), &fds);
        TIMEVAL timeout{0,500};
        do {
            int s = select(0, &fds, nullptr, nullptr, &timeout);
            if (s == SOCKET_ERROR)
                throw logic_error("select error");
            if (s == 0)
                continue;
            client_socket client = bth_socket.accept();
            char buffer[512];
            client.recv(buffer, 512, 0);
            cout << "C: " << buffer << endl;
            client.close();
        } while (active);
    } catch (exception const& ex) {
        cerr << ex.what() << endl;
        status = EXIT_FAILURE;
    }
    return status;
}