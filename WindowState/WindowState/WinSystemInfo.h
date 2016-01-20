#ifndef WinSystemInfo_include
#define WinSystemInfo_include

#include <winsock2.h>
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include <windows.h>
#include <string>
#include <vector>

namespace WINSERV_STATE
{
#pragma pack(push)
#pragma pack(8)

    struct sys_mem_info
    {
        __int64     total;              //总内存数，单位M
        __int64     free;               //可用内存数，单位M
        __int64     used;
    };

    struct sys_vir_mem_info
    {
        __int64     total;              //总内存数，单位M
        __int64     free;               //可用内存数，单位M
        __int64     used;
    }; 

    struct sys_net_info
    {
        __int64     send;               //每秒发送流量
        __int64     recv;               //每秒接受流量
        __int64     total;              //每秒钟总流量
    };

    struct DiskInfo
    {
        std::string     name;               //盘符
        __int64         total;              //总数单位M
        __int64         free;               //可用空间数单位M
    };

    struct sys_process_info
    {
        int             pid;            //进程ID
        std::string     name;           //进程名
        int             cpuUsage;       //进程cpu使用率
        double          memUsage;       //进程内存大小
    };

    struct process_time_info
    {
        int             pid;            //process id
        FILETIME        kernelTime;     //time spent in kernel mode
        FILETIME        userTime;       //time spent in user mode
        FILETIME        lastSysTime;    //上次时间
    };

    enum connection_type{ UNKNOWN = 0, TCP, UDP, TCP6, UDP6 };
    /*
    struct sys_conn_info
    {
    int             id;
    connection_type type;
    union   connInfo{
    #if (WINVER>=0x0600)
    MIB_TCP6ROW     tcp6Info;
    MIB_UDP6ROW     udp6Info;
    #endif
    MIB_TCPROW      tcpInfo;
    MIB_UDPROW      udpInfo;
    }ipInfo;
    };
    */
    struct SysNetConnInfo
    {
        int               id;
        int               protocol;
        std::string       localAddr;
        int               localPort;
        std::string       remoteAddr;
        int               remotePort;
        int               state;
    };

#pragma pack(pop)
    DWORD WINAPI CountCpuUsage(LPVOID lpParameter);

    class WinSystemInfo
    {
    public:
        WinSystemInfo(void);
        ~WinSystemInfo(void);
        //获取可用百分比
        int                 getDiskAvailable();
        //获取磁盘总空间和空闲空间
        int                 getDrivesInfo(std::vector<DiskInfo>& drives, ULONGLONG& totalSpace, ULONGLONG& totalFreeSpace);

        void                getVirMemInfo(sys_vir_mem_info& info);
        //get memory info
        void                GetMemInfo(sys_mem_info& info);
        //connection info
        void                GetConnInfo(std::vector<SysNetConnInfo>& conns);
        //return cpu usage 0~100
        double              GetCpuInfo();
        //net flow info
        sys_net_info        GetNetInfo();
        //process info
        void                GetProcInfo(std::vector<sys_process_info>& procInfo);
    private:
        friend DWORD WINAPI CountCpuUsage(LPVOID lpParameter);
        //内部使用变量
        __int64             CompareFileTime(const FILETIME& time1, const FILETIME& time2);
        int                 Get_processor_number();
        CRITICAL_SECTION    m_cpu_critical;
        double              m_cpuUsage;     //
        sys_net_info        m_netInfo;      //net flow info
        std::vector<sys_process_info>     m_procInfo;

        WinSystemInfo(const WinSystemInfo&);
        WinSystemInfo& operator= (const WinSystemInfo&);
        void                initialize();
        ULONGLONG           m_totalDiskSpace;
        HANDLE              m_hCpuCountThread;


    };
}
#endif