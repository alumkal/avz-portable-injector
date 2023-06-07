#include <cstring>
#include <filesystem>
#include <windows.h>
#include <tlhelp32.h>
#pragma comment(lib, "User32.lib")

namespace fs = std::filesystem;

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
        (LPTHREAD_START_ROUTINE)LoadLibraryA,
        dll_name_remote,
        0,
        NULL
    );
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    VirtualFreeEx(process, dll_name_remote, 0, MEM_RELEASE);
}

bool end_with_uuid(const std::string& str) {
    if(str.length() < 36)
        return false;
    std::string uuid_part = str.substr(str.size() - 36, 36);
    return uuid_part.find_first_not_of("0123456789abcdef-") == std::string::npos;
}

void clean_tmp_dll() {
    for(auto dir : fs::directory_iterator(fs::temp_directory_path()))
        if(end_with_uuid(dir.path().string())) {
            bool is_appacker_dir =
                fs::exists(dir.path() / "ProgressBarSplash.exe") ||
                fs::exists(dir.path() / "target.dll");
            if(!is_appacker_dir)
                continue;
            std::error_code _;
            fs::remove_all(dir.path(), _);
        }
}

int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    char dll_name[1024];
    int len = GetModuleFileNameA(NULL, dll_name, sizeof(dll_name));
    strcpy(strrchr(dll_name, '\\') + 1, "target.dll");
    if(!fs::is_regular_file(dll_name)) {
        MessageBoxW(NULL, L"DLL 不存在", L"Error", MB_ICONERROR);
        return 1;
    }
    HANDLE process = find_process();
    free_dll(process, dll_name);
    load_dll(process, dll_name);
    CloseHandle(process);
    clean_tmp_dll();
    return 0;
}
