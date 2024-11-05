#include "framework.h"

#include <QSettings>
#include <QString>
#include <QWidget>

QSettings *INIHelper()
{
    QSettings *inst = new QSettings("fpsunlocker.cfg", QSettings::IniFormat);
    return inst;
}

namespace framework
{
    const BYTE shellCode_Patch[] =
        {
            0x00, 0x00, 0x00, 0x00,                         // uint32_t unlocker_pid              _shellcode_genshin[0]
            0xCC, 0x54, 0xD0, 0x66,                         // uint32_t timestamp                 _shellcode_genshin[4]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t unlocker_FpsValue_addr    _shellcode_genshin[8]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t API_OpenProcess           _shellcode_genshin[0x10]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t API_ReadProcessmem        _shellcode_genshin[0x16]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t API_Sleep                 _shellcode_genshin[0x20]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t API_MessageBoxA           _shellcode_genshin[0x28]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t API_CloseHandle           _shellcode_genshin[0x30]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t Ptr_il2cpp_fps            _shellcode_genshin[0x38]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // uint64_t Ptr_Engine_fps            _shellcode_genshin[0x40]
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FREE
            // int3
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            // int3
            0x48, 0x83, 0xEC, 0x38,                         // sub rsp,0x38                       _shellcode_genshin[0x60] _sync_thread
            0x8B, 0x05, 0x96, 0xFF, 0xFF, 0xFF,             // mov eax,dword[unlocker_pid]
            0x85, 0xC0,                                     // test eax, eax
            0x74, 0x66,                                     // jz return
            0x41, 0x89, 0xC0,                               // mov r8d, eax
            0x33, 0xD2,                                     // xor edx, edx
            0xB9, 0xFF, 0xFF, 0x1F, 0x00,                   // mov ecx,1FFFFF
            0xFF, 0x15, 0x92, 0xFF, 0xFF, 0xFF,             // call [API_OpenProcess]
            0x85, 0xC0,                                     // test eax, eax
            0x74, 0x52,                                     // jz return
            0x89, 0xC6,                                     // mov esi, eax
            0x66, 0x48, 0x8B, 0x3D, 0x7C, 0xFF, 0xFF, 0xFF, // mov rdi, qword[unlocker_FpsValue_addr]
            0x48, 0x31, 0xDB,                               // xor rbx, rbx
            0x90,                                           // nop
            0x66, 0x4C, 0x8D, 0x05, 0x08, 0x01, 0x00, 0x00, // lea r8, qword:[Readmem_buffer]        //Read_tar_fps
            0x41, 0xB9, 0x04, 0x00, 0x00, 0x00,             // mov r9d, 0x4
            0x48, 0x89, 0x5C, 0x24, 0x20,                   // mov qword ptr ss:[rsp+20],rbx
            0x48, 0x89, 0xFA,                               // mov rdx, rdi
            0x89, 0xF1,                                     // mov ecx, esi
            0xFF, 0x15, 0x6A, 0xFF, 0xFF, 0xFF,             // call [API_ReadProcessmem]
            0x85, 0xC0,                                     // test eax, eax
            0x75, 0x0D,                                     // jnz continue
            0x48, 0xE8, 0x88, 0x00, 0x00, 0x00,             // call Show Errormsg and CloseHandle
            0xC6, 0x05, 0x12, 0x00, 0x00, 0x00, 0xED,       // mov byte ptr ds:[rip + 0x14], 0xEB  //控制循环范围
            // continue
            0xB9, 0xF4, 0x01, 0x00, 0x00,       // mov ecx, 0x1F4        (500ms)
            0xFF, 0x15, 0x56, 0xFF, 0xFF, 0xFF, // call [API_Sleep]
            0x48, 0xE8, 0x10, 0x00, 0x00, 0x00, // call Sync_auto
            0xEB, 0xBE,                         // jmp Read_tar_fps
            // int3
            0xCC, 0xCC,
            // return
            0x48, 0x83, 0xC4, 0x38, // add rsp,0x38
            0xC3,                   // ret
            // int3
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            // int3
            0x66, 0x48, 0x8B, 0x05, 0x50, 0xFF, 0xFF, 0xFF, // mov  rax, qword ptr ds:[il2cpp_fps]
            0x85, 0xC0,                                     // test eax, eax
            0x75, 0x0A,                                     // jnz read_game_set
            // Sync_unlocker
            0x66, 0x90,                         // nop
            0x8B, 0x0D, 0xAC, 0x00, 0x00, 0x00, // mov ecx, dword ptr ds:[Readmem_buffer]
            0xEB, 0x1A,                         // jmp Write
            // read_game_set
            0x8B, 0x08,                         // mov ecx, qword ptr ss:[rax]
            0x83, 0xF9, 0x1E,                   // cmp ecx, 0x1E
            0x74, 0x0D,                         // je set 60
            0x83, 0xF9, 0x2D,                   // cmp ecx, 0x2D
            0x74, 0xEC,                         // je Sync_unlocker
            0x2E, 0xB9, 0xE8, 0x03, 0x00, 0x00, // mov ecx, 0x3E8
            0xEB, 0x06,                         // jmp Write
            0x2E, 0xB9, 0x3C, 0x00, 0x00, 0x00, // mov ecx, 0x3C
            // Write
            0x66, 0x48, 0x8B, 0x05, 0x28, 0xFF, 0xFF, 0xFF, // mov rax, qword ptr ds:[engine_fps]
            0x89, 0x08,                                     // mov dword ptr ds:[rax], ecx
            0xC3,                                           // ret
            // int3
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            // int3
            0x48, 0x83, 0xEC, 0x28,                   // sub rsp, 0x28                        //Show Errormsg and closehandle
            0x31, 0xC9,                               // xor ecx, ecx
            0x48, 0x8D, 0x15, 0x33, 0x00, 0x00, 0x00, // lea rdx, qword:["Sync failed!"]
            0x4C, 0x8D, 0x05, 0x3C, 0x00, 0x00, 0x00, // lea r8, qword:["Error"]
            0x41, 0xB9, 0x10, 0x00, 0x00, 0x00,       // mov r9d, 0x10
            0xFF, 0x15, 0xC8, 0xFE, 0xFF, 0xFF,       // call [API_MessageBoxA]
            0x89, 0xF1,                               // mov ecx, esi
            0xFF, 0x15, 0xC8, 0xFE, 0xFF, 0xFF,       // call [API_CloseHandle]
            0x48, 0x83, 0xC4, 0x28,                   // add rsp, 0x28
            0xC3,                                     // ret
            // int3
            0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
            'S', 'y', 'n', 'c', ' ', 'f', 'a', 'i', 'l', 'e', 'd', '!', 0x00, 0x00, 0x00, 0x00,
            'E', 'r', 'r', 'o', 'r', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, // uint32_t Readmem_buffer
            0x00, 0x00, 0x00, 0x00, // FREE
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    void Log(QString)
    {

    }

    QString LastError(DWORD code)
    {
        char* buf = nullptr;
        ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 0, NULL);
        QString ret{buf};
        LocalFree(buf);
        return ret;

    }

