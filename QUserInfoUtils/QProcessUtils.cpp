#include "QProcessUtils.h"

#include <Psapi.h>
#include <strsafe.h>
#include <Tlhelp32.h>
#include <ShellAPI.h>
#include <stdexcept>

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Advapi32.lib")

namespace QProcessInfo
{

    BOOL setPrivilege(HANDLE hProcess, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
    {
        HANDLE hToken;
        if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
        {
            return FALSE;
        }

        LUID luid;
        if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
        {
            return FALSE;
        }

        TOKEN_PRIVILEGES tkp;
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = luid;
        tkp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : FALSE;

        if (!AdjustTokenPrivileges(hToken,
                                   FALSE,
                                   &tkp,
                                   sizeof(TOKEN_PRIVILEGES),
                                   (PTOKEN_PRIVILEGES)NULL,
                                   (PDWORD)NULL))
        {
            return FALSE;
        }

        return TRUE;
    }

    int Compare(const void * Val1, const void * Val2)
    {
        if (*(PDWORD)Val1 == *(PDWORD)Val2)
        {
            return 0;
        }

        return *(PDWORD)Val1 > *(PDWORD)Val2 ? 1 : -1;
    }


    class CpuUsage
    {
    public:
        CpuUsage::CpuUsage(DWORD dwProcessID)
            : m_nCpuUsage(0),
              m_dwLastRun(0),
              m_lRunCount(0),
              m_dwProcessID(dwProcessID),
              m_ullPrevProcNonIdleTime(0),
              m_ullPrevSysNonIdleTime(0)
        {
            HANDLE hProcess = GetCurrentProcess();
            setPrivilege(hProcess, SE_DEBUG_NAME, TRUE);

            m_hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION , TRUE, m_dwProcessID);

            ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
            ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

            ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
            ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
        }

        ULONGLONG getUsageEx()
        {
            ULONGLONG nCpuCopy = m_nCpuUsage;

            if (::InterlockedIncrement(&m_lRunCount) == 1)
            {
                if (!enoughTimePassed())
                {
                    ::InterlockedDecrement(&m_lRunCount);
                    return nCpuCopy;
                }

                ULONGLONG ullSysNonIdleTime = getSystemNonIdleTimes();
                ULONGLONG ullProcNonIdleTime = getProcessNonIdleTimes();

                if (!isFirstRun())
                {
                    ULONGLONG ullTotalSys = ullSysNonIdleTime - m_ullPrevSysNonIdleTime;

                    if (ullTotalSys == 0)
                    {
                        ::InterlockedDecrement(&m_lRunCount);
                        return nCpuCopy;
                    }

                    m_nCpuUsage = ULONGLONG((ullProcNonIdleTime - m_ullPrevProcNonIdleTime) * 100.0 / (ullTotalSys));
                    m_ullPrevSysNonIdleTime = ullSysNonIdleTime;
                    m_ullPrevProcNonIdleTime = ullProcNonIdleTime;
                }

                m_dwLastRun = (ULONGLONG)GetTickCount();
                nCpuCopy = m_nCpuUsage;
            }

            ::InterlockedDecrement(&m_lRunCount);

            return nCpuCopy;
        }

        ULONGLONG getSystemNonIdleTimes()
        {
            FILETIME ftSysIdle, ftSysKernel, ftSysUser;

            if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser))
            {
                return 0;
            }

