#include "stdafx.h"
#include "GLDProcess.h"
#include "StringUtils.h"

#include <regex>
#include <algorithm>

#include <strsafe.h>
#include <Tlhelp32.h>
#include <Psapi.h>

void errorMsg(LPTSTR lpszFunction);
BOOL setPrivilege(HANDLE hProcess, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);

CpuUsage::CpuUsage(DWORD dwProcessID)
    : m_nCpuUsage(0)
    , m_dwLastRun(0)
    , m_lRunCount(0)
    , m_dwProcessID(dwProcessID)
    , m_ullPrevProcNonIdleTime(0)                                                                                                                                      
    , m_ullPrevSysNonIdleTime(0)
{
    HANDLE hProcess = GetCurrentProcess();
    setPrivilege(hProcess, SE_DEBUG_NAME, TRUE);

    m_hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, m_dwProcessID);

    if (m_hProcess == 0)
    {
        errorMsg(TEXT("OpenProcess"));
    }

    ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

    ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
}

ULONGLONG CpuUsage::subtractTimes(const FILETIME &ftA, const FILETIME &ftB)
{
    LARGE_INTEGER liA, liB;
    liA.LowPart = ftA.dwLowDateTime;
    liA.HighPart = ftA.dwHighDateTime;

    liB.LowPart = ftB.dwLowDateTime;
    liB.HighPart = ftB.dwHighDateTime;

    return liA.QuadPart - liB.QuadPart;
}

ULONGLONG CpuUsage::addTimes(const FILETIME &ftA, const FILETIME &ftB)
{
    LARGE_INTEGER liA, liB;
    liA.LowPart = ftA.dwLowDateTime;
    liA.HighPart = ftA.dwHighDateTime;

    liB.LowPart = ftB.dwLowDateTime;
    liB.HighPart = ftB.dwHighDateTime;

    return liA.QuadPart + liB.QuadPart;
}

bool CpuUsage::enoughTimePassed()
{
    const int c_minElapsedMS = 250;//milliseconds

    ULONGLONG dwCurrentTickCount = GetTickCount();
    return (dwCurrentTickCount - m_dwLastRun) > c_minElapsedMS;
}

ULONGLONG CpuUsage::getSystemNonIdleTimes()
{
    FILETIME ftSysIdle, ftSysKernel, ftSysUser;

    if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser))
    {
        errorMsg(TEXT("GetSystemTimes"));
        return 0;
    }

    return addTimes(ftSysKernel, ftSysUser);
}

ULONGLONG CpuUsage::getProcessNonIdleTimes()
{
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

    if (!GetProcessTimes(m_hProcess, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser) && false)
    {
        errorMsg(TEXT("GetProcessNonIdleTimes"));
        return 0;
    }

    return addTimes(ftProcKernel, ftProcUser);
}

ULONGLONG CpuUsage::getUsageEx()
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

BOOL setPrivilege(HANDLE hProcess, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
    HANDLE hToken;

    if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        errorMsg(TEXT("OpenProcessToken"));
        return FALSE;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        errorMsg(TEXT("LookupPrivilegeValue"));
        return FALSE;
    }

    TOKEN_PRIVILEGES tkp;
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Luid = luid;
    tkp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : FALSE;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
    {
        errorMsg(TEXT("AdjustTokenPrivileges"));
        return FALSE;
    }

    return TRUE;
}

void errorMsg(LPTSTR lpszFunction)
{
        // 处理错误消息不正确，不要弹出MessageBox，不要ExitProcess退出进程，
        // 先把代码注释掉

        // Retrieve the system error message for the last-error code

        //    LPVOID lpMsgBuf;
        //    LPVOID lpDisplayBuf;
        //    DWORD dw = GetLastError();

        //    FormatMessage(
        //        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        //        FORMAT_MESSAGE_FROM_SYSTEM |
        //        FORMAT_MESSAGE_IGNORE_INSERTS,
        //        NULL,
        //        dw,
        //        LANG_USER_DEFAULT,
        //        (LPTSTR) &lpMsgBuf,
        //        0, NULL );

        //    // Display the error message

        //    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        //                                      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
        //    StringCchPrintf((LPTSTR)lpDisplayBuf,
        //                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        //                    TEXT("%s failed with error %d: %s"),
        //                    lpszFunction, dw, lpMsgBuf);
        //    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);


        //    LocalFree(lpMsgBuf);
        //    LocalFree(lpDisplayBuf);
        //    ExitProcess(dw);
}