    bool ReadConfig(const char *is, const char *ik, QString &value)
    {
        auto ini = INIHelper();
        value = ini->value(QString::asprintf("/%s/%s", is, ik), "NO_VALUE").toString();
        delete ini;
        return value != "NO_VALUE";
    }

    bool ReadConfig(const char *is, const char *ik, int &value)
    {
        auto ini = INIHelper();
        value = ini->value(QString::asprintf("/%s/%s", is, ik), -1).toInt();
        delete ini;
        return value != -1;
    }

    bool ReadConfigBoolean(const char *is, const char *ik)
    {
        auto ini = INIHelper();
        bool ret = ini->value(QString::asprintf("/%s/%s", is, ik), false).toBool();
        delete ini;
        return ret;
    }

    bool WriteConfig(const char *is, const char *ik, QString value)
    {
        auto ini = INIHelper();
        bool ret = false;
        if (ini->isWritable())
        {
            ini->setValue(QString::asprintf("/%s/%s", is, ik), value);
            ret = true;
        }
        delete ini;
        return ret;
    }

    bool WriteConfig(const char *is, const char *ik, int value)
    {
        return WriteConfig(is, ik, QString::number(value));
    }

    bool WriteConfigBoolean(const char *is, const char *ik, bool value)
    {
        return WriteConfig(is, ik, value ? 1 : 0);
    }

