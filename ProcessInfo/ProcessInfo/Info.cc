
#include "Info.h"

#pragma comment(lib, "psapi.lib") 

ProcInfo::ProcInfo(wchar_t* name)
    : m_wstrEXEName(name)
    , m_Handle(NULL)
{

}

ProcInfo::~ProcInfo()
{
    clear();
}

void ProcInfo::clear()
{
    m_wstrEXEName = NULL;

    if (NULL != m_Handle)
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
    }
}

HANDLE ProcInfo::GetProcessHandle(int nID)
{
    return OpenProcess(PROCESS_ALL_ACCESS, FALSE, nID);
}

HANDLE ProcInfo::GetProcessHandleByName(wchar_t* pName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (INVALID_HANDLE_VALUE == hSnapshot)
    {
        return NULL;
    }

    PROCESSENTRY32 pe = { sizeof(pe) };

    BOOL fOk;

    for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
    {
        if (!wcscmp(pe.szExeFile, pName))
        {
            CloseHandle(hSnapshot);
            return GetProcessHandle(pe.th32ProcessID);
        }
    }

    return NULL;
}

int ProcInfo::get_processor_number()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<int>(info.dwNumberOfProcessors); 
}

uint64_t ProcInfo::file_time_2_utc(const FILETIME* ftime)
{
    LARGE_INTEGER li;

    if (NULL == ftime)
    {
        return -1;
    }

    li.LowPart = ftime->dwLowDateTime;
    li.HighPart = ftime->dwHighDateTime;
    return li.QuadPart; 
}

bool ProcInfo::setHandle()
{
    m_Handle = GetProcessHandleByName(m_wstrEXEName);

    return NULL != m_Handle;
}

bool ProcInfo::checkHandle()
{
    return setHandle();
}


int ProcInfo::get_cpu_usage()
{
    processor_count_ = -1;

    //上一次的时间  
    static int64_t last_time_ = 0;
    static int64_t last_system_time_ = 0;

    FILETIME now;
    FILETIME creation_time;
    FILETIME exit_time;
    FILETIME kernel_time;
    FILETIME user_time;
    int64_t system_time;
    int64_t time;
    int64_t system_time_delta;
    int64_t time_delta;  

    int cpu = -1;

    if(processor_count_ == -1)
    {
        processor_count_ = ProcInfo::get_processor_number();
    }

    GetSystemTimeAsFileTime(&now);

    if (!GetProcessTimes(getHandle(), &creation_time, &exit_time, &kernel_time, &user_time))
    {
        // We don't assert here because in some cases (such as in the Task   Manager)
        // we may call this function on a process that has just exited but we have
        // not yet received the notification.
        return -1;
    }

    system_time = (ProcInfo::file_time_2_utc(&kernel_time) + ProcInfo::file_time_2_utc(&user_time))/processor_count_;
    time = ProcInfo::file_time_2_utc(&now);

    if ((last_system_time_ == 0) || (last_time_ == 0))
    {  
        // First call, just set the last values.
        last_system_time_ = system_time;
        last_time_ = time;
        return -1;
    }  

    system_time_delta = system_time - last_system_time_;
    time_delta = time - last_time_;

    assert(time_delta != 0);

    if (time_delta == 0)
    {
        return -1;
    }

    // We add time_delta / 2 so the result is rounded.
    cpu = (int)((system_time_delta * 100 + time_delta / 2) / time_delta);
    last_system_time_ = system_time;
    last_time_ = time;

    return cpu;
}

int ProcInfo::get_memory_usage(uint64_t* mem, uint64_t* vmem)
{
    PROCESS_MEMORY_COUNTERS pmc;

    if(GetProcessMemoryInfo(getHandle(), &pmc, sizeof(pmc)))
    {
        if(mem)
        {
            *mem = pmc.WorkingSetSize;
        }

        if(vmem)
        {
            *vmem = pmc.PagefileUsage; 
        }

        return 0;
    }  
    return -1;
}

int ProcInfo::get_io_bytes(uint64_t* read_bytes, uint64_t* write_bytes)
{
    IO_COUNTERS io_counter;

    if(GetProcessIoCounters(getHandle(), &io_counter))
    {
        if(read_bytes)
        {
            *read_bytes = io_counter.ReadTransferCount;
        }

        if(write_bytes)
        {
            *write_bytes = io_counter.WriteTransferCount;
        }

        return 0;
    }

    return -1;
}