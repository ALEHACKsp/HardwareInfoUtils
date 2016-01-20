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
        __int64     total;              //���ڴ�������λM
        __int64     free;               //�����ڴ�������λM
        __int64     used;
    };

    struct sys_vir_mem_info
    {
        __int64     total;              //���ڴ�������λM
        __int64     free;               //�����ڴ�������λM
        __int64     used;
    }; 

    struct sys_net_info
    {
        __int64     send;               //ÿ�뷢������
        __int64     recv;               //ÿ���������
        __int64     total;              //ÿ����������
    };

    struct DiskInfo
    {
        std::string     name;               //�̷�
        __int64         total;              //������λM
        __int64         free;               //���ÿռ�����λM
    };

    struct sys_process_info
    {
        int             pid;            //����ID
        std::string     name;           //������
        int             cpuUsage;       //����cpuʹ����
        double          memUsage;       //�����ڴ��С
    };

    struct process_time_info
    {
        int             pid;            //process id
        FILETIME        kernelTime;     //time spent in kernel mode
        FILETIME        userTime;       //time spent in user mode
        FILETIME        lastSysTime;    //�ϴ�ʱ��
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
        //��ȡ���ðٷֱ�
        int                 getDiskAvailable();
        //��ȡ�����ܿռ�Ϳ��пռ�
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
        //�ڲ�ʹ�ñ���
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