ULONGLONG GLDProcessFunc::getCpuUsage(const string &processName)
{
    DWORD handle = GLDProcessFunc::getIDByName(processName);

    if (handle != 0)
    {
        return GLDProcessFunc::getCpuUsage(handle);
    }

    return 0;
}

ULONGLONG GLDProcessFunc::getCpuUsage(DWORD processID)
{
    if (0 == processID)
    {
        return 0;
    }

    CpuUsage cpu(processID);

    return cpu.getUsageEx();
}

ULONGLONG GLDProcessFunc::getMemoryInfo(const string &processName)
{
    DWORD handle = GLDProcessFunc::getIDByName(processName);

    if (handle != 0)
    {
        return GLDProcessFunc::getMemoryInfo(handle);
    }

    return 0;
}

ULONGLONG GLDProcessFunc::getMemoryInfo(DWORD processID)
{
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo((HANDLE)processID, &pmc, sizeof(pmc));
    return pmc.PagefileUsage / 1024;
}

DWORD GLDProcessFunc::getIDByName(const string &processName)
{
    std::wstring ws;
    ws.assign(processName.begin(), processName.end());

    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        if (Process32First(hSnapshot, &pe))
        {
            while (Process32Next(hSnapshot, &pe))
            {
                if (lstrcmpi(ws.c_str(), pe.szExeFile) == 0)
                {
                    return pe.th32ProcessID;
                }
            }
        }

        CloseHandle(hSnapshot);
    }

    return 0;
}

string GLDProcessFunc::getNameByID(DWORD processID)
{
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (hProcess != NULL)
    {
        TCHAR* procName = new TCHAR[MAX_PATH];
        GetModuleFileNameEx(hProcess, NULL, procName, MAX_PATH);

        wstring ws(procName);
        string processName = wstringTostring(ws);
        std::size_t pathDelim = processName.find_last_of("/\\");

        if (pathDelim != string::npos)
        {
            return processName.substr(pathDelim + 1);
        }

        return "";
    }

    return "";
}

bool GLDProcessFunc::killProcess(const string &lpProcessName)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapShot, &pe))
    {
        return FALSE;
    }

    bool result = false;
    string strProcessName = lpProcessName;
    while (Process32Next(hSnapShot, &pe))
    {
        std::wstring ws(pe.szExeFile);
        string scTmp = wstringTostring(ws);

        if (0 == _stricmp(scTmp.c_str(), strProcessName.c_str()))
        {
            DWORD dwProcessID = pe.th32ProcessID;

            if (dwProcessID == ::GetCurrentProcessId())
            {
                continue;
            }

            HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
            ::TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            result = true;
        }
    }

    return result;
}

list<string> GLDProcessFunc::getPathByName(const string &lpProcessName)
{
    list<string> retList;
    wstring ws = stringTowstring(lpProcessName);
    HANDLE hHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // 为当前系统进程建立快照
    DWORD dwId = ::GetCurrentProcessId();// 当前进程的Id
    if (INVALID_HANDLE_VALUE != hHandle)// 如果快照建立成功
    {
        PROCESSENTRY32 stEntry;
        stEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hHandle, &stEntry))     //在快照中查找一个进程,stEntry返回进程相关属性和信息
        {
            do{
                if (wcsstr(stEntry.szExeFile, ws.c_str()))    // 比较该进程名称是否与strProcessName相符
                {
                    if (dwId != stEntry.th32ProcessID)       // 如果相等，且该进程的Id与当前进程不相等，则找到
                    {
                        HANDLE h_Process = OpenProcess(PROCESS_QUERY_INFORMATION |
                            PROCESS_VM_READ,
                            FALSE, stEntry.th32ProcessID);
                        if (h_Process != NULL)
                        {
                            WCHAR name[MAX_PATH + 1] = { 0 };
                            GetModuleFileNameEx(h_Process, NULL, name, MAX_PATH + 1);
                            retList.push_back(wstringTostring(std::wstring(name)));
                            CloseHandle(h_Process);
                        }
                    }
                }
            } while (Process32Next(hHandle, &stEntry));   //再快照中查找下一个进程。
        }
        //       CloseToolhelp32Snapshot(hHandle);             //释放快照句柄。
    }
    return retList;
}

