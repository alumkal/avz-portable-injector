#include <cstring>
#include <windows.h>
#include <tlhelp32.h>
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "User32.lib")

template <typename T>
T read_memory(HANDLE process, auto addr) {
    T ret;
    ReadProcessMemory(process, (void*)addr, &ret, sizeof(T), NULL);
    return ret;
}

HANDLE find_process() {
    HWND hwnd = FindWindowA("MainWindow", "Plants vs. Zombies");
    while(hwnd == nullptr) {
        int resp = MessageBoxW(
            NULL,
            L"您是否未打开游戏？",
            L"注入失败",
            MB_RETRYCANCEL | MB_ICONWARNING
        );
        if(resp != IDRETRY)
            ExitProcess(0);
        hwnd = FindWindowA("MainWindow", "Plants vs. Zombies");
    }
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    auto base = read_memory<char*>(process, 0x6a9ec0);
    auto game_ui = read_memory<int>(process, base + 0x7fc);
    while(game_ui == 2 || game_ui == 3) {
        int resp = MessageBoxW(
            NULL,
            L"检测到游戏窗口在选卡或战斗界面，这种行为可能会导致注入失败，请在游戏主界面进行注入",
            L"注入失败",
            MB_RETRYCANCEL | MB_ICONWARNING
        );
        if(resp != IDRETRY)
            ExitProcess(0);
        game_ui = read_memory<int>(process, base + 0x7fc);
    }
    return process;
}

void free_dll(HANDLE process, const char* dll_name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetProcessId(process));
    MODULEENTRY32 me = {sizeof(me)};
    bool found = false;
    for(bool has_next = Module32First(snapshot, &me); has_next; has_next = Module32Next(snapshot, &me))
        if(strcmp(me.szModule, dll_name) == 0 || strcmp(me.szExePath, dll_name) == 0) {
            found = true;
            break;
        }
    CloseHandle(snapshot);
    if(!found)
        return;
    HANDLE thread = CreateRemoteThread(
        process,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)FreeLibrary,
        me.modBaseAddr,
        0,
        NULL
    );
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
}

void load_dll(HANDLE process, const char* dll_name) {
    int dll_name_len = strlen(dll_name);
    void* dll_name_remote = VirtualAllocEx(process, NULL, dll_name_len + 1, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(process, dll_name_remote, dll_name, dll_name_len + 1, NULL);
    HANDLE thread = CreateRemoteThread(
        process,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)LoadLibraryW,
        dll_name_remote,
        0,
        NULL
    );
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    VirtualFreeEx(process, dll_name_remote, 0, MEM_RELEASE);
}

bool choose_file(char* buf, size_t buf_size) {
    OPENFILENAME ofn = {sizeof(ofn)};
    ofn.lpstrFilter = "libavz.dll\0*.dll\0";
    ofn.lpstrFile = buf;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = buf_size;
    ofn.Flags = OFN_FILEMUSTEXIST;
    return GetOpenFileNameA(&ofn);
}

bool file_exists(const char* path) {
    DWORD attr = GetFileAttributesA(path);
    if(attr == INVALID_FILE_ATTRIBUTES)
        return false;
    return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    char dll_name[1024];
    if(file_exists(cmdline))
        GetFullPathNameA(cmdline, sizeof(dll_name), dll_name, NULL);
    else if(!choose_file(dll_name, sizeof(dll_name)))
        return 0;
    HANDLE process = find_process();
    free_dll(process, dll_name);
    load_dll(process, dll_name);
    CloseHandle(process);
    return 0;
}
