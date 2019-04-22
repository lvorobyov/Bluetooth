//
// Created by Lev on 22.04.2019.
//

#include <windows.h>
#include <cstdio>
#include <fcntl.h>
#include <tchar.h>

#ifdef _WIN32
#define mbstowcs(dst,src,len) \
    MultiByteToWideChar(CP_UTF8, 0, src, 8, dst, len)
#endif

static const int len = 8;

int main() {
    _setmode(_fileno(stdout), _O_U8TEXT);
    char buf[len];
    const char* str = u8"абв\xe2\x88\x85 где";
    memcpy(buf,str,len);
    int c = mbstowcs(nullptr, buf, 0);
    _tprintf(_T("%d\n"),c);
    auto *wcs = new wchar_t[c+1];
    c = mbstowcs(wcs, buf, c);
    wcs[c] = _T('\0');
    _tprintf(_T("%d\n%ls\n%04X\n"), c, wcs, wcs[c-1]);
    if (wcs[c-1] == 0xFFFD)
        _tprintf(_T("ФФТ"));
    delete [] wcs;
}