    DWORD GetPID(QString ProcessName)
    {
        DWORD pid = 0;
        PROCESSENTRY32 pe32{};
        pe32.dwSize = sizeof(pe32);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        for (Process32First(snap, &pe32); Process32Next(snap, &pe32);)
        {
            if (pe32.szExeFile == ProcessName.toStdWString())
            {
                pid = pe32.th32ProcessID;
                break;
            }
        }
        CloseHandle(snap);
        return pid;
    }

    BOOL GetModule(HANDLE GameHandle, QString ModuleName, PMODULEENTRY32 pEntry, DWORD pid)
    {
        if (!pEntry)
            return false;

        MODULEENTRY32 mod32{};
        mod32.dwSize = sizeof(mod32);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        bool temp = Module32FirstW(snap, &mod32);
        if (temp)
        {
            do
            {
                if (mod32.th32ProcessID != pid)
                    continue;
                if (mod32.szModule == ModuleName.toStdWString())
                {
                    *pEntry = mod32;
                    CloseHandle(snap);
                    return true;
                }

            } while (Module32NextW(snap, &mod32));
        }
        CloseHandle(snap);
        return false;
    }

    BOOL GetSectionInfo(LPVOID PE_buffer, LPCSTR SectionName, uint32_t *Sec_Vsize, uintptr_t *Sec_Remote_RVA, uintptr_t Remote_BaseAddr)
    {
        if ((!PE_buffer) || (!SectionName) || (!Sec_Vsize) || (!Sec_Remote_RVA) || (!Remote_BaseAddr))
            return 0;
        uint64_t tar_sec = *(uint64_t *)SectionName;                              // max 8 byte
        uintptr_t WinPEfileVA = *(uintptr_t *)(&PE_buffer) + 0x3c;                // dos_header
        uintptr_t PEfptr = *(uintptr_t *)(&PE_buffer) + *(uint32_t *)WinPEfileVA;
        _IMAGE_NT_HEADERS64 _FilePE_Nt_header = *(_IMAGE_NT_HEADERS64 *)PEfptr;
        _IMAGE_SECTION_HEADER _sec_temp{};
        if (_FilePE_Nt_header.Signature == 0x00004550)
        {
            DWORD sec_num = _FilePE_Nt_header.FileHeader.NumberOfSections;
            sec_num++;
            DWORD num = sec_num;
            DWORD target_sec_VA_start = 0;
            do
            {
                _sec_temp = *(_IMAGE_SECTION_HEADER *)(PEfptr + 264 + (40 * (static_cast<unsigned long long>(sec_num) - num)));

                if (*(uint64_t *)(_sec_temp.Name) == tar_sec)
                {
                    target_sec_VA_start = _sec_temp.VirtualAddress;
                    *Sec_Vsize = _sec_temp.Misc.VirtualSize;
                    *Sec_Remote_RVA = Remote_BaseAddr + target_sec_VA_start;
                    return 1;
                }
                num--;

            } while (num);

            return 0;
        }
        return 0;
    }

    uintptr_t PatternScan_Region(uintptr_t startAddress, size_t regionSize, const char *signature)
    {
        auto pattern_to_byte = [](const char *pattern)
        {
            std::vector<int> bytes;
            const char *start = pattern;
            const char *end = pattern + strlen(pattern);

            for (const char *current = start; current < end; ++current)
            {
                if (*current == '?')
                {
                    ++current;
                    if (*current == '?')
                        ++current;
                    bytes.push_back(-1);
                }
                else
                {
                    bytes.push_back(strtoul(current, const_cast<char **>(&current), 16));
                }
            }
            return bytes;
        };

        std::vector<int> patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t *>(startAddress);

        for (size_t i = 0; i < regionSize - patternBytes.size(); ++i)
        {
            bool found = true;
            for (size_t j = 0; j < patternBytes.size(); ++j)
            {
                if (scanBytes[i + j] != patternBytes[j] && patternBytes[j] != -1)
                {
                    found = false;
                    break;
                }
            }
            if (found)
            {
                return (uintptr_t)&scanBytes[i];
            }
        }
        return 0;
    }

