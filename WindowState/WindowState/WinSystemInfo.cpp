#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tlhelp32.h>
#include "WinSystemInfo.h"

#include <stdlib.h>
//#include <stdio.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x)   HeapFree(GetProcessHeap(), 0, (x))
//#include <iomanip>//for get cpu usage
#include "psapi.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"psapi.lib")
#include <direct.h>

namespace WINSERV_STATE
{
    DWORD WINAPI CountCpuUsage(LPVOID lpParameter)
    {
        BOOL                        res;
        HANDLE                      hEvent;
        HANDLE                      hProcessSnap;
        PROCESSENTRY32              pe32;
        FILETIME                    exitTime;
        FILETIME                    creatTime;
        FILETIME                    procKernelTime;
        FILETIME                    procUserTime;
        FILETIME                    sysTime;
        bool                        newProc = false;
        std::vector<process_time_info>    procTimeInfo;
        WinSystemInfo* p = (WinSystemInfo*)lpParameter;
        //
        FILETIME preidleTime;
        FILETIME prekernelTime;
        FILETIME preuserTime;
        //
        FILETIME idleTime;
        FILETIME kernelTime;
        FILETIME userTime;
        //total cpu usage
        res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
        if (!res)
        {
            return GetLastError();
        }
        preidleTime = idleTime;
        prekernelTime = kernelTime;
        preuserTime = userTime;
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        MIB_IFTABLE *pIfTable = (MIB_IFTABLE *)MALLOC(sizeof(MIB_IFTABLE));
        //MIB_IFROW *pIfRow;
        static __int64 net_send = 0;
        static __int64 net_recv = 0;
        __int64 idle;
        __int64 kernel;
        __int64 user;
        DWORD dwRet = 0;
        DWORD dwSize = sizeof(MIB_IFTABLE);
        //获取初始网络流量数据
        if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
            FREE(pIfTable);
            pIfTable = (MIB_IFTABLE *)MALLOC(dwSize);
            if (pIfTable == NULL) {
                return GetLastError();
            }
        }
        dwRet = GetIfTable(pIfTable, &dwSize, FALSE);
        if (dwRet == NO_ERROR)
        {
            for (DWORD i = 0; i < pIfTable->dwNumEntries; i++){
                net_send += (__int64)(pIfTable->table[i].dwOutOctets);
                net_recv += (__int64)(pIfTable->table[i].dwInOctets);
            }
        }
        //process infos
        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE)
            return GetLastError();
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (!Process32First(hProcessSnap, &pe32))
        {
            CloseHandle(hProcessSnap);
            return GetLastError();
        }
        GetSystemTimeAsFileTime(&sysTime);
        do{
            process_time_info   procTime;
            HANDLE  hP = OpenProcess(PROCESS_ALL_ACCESS/*PROCESS_QUERY_INFORMATION | PROCESS_VM_READ*/, FALSE, pe32.th32ProcessID);
            if (hP == NULL)
                continue;

            procTime.pid = (int)pe32.th32ProcessID;
            procTime.lastSysTime = sysTime;
            GetProcessTimes(hP, &creatTime, &exitTime, &procTime.kernelTime, &procTime.userTime);
            procTimeInfo.push_back(procTime);
            CloseHandle(hP);
        } while (Process32Next(hProcessSnap, &pe32));
        CloseHandle(hProcessSnap);
        int processorCount = p->Get_processor_number();
        while (1)
        {

            if (WaitForSingleObject(hEvent, 1000) == WAIT_FAILED)
                return GetLastError();
            res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
            if (!res)
            {
                return GetLastError();
            }
            idle = p->CompareFileTime(preidleTime, idleTime);
            kernel = p->CompareFileTime(prekernelTime, kernelTime);
            user = p->CompareFileTime(preuserTime, userTime);
            double cpu = (((double)(kernel + user - idle) / (double)(kernel + user)) * 100 + 0.5);
            if ((UINT)cpu != 0)
            {
                EnterCriticalSection(&p->m_cpu_critical);
                p->m_cpuUsage = cpu;
                LeaveCriticalSection(&p->m_cpu_critical);
            }
            preidleTime = idleTime;
            prekernelTime = kernelTime;
            preuserTime = userTime;
            //计算网络流量
            dwRet = GetIfTable(pIfTable, &dwSize, FALSE);
            if (dwRet == NO_ERROR)
            {
                __int64                 total_send = 0;
                __int64                 total_recv = 0;
                for (DWORD i = 0; i < pIfTable->dwNumEntries; i++)
                {

                    total_send += pIfTable->table[i].dwOutOctets;
                    total_recv += pIfTable->table[i].dwInOctets;

                }
                EnterCriticalSection(&p->m_cpu_critical);
                p->m_netInfo.send = total_send - net_send;
                p->m_netInfo.recv = total_recv - net_recv;
                p->m_netInfo.total = p->m_netInfo.send + p->m_netInfo.recv;
                LeaveCriticalSection(&p->m_cpu_critical);
                net_send = total_send;
                net_recv = total_recv;
            }//no error

            //更新进程信息;
            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == INVALID_HANDLE_VALUE)
                return GetLastError();

            pe32.dwSize = sizeof(PROCESSENTRY32);

            if (!Process32First(hProcessSnap, &pe32))
            {
                CloseHandle(hProcessSnap);    // Must clean up the snapshot object!
                return GetLastError();
            }
            EnterCriticalSection(&p->m_cpu_critical);
            p->m_procInfo.clear();
            LeaveCriticalSection(&p->m_cpu_critical);

            do{
                HANDLE  hP = OpenProcess(PROCESS_ALL_ACCESS/*PROCESS_QUERY_INFORMATION | PROCESS_VM_READ*/, FALSE, pe32.th32ProcessID);
                if (hP == NULL)
                    continue;
                GetProcessTimes(hP, &creatTime, &exitTime, &procKernelTime, &procUserTime);
                GetSystemTimeAsFileTime(&sysTime);
                for (size_t i = 0; i < procTimeInfo.size(); i++)
                {
                    newProc = true;
                    if (procTimeInfo[i].pid == pe32.th32ProcessID)
                    {
                        //进程运行的总时间
                        __int64 k = p->CompareFileTime(procTimeInfo[i].kernelTime, procKernelTime);
                        __int64 u = p->CompareFileTime(procTimeInfo[i].userTime, procUserTime);
                        //已用系统时间
                        __int64 sysProc = p->CompareFileTime(procTimeInfo[i].lastSysTime, sysTime);

                        double cpu = (((double)(k + u) * 100) / sysProc / processorCount + 0.50001);
                        PROCESS_MEMORY_COUNTERS_EX pmc;
                        pmc.cb = sizeof(pmc);
                        GetProcessMemoryInfo(hP, (PROCESS_MEMORY_COUNTERS*)(&pmc), sizeof(pmc));
                        sys_process_info    tmp;
                        tmp.cpuUsage = (int)cpu;
                        tmp.memUsage = pmc.PrivateUsage / 1024;
                        std::wstring ws(pe32.szExeFile);
                        tmp.name = std::string(ws.begin(), ws.end());
                        tmp.pid = (int)pe32.th32ProcessID;

                        EnterCriticalSection(&p->m_cpu_critical);
                        p->m_procInfo.push_back(tmp);
                        LeaveCriticalSection(&p->m_cpu_critical);
                        newProc = false;
                        procTimeInfo[i].kernelTime = procKernelTime;
                        procTimeInfo[i].userTime = procUserTime;
                        procTimeInfo[i].lastSysTime = sysTime;
                        break;
                    }
                }
                if (newProc)
                {
                    process_time_info   procTime;
                    HANDLE  hP2 = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
                    if (hP == NULL)
                        break;

                    procTime.pid = (int)pe32.th32ProcessID;
                    procTime.lastSysTime = sysTime;
                    procTime.kernelTime = procKernelTime;
                    procTime.userTime = procUserTime;
                    CloseHandle(hP2);
                }
                CloseHandle(hP);
            } while (Process32Next(hProcessSnap, &pe32));
            CloseHandle(hProcessSnap);
        }//while
        if (pIfTable != NULL)
        {
            FREE(pIfTable);
            pIfTable = NULL;
        }

        EnterCriticalSection(&p->m_cpu_critical);

        if (p->m_cpuUsage > 100)
        {
            p->m_cpuUsage = 100;
        }

        if (p->m_cpuUsage < 0)
        {
            p->m_cpuUsage = 1;
        }

        LeaveCriticalSection(&p->m_cpu_critical);
        return DWORD(p->m_cpuUsage);
    }

    WinSystemInfo::WinSystemInfo(void)
    {
        InitializeCriticalSection(&m_cpu_critical);
        m_hCpuCountThread = NULL;
        m_cpuUsage = 10;

        m_totalDiskSpace = 0;
        initialize();
    }

    WinSystemInfo::~WinSystemInfo(void)
    {
        DeleteCriticalSection(&m_cpu_critical);
        if (m_hCpuCountThread != NULL)
            CloseHandle(m_hCpuCountThread);
    }

    void WinSystemInfo::initialize()
    {
        if (m_hCpuCountThread == NULL)
        {
            SECURITY_ATTRIBUTES sa;
            sa.bInheritHandle = true;
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = NULL;
            m_hCpuCountThread = CreateThread(&sa, 0, CountCpuUsage, this, 0, NULL);
        }
    }

    int WinSystemInfo::getDrivesInfo(std::vector<DiskInfo>& drives, ULONGLONG& totalSpace, ULONGLONG& totalFreeSpace)
    {
        char name;
        BOOL b_flag = FALSE;
        ULARGE_INTEGER FreeAvailable, TotalNum, TotalFreeNum;
        totalSpace = 0;
        totalFreeSpace = 0;
        ULONGLONG totalSpaceTmp = 0;
        ULONGLONG totalFreeSpaceTmp = 0;
        for (name = 'A'; name <= 'Z'; name++)
        {
            wchar_t diskname[16] = { 0 };
            swprintf_s(diskname, L"%c:", name);

            b_flag = GetDiskFreeSpaceEx(diskname, &FreeAvailable, &TotalNum, &TotalFreeNum);
            if (b_flag)
            {
                if (m_totalDiskSpace == 0)
                {
                    totalSpaceTmp = (int)(TotalNum.QuadPart / (1024 * 1024));
                    totalSpace += totalSpaceTmp;
                }
                totalFreeSpaceTmp = (int)(FreeAvailable.QuadPart / (1024 * 1024));
                totalFreeSpace += totalFreeSpaceTmp;
                b_flag = false;
                DiskInfo        disk;
                std::wstring ws(diskname);
                disk.name = std::string(ws.begin(), ws.end());
                disk.total = totalSpaceTmp;
                disk.free = totalFreeSpaceTmp;
                drives.push_back(disk);
            }
        }

        if (m_totalDiskSpace == 0)
        {
            m_totalDiskSpace = totalSpace;
        }

        totalSpace = m_totalDiskSpace;
        return drives.size();
    }

    void WinSystemInfo::getVirMemInfo(sys_vir_mem_info& info)
    {
        MEMORYSTATUSEX                  mymem;
        mymem.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mymem);
        memset(&info, 0, sizeof(info));
        info.total = (mymem.ullTotalVirtual) / 1024 / 1024;
        info.free = (mymem.ullAvailVirtual) / 1024 / 1024;
    }

    int WinSystemInfo::getDiskAvailable()
    {
        std::vector<DiskInfo> tmp;
        ULONGLONG totalSpace, totalFreeSpace;
        getDrivesInfo(tmp, totalSpace, totalFreeSpace);
        return (int)((totalSpace - totalFreeSpace) * 100 / totalSpace);
    }

    void WinSystemInfo::GetMemInfo(sys_mem_info& info)
    {
        //mem status info
        MEMORYSTATUSEX                  mymem;
        mymem.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mymem);
        memset(&info, 0, sizeof(info));
        info.total = (mymem.ullTotalPhys) / 1024 / 1024;
        info.free = (mymem.ullAvailPhys) / 1024 / 1024;
        info.used = mymem.dwMemoryLoad;
    }

    __int64 WinSystemInfo::CompareFileTime(const FILETIME& time1, const FILETIME& time2)
    {
        __int64 a = time1.dwHighDateTime << 16 << 16;
        a |= time1.dwLowDateTime;
        __int64 b = time2.dwHighDateTime << 16 << 16;
        b |= time2.dwLowDateTime;
        return   (b - a);
    }

    double WinSystemInfo::GetCpuInfo()
    {
        double tmp;
        EnterCriticalSection(&m_cpu_critical);
        tmp = m_cpuUsage;
        LeaveCriticalSection(&m_cpu_critical);
        return tmp;
    }

    sys_net_info WinSystemInfo::GetNetInfo()
    {
        sys_net_info netInfo;
        EnterCriticalSection(&m_cpu_critical);
        netInfo.recv = m_netInfo.recv;
        netInfo.send = m_netInfo.send;
        netInfo.total = m_netInfo.total;
        LeaveCriticalSection(&m_cpu_critical);
        return netInfo;
    }

    void WinSystemInfo::GetConnInfo(std::vector<SysNetConnInfo>& conns)
    {
        MIB_TCPTABLE*   ptTables = NULL;
        MIB_UDPTABLE*   puTables = NULL;
        DWORD           retVal = 0;
#if WINVER >= 0x0600 //0x0600是vista的版本号
        MIB_TCP6TABLE*  pt6Tables = NULL;
        MIB_UDP6TABLE*  pu6Tables = NULL;
#endif
        DWORD           size = 0;
        int             id = 0;

        if (ERROR_INSUFFICIENT_BUFFER == (retVal = GetExtendedTcpTable(ptTables, &size, FALSE, AF_INET, TCP_TABLE_BASIC_ALL, NULL)))
        {
            if (ptTables != NULL)
            {
                delete ptTables;
                ptTables = 0;
            }
            ptTables = new MIB_TCPTABLE[size];
            if (ptTables != NULL)
                if (NO_ERROR != (retVal = GetExtendedTcpTable(ptTables, &size, FALSE, AF_INET, TCP_TABLE_BASIC_ALL, NULL)))
                    return;
        }

        //get tcp connections
        if (retVal == NO_ERROR)
        {
            for (DWORD i = 0; i < ptTables->dwNumEntries; i++)
            {
                SysNetConnInfo  tmp;
                in_addr n1, n2;
                n1.S_un.S_addr = (u_long)ptTables->table[i].dwLocalAddr;
                n2.S_un.S_addr = (u_long)ptTables->table[i].dwRemoteAddr;
                char localBuf[20] = { '\0' };
                char remoteBuf[20] = { '\0' };
                tmp.id = int(id++);
                tmp.localAddr = std::string(inet_ntop(AF_INET, (void*)&n1, localBuf, 16));
                tmp.localPort = ptTables->table[i].dwLocalPort;
                tmp.remoteAddr = std::string(inet_ntop(AF_INET, (void*)&n2, remoteBuf, 16));
                tmp.remotePort = ptTables->table[i].dwRemotePort;
                tmp.protocol = TCP;
                tmp.state = ptTables->table[i].State;
                /*
                tmp.ipInfo.tcpInfo.dwState          = -1;
                tmp.id                              = int(id++);
                tmp.type                            = TCP;
                tmp.ipInfo.tcpInfo.State            = ptTables->table[i].State;
                tmp.ipInfo.tcpInfo.dwLocalAddr      = ptTables->table[i].dwLocalAddr;
                tmp.ipInfo.tcpInfo.dwLocalPort      = ptTables->table[i].dwLocalPort;
                tmp.ipInfo.tcpInfo.dwRemoteAddr     = ptTables->table[i].dwRemoteAddr;
                tmp.ipInfo.tcpInfo.dwRemotePort     = ptTables->table[i].dwRemotePort;
                tmp.ipInfo.tcpInfo.dwState          = ptTables->table[i].dwState;
                */
                conns.push_back(tmp);
            }
            if (ptTables != NULL)
            {
                delete ptTables;
                ptTables = NULL;
            }
        }
        size = 0;
        //get udp connections
        if (ERROR_INSUFFICIENT_BUFFER == (retVal = GetExtendedUdpTable(puTables, &size, FALSE, AF_INET, UDP_TABLE_BASIC, NULL)))
        {
            if (puTables != NULL)
            {
                delete puTables;
                puTables = 0;
            }

            puTables = new MIB_UDPTABLE[size];

            if (puTables != NULL)
            {
                if (NO_ERROR != (retVal = GetExtendedUdpTable(puTables, &size, FALSE, AF_INET, UDP_TABLE_BASIC, NULL)))
                {
                    return;
                }
            }
        }

        if (retVal == NO_ERROR)
        {
            for (DWORD i = 0; i < puTables->dwNumEntries; i++)
            {
                SysNetConnInfo  tmp;
                in_addr n1;
                n1.S_un.S_addr = (u_long)puTables->table[i].dwLocalAddr;
                char localBuf[20] = {'\0'};
                tmp.id = int(id++);
                tmp.localAddr = std::string(inet_ntop(AF_INET, (void*)&n1, localBuf, 16));
                tmp.localPort = puTables->table[i].dwLocalPort;
                tmp.remoteAddr = "";
                tmp.remotePort = 0;
                tmp.protocol = UDP;
                tmp.state = 0;
                /*
                tmp.id                              = int(id++);
                tmp.type                            = UDP;
                tmp.ipInfo.udpInfo.dwLocalAddr      = puTables->table[i].dwLocalAddr;
                tmp.ipInfo.udpInfo.dwLocalPort      = puTables->table[i].dwLocalPort;
                */
                conns.push_back(tmp);
            }
            if (puTables != NULL)
            {
                delete puTables;
                puTables = NULL;
            }
        }
        size = 0;

#if WINVER >= 0x0600 //0x0600是vista的版本号
        if (ERROR_INSUFFICIENT_BUFFER == (retVal = GetExtendedTcpTable(pt6Tables, &size, FALSE, AF_INET6, TCP_TABLE_BASIC_ALL, NULL)))
        {
            if (pt6Tables != NULL)
            {
                delete pt6Tables;
                pt6Tables = 0;
            }

            pt6Tables = new MIB_TCP6TABLE[size];

            if (pt6Tables != NULL)
            {
                if (NO_ERROR != (retVal = GetExtendedTcpTable(pt6Tables, &size, FALSE, AF_INET6, TCP_TABLE_BASIC_ALL, NULL)))
                {
                    return;
                }
            }
        }

        //get tcp6 connections
        if (retVal == NO_ERROR)
        {
            for (DWORD i = 0; i < pt6Tables->dwNumEntries; i++)
            {
                SysNetConnInfo  tmp;
                char in1[64] = { '\0' };
                char in2[64] = { '\0' };
                sprintf_s(in1, "%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x",
                    pt6Tables->table[i].LocalAddr.u.Word[0],
                    pt6Tables->table[i].LocalAddr.u.Word[1],
                    pt6Tables->table[i].LocalAddr.u.Word[2],
                    pt6Tables->table[i].LocalAddr.u.Word[3],
                    pt6Tables->table[i].LocalAddr.u.Word[4],
                    pt6Tables->table[i].LocalAddr.u.Word[5],
                    pt6Tables->table[i].LocalAddr.u.Word[6],
                    pt6Tables->table[i].LocalAddr.u.Word[7]
                    );
                sprintf_s(in2, "%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x",
                    pt6Tables->table[i].RemoteAddr.u.Word[0],
                    pt6Tables->table[i].RemoteAddr.u.Word[1],
                    pt6Tables->table[i].RemoteAddr.u.Word[2],
                    pt6Tables->table[i].RemoteAddr.u.Word[3],
                    pt6Tables->table[i].RemoteAddr.u.Word[4],
                    pt6Tables->table[i].RemoteAddr.u.Word[5],
                    pt6Tables->table[i].RemoteAddr.u.Word[6],
                    pt6Tables->table[i].RemoteAddr.u.Word[7]
                    );
                //InetNtop(AF_INET6,&(pt6Tables->table[i].LocalAddr),in1,64);
                //InetNtop(AF_INET6,&(pt6Tables->table[i].RemoteAddr),in2,64);
                tmp.id = int(id++);
                tmp.localAddr = std::string(in1);
                tmp.localPort = pt6Tables->table[i].dwLocalPort;
                tmp.remoteAddr = std::string(in2);
                tmp.remotePort = pt6Tables->table[i].dwRemotePort;
                tmp.protocol = TCP6;
                tmp.state = pt6Tables->table[i].State;
                /*
                //tmp.ipInfo.tcp6Info.dwState           = -1;
                tmp.id                              = int(id++);
                tmp.type                            = TCP6;
                tmp.ipInfo.tcp6Info.State           = pt6Tables->table[i].State;
                tmp.ipInfo.tcp6Info.LocalAddr       = pt6Tables->table[i].LocalAddr;
                tmp.ipInfo.tcp6Info.dwLocalScopeId  = pt6Tables->table[i].dwLocalScopeId;
                tmp.ipInfo.tcp6Info.dwLocalPort     = pt6Tables->table[i].dwLocalPort;
                tmp.ipInfo.tcp6Info.RemoteAddr      = pt6Tables->table[i].RemoteAddr;
                tmp.ipInfo.tcp6Info.dwRemoteScopeId = pt6Tables->table[i].dwRemoteScopeId;
                tmp.ipInfo.tcp6Info.dwRemotePort    = pt6Tables->table[i].dwRemotePort;
                //tmp.ipInfo.tcp6Info.dwState           = pt6Tables->table[i].dwState;
                */
                conns.push_back(tmp);
            }

            if (pu6Tables != NULL)
            {
                delete pt6Tables;
                pt6Tables = NULL;
            }
        }
        size = 0;
        //get udp6 connections
        if (ERROR_INSUFFICIENT_BUFFER == (retVal = GetExtendedUdpTable(pu6Tables, &size, FALSE, AF_INET6, UDP_TABLE_BASIC, NULL)))
        {
            if (pu6Tables != NULL)
            {
                delete pu6Tables;
                pu6Tables = 0;
            }

            pu6Tables = new MIB_UDP6TABLE[size];

            if (pu6Tables != NULL)
            {
                if (NO_ERROR != (retVal = GetExtendedUdpTable(pu6Tables, &size, FALSE, AF_INET6, UDP_TABLE_BASIC, NULL)))
                {
                    return;
                }
            }
        }

        if (retVal == NO_ERROR)
        {
            for (DWORD i = 0; i < pu6Tables->dwNumEntries; i++)
            {
                SysNetConnInfo  tmp;
                char in1[64] = { '\0' };
                sprintf_s(in1, "%4x:%4x:%4x:%4x:%4x:%4x:%4x:%4x",
                    pu6Tables->table[i].dwLocalAddr.u.Word[0],
                    pu6Tables->table[i].dwLocalAddr.u.Word[1],
                    pu6Tables->table[i].dwLocalAddr.u.Word[2],
                    pu6Tables->table[i].dwLocalAddr.u.Word[3],
                    pu6Tables->table[i].dwLocalAddr.u.Word[4],
                    pu6Tables->table[i].dwLocalAddr.u.Word[5],
                    pu6Tables->table[i].dwLocalAddr.u.Word[6],
                    pu6Tables->table[i].dwLocalAddr.u.Word[7]
                    );
                //InetNtop(AF_INET6,&(pt6Tables->table[i].LocalAddr),in1,64);

                tmp.id = int(id++);
                tmp.localAddr = std::string(in1);
                tmp.localPort = pu6Tables->table[i].dwLocalPort;
                tmp.remoteAddr = "";
                tmp.remotePort = 0;
                tmp.protocol = UDP6;
                tmp.state = 0;
                /*
                sys_conn_info   tmp;
                tmp.id                              = int(id++);
                tmp.type                            = UDP6;
                tmp.ipInfo.udp6Info.dwLocalAddr     = pu6Tables->table[i].dwLocalAddr;
                tmp.ipInfo.udp6Info.dwLocalPort     = pu6Tables->table[i].dwLocalPort;
                tmp.ipInfo.udp6Info.dwLocalScopeId  = pu6Tables->table[i].dwLocalScopeId;
                */
                conns.push_back(tmp);
            }
            if (pu6Tables != NULL)
            {
                delete pu6Tables;
                pu6Tables = 0;
            }
        }
#endif
    }

    void WinSystemInfo::GetProcInfo(std::vector<sys_process_info>& procInfo)
    {
        EnterCriticalSection(&m_cpu_critical);
        procInfo = m_procInfo;
        LeaveCriticalSection(&m_cpu_critical);
    }

    int WinSystemInfo::Get_processor_number()
    {
        SYSTEM_INFO info;
        GetSystemInfo(&info);
        return (int)info.dwNumberOfProcessors;
    }
}