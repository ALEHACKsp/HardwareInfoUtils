
/*
*参考 http://zhangyafeikimi.iteye.com/blog/378658
*/

#ifndef INFO_H
#define INFO_H

#include <windows.h>
#include <psapi.h>
#include <assert.h>
#include <tlhelp32.h>
#include <stdio.h>

typedef long long           int64_t;
typedef unsigned long long  uint64_t;

//cpu数量
static int processor_count_ = 0;

class ProcInfo
{
public:
    ProcInfo(wchar_t* name);
    ~ProcInfo();


public:
    //获取当前进程的cpu使用率，返回-1失败  
    int get_cpu_usage();

    //获取当前进程内存和虚拟内存使用量，返回-1失败，0成功
    int get_memory_usage(uint64_t* mem, uint64_t* vmem);

    //获取当前进程总共读和写的IO字节数，返回-1失败，0成功  
    int get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes);

    //时间转换  
    static uint64_t file_time_2_utc(const FILETIME* ftime);

    //获得CPU的核数
    static int get_processor_number();

    HANDLE GetProcessHandle(int nID);

    HANDLE GetProcessHandleByName(wchar_t* pName);

    void setEXEName(wchar_t* name) { m_wstrEXEName = name; }
    wchar_t* getEXEName() { return m_wstrEXEName; }

    HANDLE getHandle() { return m_Handle; }
    bool setHandle();
    bool checkHandle();

    void clear();

private:
    wchar_t*    m_wstrEXEName;
    HANDLE      m_Handle;
};


#endif