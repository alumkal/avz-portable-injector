#include "_process.h"
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#include <direct.h>
#include <iostream>
#include <string>

Process::Process()
{
    hwnd = nullptr;
    pid = 0;
    handle = nullptr;
}

Process::~Process()
{
    if (IsValid())
        CloseHandle(handle);
}

bool Process::OpenByWindow(const wchar_t* class_name, const wchar_t* window_name)
{
    if (IsValid())
        CloseHandle(handle);
    hwnd = FindWindowW(class_name, window_name);

    while (hwnd == nullptr) {
        MessageBoxW(NULL, L"���Ƿ�δ����Ϸ? (ע�������Ӣ��ԭ�棬Steam ��Ҳ�ǲ����Եģ�)", L"Warning", MB_ICONWARNING);
        hwnd = FindWindowW(class_name, window_name);
    }

    GetWindowThreadProcessId(hwnd, &pid);
    if (pid != 0) {
        handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    }

    if (ReadMemory<uint32_t>(0x4140c5) != 0x0019b337) {
        MessageBoxW(NULL, L"��ʹ�õ���Ϸ�汾����Ӣ��ԭ�棬�뵽���ذ�װ������������ AvZ ��֧�ֵ�Ӣ��ԭ��", L"Error", MB_ICONERROR);
        return false;
    }

    auto address = ReadMemory<uintptr_t>(0x6a9ec0);
    auto game_ui = ReadMemory<int>(address + 0x7fc);
    while (game_ui == 2 || game_ui == 3) {
        MessageBoxW(NULL, L"��⵽��Ϸ������ѡ����ս�����棬������Ϊ���ܻᵼ��ע��ʧ�ܣ�������Ϸ���������ע��", L"Warning", MB_ICONWARNING);
        game_ui = ReadMemory<int>(address + 0x7fc);
    }

    return hwnd != nullptr;
}

void Process::ManageDLL()
{
    wchar_t szBuf[512];
    GetModuleFileNameW(NULL, szBuf, sizeof(szBuf));

    std::wstring dll_path = szBuf;
    dll_path = dll_path.substr(0, dll_path.size() - 4) + L".dll";

    if (!InjectDLL(dll_path.c_str())) {
        MessageBoxW(NULL, L"DLL ע��ʧ�ܣ��������Ϊ�Ƿ�ɱ���������", L"Error", MB_ICONERROR);
    }
}

DWORD Process::InjectDLL(PCWSTR pszLibFile)
{
    // Calculate the number of bytes needed for the DLL's pathname
    DWORD dwSize = (lstrlenW(pszLibFile) + 1) * sizeof(wchar_t);
    if (handle == NULL) {
        wprintf(L"[-] Error: Could not open process for PID (%d).\n", pid);
        return FALSE;
    }

    // Allocate space in the remote process for the pathname
    LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(handle, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
    if (pszLibFileRemote == NULL) {
        wprintf(L"[-] Error: Could not allocate memory inside PID (%d).\n", pid);
        return FALSE;
    }

    // Copy the DLL's pathname to the remote process address space
    DWORD n = WriteProcessMemory(handle, pszLibFileRemote, (PVOID)pszLibFile, dwSize, NULL);
    if (n == 0) {
        wprintf(L"[-] Error: Could not write any bytes into the PID [%d] address space.\n", pid);
        return FALSE;
    }

    // Get the real address of LoadLibraryW in Kernel32.dll
    LPTHREAD_START_ROUTINE pfnThreadRtn = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
    if (pfnThreadRtn == NULL) {
        wprintf(L"[-] Error: Could not find LoadLibraryA function inside kernel32.dll library.\n");
        return FALSE;
    }

    // Create a remote thread that calls LoadLibraryW(DLLPathname)
    HANDLE hThread = CreateRemoteThread(handle, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);
    if (hThread == NULL) {
        wprintf(L"[-] Error: Could not create the Remote Thread.\n");
        return FALSE;
    }

    // Wait for the remote thread to terminate
    WaitForSingleObject(hThread, INFINITE);

    // Free the remote memory that contained the DLL's pathname and close Handles
    if (pszLibFileRemote != NULL)
        VirtualFreeEx(handle, pszLibFileRemote, 0, MEM_RELEASE);

    if (hThread != NULL)
        CloseHandle(hThread);

    return TRUE;
}

void Process::Write(uintptr_t addr, size_t len, uint8_t* data)
{
    WriteProcessMemory(handle, (void*)addr, data, len, nullptr);
}

bool Process::IsValid()
{
    if (handle == nullptr)
        return false;

    DWORD exit_code;
    GetExitCodeProcess(handle, &exit_code);
    bool valid = (exit_code == STILL_ACTIVE);

#ifdef _DEBUG
    if (!valid)
        std::cout << "Not Valid" << std::endl;
#endif

    return valid;
}