bool GLDProcessFunc::killProcessByAbPath(const string &lpProcessPath)
{
    bool bRet = false;

    string plPath;
    std::size_t pathDelim = lpProcessPath.find_last_of("/\\");
    if (pathDelim != string::npos)
    {
        plPath = lpProcessPath.substr(pathDelim + 1);
    }

    HANDLE hHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  //为当前系统进程建立快照
    DWORD dwId = ::GetCurrentProcessId();     //当前进程的Id
    if (INVALID_HANDLE_VALUE != hHandle)      //如果快照建立成功
    {
        PROCESSENTRY32 stEntry;
        stEntry.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hHandle, &stEntry))     //在快照中查找一个进程,stEntry返回进程相关属性和信息
        {
            do{
                if (wcsstr(stEntry.szExeFile, stringTowstring(plPath).c_str()))    // 比较该进程名称是否与strProcessName相符
                {
                    if (dwId != stEntry.th32ProcessID)       //如果相等，且该进程的Id与当前进程不相等，则找到
                    {
                        HANDLE h_Process = OpenProcess(PROCESS_ALL_ACCESS, TRUE, stEntry.th32ProcessID);
                        if (h_Process != NULL)
                        {
                            WCHAR name[MAX_PATH + 1] = { 0 };
                            GetModuleFileNameEx(h_Process, NULL, name, MAX_PATH + 1);

                            wstring ws(name);
                            string path = wstringTostring(ws);

                            if (path.compare(lpProcessPath) == 0)
                            {
                                TerminateProcess(h_Process, 0);
                                CloseHandle(h_Process);
                                bRet = true;
                            }
                        }
                    }
                }
            } while (Process32Next(hHandle, &stEntry));   //再快照中查找下一个进程。
        }
    }
    return bRet;
}

list<DWORD> GLDProcessFunc::getProcessIDList(const list<string> &processNameList)
{
    list<DWORD> processIDList;
    PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        if (Process32First(hSnapshot, &pe))
        {
            while (Process32Next(hSnapshot, &pe))
            {
                string sExeFile = wstringTostring(pe.szExeFile);
                list<string>::const_iterator iter = find(processNameList.begin(), processNameList.end(), sExeFile);

                if (iter != processNameList.end())
                {
                    processIDList.push_back(pe.th32ProcessID);
                }

                Sleep(10);
            }
        }

        CloseHandle(hSnapshot);
    }

    return processIDList;
}

list<string> GLDProcessFunc::parseCombinedArgString(const string &program)
{
    list<string> args;
    string tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (size_t i = 0; i < program.size(); ++i)
    {
        if (program.at(i) == '"')
        {
            ++quoteCount;

            if (quoteCount == 3)
            {
                // third consecutive quote
                quoteCount = 0;
                tmp += program.at(i);
            }

            continue;
        }

        if (quoteCount)
        {
            if (quoteCount == 1)
            {
                inQuote = !inQuote;
            }

            quoteCount = 0;
        }

        if (!inQuote && ' ' == program.at(i))
        {
            if ("" != tmp)
            {
                args.push_back(tmp);
                tmp.clear();
            }
        }
        else
        {
            tmp += program.at(i);
        }
    }

    if ("" != tmp)
    {
        args.push_back(tmp);
    }

    return args;
}

std::string GLDProcessFunc::createCommandLine(const string & program, const list<string> & arguments)
{
    string args;
    if ("" != program)
    {
        string semicolon = "\"";
        string space = " ";
        string programName = program;

        if (!startsWith(programName, semicolon) && !endsWith(programName, semicolon) && programName.find(' ') != string::npos)
        {
            programName = semicolon + programName + semicolon;
        }

        std::replace(programName.begin(), programName.end(), '/', '\\');

        // add the prgram as the first arg ... it works better
        args = programName + space;
    }

    for (size_t i = 0; i < arguments.size(); ++i)
    {
        std::list<string>::const_iterator it = arguments.begin();
        std::advance(it, i);
        string tmp = *it;

        // Quotes are escaped and their preceding backslashes are doubled.
        regex reg("(\\\\*)\"");
        string after = "\\1\\1\\\"";
        regex_replace(tmp, reg, after);

        if ("" == tmp || tmp.find(' ') != string::npos || tmp.find('\t') != string::npos)
        {
            // The argument must not end with a \ since this would be interpreted
            // as escaping the quote -- rather put the \ behind the quote: e.g.
            // rather use "foo"\ than "foo\"
            int i = tmp.length();

            while (i > 0 && tmp.at(i - 1) == '\\')
            {
                --i;
            }

            tmp.insert(i, 1, '"');
            tmp.insert(0, 1, '"');
        }

        args += ' ' + tmp;
    }

    return args;
}

