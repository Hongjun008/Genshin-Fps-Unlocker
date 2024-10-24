#ifndef __PFRAMEWORK_H__
#define __PFRAMEWORK_H__

#include <QApplication>
#include <windows.h>
#include <psapi.h>
#include <dwmapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "dwmapi.lib")

namespace Ui
{
    class MainWindow;
}
typedef Ui::MainWindow MainWindowUI;
template <typename, typename>
class QHash;
class QString;
class QPushButton;
class QWidget;
class QPixmap;

#define DEFAULT_FPS 120
#define Genshin "Genshin"
#define LMAXPATH (unsigned long)512
#define szPATTERN1
#define UPPER_LIMIT 800

namespace framework
{
    struct MemoryInfo
    {
        quint32 sectionVS;
        quintptr remoteVA;
        quintptr remoteBA;
        LPVOID buffer;
    };

    void Log(QString);
    QString LastError(DWORD code);

    bool ReadConfig(const char *section, const char *key, QString &value);
    bool ReadConfig(const char *section, const char *key, int &value);
    bool ReadConfigBoolean(const char *section, const char *key);

    bool WriteConfig(const char *section, const char *key, QString value);
    bool WriteConfig(const char *section, const char *key, int value);
    bool WriteConfigBoolean(const char *section, const char *key, bool value);

    BOOL GetModule(HANDLE GameHandle, QString ModuleName, PMODULEENTRY32 pEntry, DWORD pid);
    BOOL GetSectionInfo(LPVOID PE_buffer, LPCSTR SectionInfo, uint32_t *Sec_Vsize, uintptr_t *Sec_Remote_RVA, uintptr_t Remote_BaseAddr);
    DWORD GetPID(QString ProcessName);

    uintptr_t PatternScan_Region(uintptr_t startAddress, size_t regionSize, const char *signature);

    DWORD64 InjectPatch(LPVOID text_buffer, DWORD text_size, DWORD64 _text_baseaddr, uint64_t _ptr_fps, HANDLE Tar_handle, int &fps_target);

}
#endif