            return addTimes(ftSysKernel, ftSysUser);
        }

        ULONGLONG getProcessNonIdleTimes()
        {
            FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

            BOOL proceTime = GetProcessTimes(m_hProcess,
                                             &ftProcCreation,
                                             &ftProcExit,
                                             &ftProcKernel,
                                             &ftProcUser);
            if (!proceTime)
            {
                return 0;
            }

            return addTimes(ftProcKernel, ftProcUser);
        }

    private:
        ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB)
        {
            LARGE_INTEGER liA, liB;
            liA.LowPart = ftA.dwLowDateTime;
            liA.HighPart = ftA.dwHighDateTime;

            liB.LowPart = ftB.dwLowDateTime;
            liB.HighPart = ftB.dwHighDateTime;

            return liA.QuadPart - liB.QuadPart;
        }

        ULONGLONG addTimes(const FILETIME& ftA, const FILETIME& ftB)
        {
            LARGE_INTEGER liA, liB;
            liA.LowPart = ftA.dwLowDateTime;
            liA.HighPart = ftA.dwHighDateTime;

            liB.LowPart = ftB.dwLowDateTime;
            liB.HighPart = ftB.dwHighDateTime;

            return liA.QuadPart + liB.QuadPart;
        }

        bool enoughTimePassed()
        {
            const int c_minElapsedMS = 250;//milliseconds

            ULONGLONG dwCurrentTickCount = GetTickCount();
            return (dwCurrentTickCount - m_dwLastRun) > c_minElapsedMS;
        }

        bool isFirstRun() const
        {
            return m_dwLastRun == 0;
        }

        //system total times
        FILETIME m_ftPrevSysKernel;
        FILETIME m_ftPrevSysUser;

        //process times
        FILETIME m_ftPrevProcKernel;
        FILETIME m_ftPrevProcUser;

        ULONGLONG m_ullPrevSysNonIdleTime;//这个变量和后面的便利记录上次获取的非idle的系统cpu时间和进程cpu时间.
        ULONGLONG m_ullPrevProcNonIdleTime;//这个类只绑定一个进程,在构造函数里面初始化进来

        ULONGLONG m_nCpuUsage;
        ULONGLONG m_dwLastRun;
        DWORD m_dwProcessID;
        HANDLE m_hProcess;
        volatile LONG m_lRunCount;
    };



    ULONGLONG getCpuUsage(const QString &processName)
    {
        DWORD handle = getIDByName(processName);

        if (handle != 0)
        {
            return getCpuUsage(handle);
        }

        return -1;
    }

    ULONGLONG getCpuUsage(DWORD processID)
    {
        if (0 == processID)
        {
            return -1;
        }

        CpuUsage cpu(processID);
        return cpu.getUsageEx();
    }


    ulong getCurrentWorkingSetByPID(DWORD processID)
    {
        HANDLE processHandle = getHandleByID(processID);

        if (processHandle != NULL)
        {
            return getCurrentWorkingSetByHandle(processHandle);
        }

        return 0;
    }

    ulong getCurrentWorkingSetByName(const QString &processName)
    {
        HANDLE handle = getHandleByName(processName);

        if (handle != NULL)
        {
            return getCurrentWorkingSetByHandle(handle);
        }

        return 0;
    }

    ulong getCurrentWorkingSetByHandle(HANDLE handle)
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        return pmc.WorkingSetSize / 1024;
    }

    ulong getPeekWorkingSetByPID(DWORD processID)
    {
        HANDLE processHandle = getHandleByID(processID);

        if (processHandle != NULL)
        {
            return getPeekWorkingSetByHandle(processHandle);
        }

        return 0;
    }

    ulong getPeekWorkingSetByName(const QString &processName)
    {
        HANDLE handle = getHandleByName(processName);

        if (handle != NULL)
        {
            return getPeekWorkingSetByHandle(handle);
        }

        return 0;
    }

    ulong getPeekWorkingSetByHandle(HANDLE handle)
    {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(handle, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        return pmc.PeakWorkingSetSize / 1024;
    }

    ulong getPrivateWorkingSetByPID(DWORD processID)
    {
        // hold the working set
        DWORD dWorkingSetPages[1024 * 128] = {0};

        // information get from QueryWorkingSet()
        DWORD dPageSize = 0x1000;

        DWORD dSharedPages = 0;
        DWORD dPrivatePages = 0;
        DWORD dPageTablePages = 0;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                                      FALSE,
                                      processID);

        if (!hProcess)
        {
            return 0;
        }

        ulong WSPrivate;

        __try
        {
            if (!QueryWorkingSet(hProcess, dWorkingSetPages, sizeof(dWorkingSetPages)))
            {
                __leave;
            }

            DWORD dPages = dWorkingSetPages[0];

            qsort(&dWorkingSetPages[1], dPages, sizeof(DWORD), Compare);

            for (DWORD i = 1; i <= dPages; i++)
            {
                DWORD dCurrentPageStatus = 0;
                DWORD dCurrentPageAddress;
                DWORD dNextPageAddress;
                DWORD dNextPageFlags;
                DWORD dPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
                DWORD dPageFlags = dWorkingSetPages[i] & 0x00000FFF;

                while (i <= dPages) // iterate all pages
                {
                    dCurrentPageStatus++;

                    if (i == dPages) //if last page
                    {
                        break;
                    }

                    dCurrentPageAddress = dWorkingSetPages[i] & 0xFFFFF000;
                    dNextPageAddress = dWorkingSetPages[i + 1] & 0xFFFFF000;
                    dNextPageFlags = dWorkingSetPages[i + 1] & 0x00000FFF;

                    //decide whether iterate further or exit
                    //(this is non-contiguous page or have different flags) 
                    if ((dNextPageAddress == (dCurrentPageAddress + dPageSize))
                        && (dNextPageFlags == dPageFlags))
                    {
                        i++;
                    }
                    else
                    {
                        break;
                    }
                }

                if ((dPageAddress < 0xC0000000) || (dPageAddress > 0xE0000000))
                {
                    if (dPageFlags & 0x100)
                    {
                        // this is shared one
                        dSharedPages += dCurrentPageStatus;
                    }
                    else
                    {
                        // private one
                        dPrivatePages += dCurrentPageStatus;
                    }
                }
                else
                {
                    //page table region
                    dPageTablePages += dCurrentPageStatus;
                }
            }

            DWORD dTotal = dPages * 4;
            DWORD dShared = dSharedPages * 4;
            WSPrivate = dTotal - dShared;
        }
        __finally
        {
            CloseHandle(hProcess);
        }

        return WSPrivate;
    }

    ulong getPrivateWorkingSetByName(const QString &processName)
    {
        return getPrivateWorkingSetByPID(getIDByName(processName));
    }

    ulong getSharedWorkingSetByPID(DWORD processID)
    {
        return getCurrentWorkingSetByPID(processID) - getPrivateWorkingSetByPID(processID);
    }

    ulong getSharedWorkingSetByName(const QString &processName)
    {
        return getCurrentWorkingSetByName(processName) - getPrivateWorkingSetByName(processName);
    }

    ulong getIDByName(const QString &processName)
    {
        PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

        if (hSnapshot != INVALID_HANDLE_VALUE)
        {
            if (Process32First(hSnapshot, &pe))
            {
                while (Process32Next(hSnapshot, &pe))
                {
                    if (lstrcmpi(processName.toStdWString().c_str(), pe.szExeFile) == 0)
                    {
                        return pe.th32ProcessID;
                    }
                }
            }

            CloseHandle(hSnapshot);
        }

        return 0;
    }

    QString getNameByID(DWORD processID)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

        if (hProcess != NULL)
        {
            WCHAR procName[MAX_PATH + 1] = {0};
            GetModuleFileNameEx(hProcess, NULL, procName, MAX_PATH);
            std::wstring ws(procName);
            std::string processName(ws.begin(), ws.end());
            std::size_t pathDelim = processName.find_last_of("/\\");

            if (pathDelim != std::string::npos)
            {
                CloseHandle(hProcess);
                return QString::fromStdString(processName.substr(pathDelim + 1));
            }
        }

        return "";
    }

    HANDLE getHandleByName(const QString &processName)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (INVALID_HANDLE_VALUE == hSnapshot)
        {
            return NULL;
        }

        PROCESSENTRY32 pe = { sizeof(pe) };

        if (Process32First(hSnapshot, &pe))
        {
            do
            {
                if (!wcscmp(pe.szExeFile, processName.toStdWString().c_str()))
                {
                    CloseHandle(hSnapshot);
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
                }
            } while (Process32Next(hSnapshot, &pe));
        }

        CloseHandle(hSnapshot);
        return NULL;
    }

    HANDLE getHandleByID(DWORD processId)
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (INVALID_HANDLE_VALUE == hSnapshot)
        {
            return NULL;
        }

        PROCESSENTRY32 pe = { sizeof(pe) };

        if (Process32First(hSnapshot, &pe))
        {
            do
            {
                if (pe.th32ProcessID == processId)
                {
                    CloseHandle(hSnapshot);
                    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe.th32ProcessID);
                }
            } while (Process32Next(hSnapshot, &pe));
        }

        CloseHandle(hSnapshot);
        return NULL;
    }

    HANDLE getCurrentID()
    {
        return GetCurrentProcess();
    }

}
