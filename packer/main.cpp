#include <filesystem>
#include <fstream>
#include <windows.h>
#pragma comment(lib, "Comdlg32.lib")

using namespace std::literals;
namespace fs = std::filesystem;

void extract_resource(const char* res_name, fs::path path) {
    HRSRC res_info = FindResourceA(NULL, res_name, RT_RCDATA);
    if(res_info == NULL)
        throw std::runtime_error("Could not find resource "s + res_name);
    HGLOBAL res_handle = LoadResource(NULL, res_info);
    char* res = (char*)LockResource(res_handle);
    size_t sz = SizeofResource(NULL, res_info);
    std::ofstream out(path, std::ios::binary);
    out.write(res, sz).flush();
}

void init() {
    fs::path cwd = fs::temp_directory_path() / "AvZPacker";
    fs::remove_all(cwd);
    fs::create_directory(cwd);
    fs::current_path(cwd);
    extract_resource("APPACKER_EXE", "appacker.exe");
    extract_resource("AVZ_ICO", "avz.ico");
    fs::create_directory("app");
    extract_resource("INJECTOR_EXE", "app\\injector.exe");
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

int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
    fs::path dll_path(cmdline);
    if(!fs::is_regular_file(dll_path)) {
        char buf[1024];
        if(!choose_file(buf, sizeof(buf)))
            return 0;
        dll_path = buf;
    }
    init();
    fs::copy_file(dll_path, "app\\target.dll");
    fs::path output = dll_path.replace_extension("exe");
    std::string cmd = "Appacker.exe -s app -e injector.exe -i avz.ico -d \"" + output.string() + "\"";
    WinExec(cmd.c_str(), SW_HIDE);
    return 0;
}
