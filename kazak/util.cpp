#include <windows.h>
#include "util.h"

std::wstring getExecutablePath() {
    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH)) {
        return std::wstring(path);
    } else {
        return L"";
    }
}

std::wstring getDirectoryOfExecutable() {
    std::wstring path = getExecutablePath();
    size_t pos = path.find_last_of(L"\\/"); 
    if (pos != std::wstring::npos) {
        return path.substr(0, pos);
    }
    return L"";
}
