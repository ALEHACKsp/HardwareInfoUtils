#ifndef GLDPROCESSFUNC_H
#define GLDPROCESSFUNC_H

#include <windows.h>
#include <string>
#include <list>

using namespace std;

class CpuUsage
{
public:
    CpuUsage(DWORD dwProcessID);
    ULONGLONG getUsageEx();
    ULONGLONG getSystemNonIdleTimes();
    ULONGLONG getProcessNonIdleTimes();

private:
    ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB);
    ULONGLONG addTimes(const FILETIME& ftA, const FILETIME& ftB);
    bool enoughTimePassed();
    inline bool isFirstRun() const { return (m_dwLastRun == 0); }

    //system total times
    FILETIME m_ftPrevSysKernel;
    FILETIME m_ftPrevSysUser;

    //process times
    FILETIME m_ftPrevProcKernel;
    FILETIME m_ftPrevProcUser;

    ULONGLONG m_ullPrevSysNonIdleTime; // ��������ͺ���ı�����¼�ϴλ�ȡ�ķ�idle��ϵͳcpuʱ��ͽ���cpuʱ��
    ULONGLONG m_ullPrevProcNonIdleTime;// �����ֻ��һ������, �ڹ��캯�������ʼ������

    ULONGLONG m_nCpuUsage;
    ULONGLONG m_dwLastRun;
    DWORD m_dwProcessID;
    HANDLE m_hProcess;
    volatile LONG m_lRunCount;
};

class GLDProcessFunc
{
public:
    static ULONGLONG getCpuUsage(const string &processName);
    static ULONGLONG getCpuUsage(DWORD processID);

    //��ǰָ�����̵�ռ�õ��ڴ�KBΪ��Ԫ
    static ULONGLONG getMemoryInfo(const string &processName);
    static ULONGLONG getMemoryInfo(DWORD processID);

    static DWORD getIDByName(const string &processName);
    static string getNameByID(DWORD processID);
    static bool isProcessRunning(TCHAR *szEXEName);
    static bool isProcessRunning(const list<string> &exeNameList);
    static bool isProcessRunning(const string &processName);
    static bool killProcess(const string &lpProcessName);
    static list<string> getPathByName(const string &lpProcessName);
    static bool killProcessByAbPath(const string &lpProcessPath);
    static list<DWORD> getProcessIDList(const list<string> &processNameList);

    //����ָ������,��Ҫ�Ƿ�ֹ·�����пո��������
    static BOOL startProcess(const string &strExe, const list<string> &params);
    static bool startProcess(const string &strExe);
    static HANDLE getCurrentID();

private:
    static list<string> parseCombinedArgString(const string &program);
    static string createCommandLine(const string & program, const list<string> & arguments);
};


// ���ϵͳCPUʹ����
class CCPUUseRate
{
public:
    BOOL Initialize()
    {
        FILETIME ftIdle, ftKernel, ftUser;
        BOOL flag = FALSE;

        if (flag = GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
        {
            m_fOldCPUIdleTime = FileTimeToDouble(ftIdle);
            m_fOldCPUKernelTime = FileTimeToDouble(ftKernel);
            m_fOldCPUUserTime = FileTimeToDouble(ftUser);

        }

        return flag;
    }

    //����Initialize��Ҫ�ȴ�1���ٵ��ô˺���
    int GetCPUUseRate()
    {
        int nCPUUseRate = -1;
        FILETIME ftIdle, ftKernel, ftUser;

        if (GetSystemTimes(&ftIdle, &ftKernel, &ftUser))
        {
            double fCPUIdleTime = FileTimeToDouble(ftIdle);
            double fCPUKernelTime = FileTimeToDouble(ftKernel);
            double fCPUUserTime = FileTimeToDouble(ftUser);
            nCPUUseRate = (int)(100.0 - (fCPUIdleTime - m_fOldCPUIdleTime)
                / (fCPUKernelTime - m_fOldCPUKernelTime + fCPUUserTime - m_fOldCPUUserTime)
                *100.0);
            m_fOldCPUIdleTime = fCPUIdleTime;
            m_fOldCPUKernelTime = fCPUKernelTime;
            m_fOldCPUUserTime = fCPUUserTime;
        }

        return nCPUUseRate;
    }

private:
    double FileTimeToDouble(FILETIME &filetime)
    {
        return (double)(filetime.dwHighDateTime * 4.294967296E9) + (double)filetime.dwLowDateTime;
    }

private:
    double m_fOldCPUIdleTime;
    double m_fOldCPUKernelTime;
    double m_fOldCPUUserTime;
};




#endif // GLDPROCESSFUNC_H