BOOL GLDProcessFunc::startProcess(const string &strExe, const list<string> &params)
{
    string args = createCommandLine(strExe, params);

    BOOL success = false;
    PROCESS_INFORMATION pinfo;

    STARTUPINFOW startupInfo = { sizeof(STARTUPINFO), 0, 0, 0,
        (DWORD)CW_USEDEFAULT, (DWORD)CW_USEDEFAULT,
        (DWORD)CW_USEDEFAULT, (DWORD)CW_USEDEFAULT,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    wstring widestr = std::wstring(args.begin(), args.end());
    success = CreateProcess(0, const_cast<wchar_t *>(widestr.c_str()),
                            0, 0, FALSE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE,
                            0, 0, &startupInfo, &pinfo);

    if (success)
    {
        CloseHandle(pinfo.hThread);
        CloseHandle(pinfo.hProcess);
    }

    return success;
}

bool GLDProcessFunc::startProcess(const string &strExe)
{
    list<string> args = parseCombinedArgString(strExe);

    if (args.empty())
    {
        return false;
    }

    string prog = args.front();
    args.pop_front();

    if (!startProcess(prog, args))
    {
        std::wstring ws;
        ws.assign(strExe.begin(), strExe.end());
        ShellExecute(0, L"open", ws.c_str(), NULL, NULL, SW_SHOW);
    }

    return false;
}

HANDLE GLDProcessFunc::getCurrentID()
{
    return GetCurrentProcess();
}

bool GLDProcessFunc::isProcessRunning(TCHAR *szEXEName)
{
    HANDLE hHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  //为当前系统进程建立快照
    DWORD dwId = ::GetCurrentProcessId();     //当前进程的Id

    if (INVALID_HANDLE_VALUE != hHandle)      //如果快照建立成功
    {
        PROCESSENTRY32 stEntry;
        stEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hHandle, &stEntry))     //在快照中查找一个进程,stEntry返回进程相关属性和信息
        {
            do
            {
                if (wcsstr(stEntry.szExeFile, szEXEName))   //比较该进程名称是否与strProcessName相符
                {
                    if (dwId != stEntry.th32ProcessID)      //如果相等，且该进程的Id与当前进程不相等，则找到
                        return true;
                }
            } while (Process32Next(hHandle, &stEntry));   //再快照中查找下一个进程。
        }
        //   CloseToolhelp32Snapshot(hHandle);             //释放快照句柄。
    }
    return false;
}

bool GLDProcessFunc::isProcessRunning(const list<string> &exeNameList)
{
    HANDLE hHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);  // 为当前系统进程建立快照
    DWORD dwId = ::GetCurrentProcessId();                              // 当前进程的Id

    if (INVALID_HANDLE_VALUE != hHandle)                               // 如果快照建立成功
    {
        PROCESSENTRY32 stEntry;
        stEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hHandle, &stEntry))     //在快照中查找一个进程,stEntry返回进程相关属性和信息
        {
            do
            {
                wstring ws(stEntry.szExeFile);
                string sExeFile = wstringTostring(ws);
                if (find(exeNameList.begin(), exeNameList.end(), sExeFile) != exeNameList.end())
                    //if (wcsstr(stEntry.szExeFile, szEXEName))   //比较该进程名称是否与strProcessName相符
                {
                    if (dwId != stEntry.th32ProcessID)       //如果相等，且该进程的Id与当前进程不相等，则找到
                    {
                        return true;
                    }
                }
            } while(Process32Next(hHandle, &stEntry));     //再快照中查找下一个进程。
        }
        //   CloseToolhelp32Snapshot(hHandle);             //释放快照句柄。
    }

    return false;
}

bool GLDProcessFunc::isProcessRunning(const string &processName)
{
    return GLDProcessFunc::isProcessRunning((TCHAR*)processName.c_str());
}