    DWORD64 InjectPatch(LPVOID text_buffer, DWORD text_size, DWORD64 _text_baseaddr, uint64_t _ptr_fps, HANDLE Tar_handle, int &fps_target)
    {
        if ((!text_buffer) || (!text_size) || (!_text_baseaddr) || (!_ptr_fps) || (!Tar_handle))
            return 0;
        DWORD64 address = 0;
        DWORD64 Module_TarSec_RVA = (DWORD64)text_buffer;
        DWORD Module_TarSec_Size = text_size;
        uintptr_t _shellcode_buffer = (uintptr_t)VirtualAlloc(0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (_shellcode_buffer == 0)
        {
            Log("[ERR]Alloc shellcode memory buffer failed");
            return 0;
        }
        memcpy((void *)_shellcode_buffer, &shellCode_Patch, sizeof(shellCode_Patch));
        DWORD64 _Ptr_il2cpp_setting_fps = 0;
        address = PatternScan_Region(Module_TarSec_RVA, Module_TarSec_Size, "48 89 F1 E8 ?? ?? ?? ?? 8B 3D ?? ?? ?? ?? 48 8B 0D");
        if (address)
        {
            uintptr_t rip = address;
            rip += 10;
            if ((*(uint32_t *)rip) >> 31)
                rip -= uint32_t(~((*(uint32_t *)rip) + 3));
            else
                rip += uint32_t(*(int32_t *)rip) + 4;

            _Ptr_il2cpp_setting_fps = rip - Module_TarSec_RVA + _text_baseaddr;
        }
        uint64_t _Addr_OpenProcess = (uint64_t)(&OpenProcess);
        uint64_t _Addr_ReadProcessmem = (uint64_t)(&ReadProcessMemory);
        uint64_t _Addr_Sleep = (uint64_t)(&Sleep);
        uint64_t _Addr_MessageBoxA = (uint64_t)(&MessageBoxA);
        uint64_t _Addr_CloseHandle = (uint64_t)(&CloseHandle);
        *(uint32_t *)_shellcode_buffer = GetCurrentProcessId();           // unlocker PID
        *(uint64_t *)(_shellcode_buffer + 0x8) = (uint64_t)(&fps_target); // unlocker fps ptr
        *(uint64_t *)(_shellcode_buffer + 0x10) = _Addr_OpenProcess;
        *(uint64_t *)(_shellcode_buffer + 0x18) = _Addr_ReadProcessmem;
        *(uint64_t *)(_shellcode_buffer + 0x20) = _Addr_Sleep;
        *(uint64_t *)(_shellcode_buffer + 0x28) = _Addr_MessageBoxA;
        *(uint64_t *)(_shellcode_buffer + 0x30) = _Addr_CloseHandle;
        *(uint64_t *)(_shellcode_buffer + 0x40) = _ptr_fps;
        *(uint32_t *)(_shellcode_buffer + 0x104) = fps_target;
        *(uint32_t *)(_shellcode_buffer + 0x10C) = 60;
        *(uint64_t *)(_shellcode_buffer + 0x38) = _Ptr_il2cpp_setting_fps;

        LPVOID __Tar_proc_buffer = VirtualAllocEx(Tar_handle, (void *)((_text_baseaddr >> 32) << 32), 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (__Tar_proc_buffer)
        {
            if (WriteProcessMemory(Tar_handle, __Tar_proc_buffer, (void *)_shellcode_buffer, sizeof(shellCode_Patch), 0))
            {
                VirtualFree((void *)_shellcode_buffer, 0, MEM_RELEASE);

                HANDLE temp = CreateRemoteThread(Tar_handle, 0, 0, (LPTHREAD_START_ROUTINE)((uint64_t)__Tar_proc_buffer + 0x60), 0, 0, 0);
                if (temp)
                    CloseHandle(temp);
                else
                {
                    Log("[ERR]Create inGame SyncThread failed");
                    return 0;
                }
                return ((uint64_t)__Tar_proc_buffer + 0x1A0);
            }
            Log("[ERR]Inject shellcode Failed");
            VirtualFree((void *)_shellcode_buffer, 0, MEM_RELEASE);
            return 0;
        }
        else
        {
            Log("[ERR]Alloc memory for shellcode failed");
            return 0;
        }
    }
    void SetFramelessWindow(QWidget *widget)
    {
        MARGINS margins = {1, 1, 1, 1};
        widget->setWindowFlags(widget->windowFlags() | Qt::FramelessWindowHint);
        auto hwnd = reinterpret_cast<HWND>(widget->winId());
        auto style = GetWindowLongW(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION;
        ::SetWindowLongW(hwnd, GWL_STYLE, style);
        ::DwmExtendFrameIntoClientArea(hwnd, &margins);
    }
}