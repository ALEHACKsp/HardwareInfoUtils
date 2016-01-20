#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <tchar.h>

typedef BOOL(WINAPI *LPFN_GLPI)(
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
    PDWORD);


struct CpuInfo
{
    int m_numaNodeCount;         // 插槽,对于有多个处理器插槽的服务器,物理cpu个数很可能会大于1
    int m_processorPackageCount; // 物理cpu个数
    int m_processorCoreCount;    // 处理器核心数
    int m_logicalProcessorCount; // 逻辑处理器
    int m_processorL1CacheCount; // L1缓存
    int m_processorL2CacheCount; // L2缓存
    int m_processorL3CacheCount; // L2缓存
    DWORD m_processorType;       // 处理器类型
    float m_processSpeed;
};

// Helper function to count set bits in the processor mask.
DWORD countSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i)
    {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }

    return bitSetCount;
}

float ProcSpeedCalc()
{
    /*
    RdTSC:
    It's the Pentium instruction "ReaD Time Stamp Counter". It measures the
    number of clock cycles that have passed since the processor was reset, as a
    64-bit number. That's what the <CODE>_emit</CODE> lines do.*/
#define RdTSC __asm _emit 0x0f __asm _emit 0x31

    // variables for the clock-cycles:
    __int64 cyclesStart = 0, cyclesStop = 0;
    // variables for the High-Res Preformance Counter:
    unsigned __int64 nCtr = 0, nFreq = 0, nCtrStop = 0;


    // retrieve performance-counter frequency per second:
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&nFreq)) return 0;

    // retrieve the current value of the performance counter:
    QueryPerformanceCounter((LARGE_INTEGER *)&nCtrStop);

    // add the frequency to the counter-value:
    nCtrStop += nFreq;


    _asm
    {// retrieve the clock-cycles for the start value:
        RdTSC
            mov DWORD PTR cyclesStart, eax
            mov DWORD PTR[cyclesStart + 4], edx
    }

    do{
        // retrieve the value of the performance counter
        // until 1 sec has gone by:
        QueryPerformanceCounter((LARGE_INTEGER *)&nCtr);
    } while (nCtr < nCtrStop);

    _asm
    {// retrieve again the clock-cycles after 1 sec. has gone by:
        RdTSC
            mov DWORD PTR cyclesStop, eax
            mov DWORD PTR[cyclesStop + 4], edx
    }

    // stop-start is speed in Hz divided by 1,000,000 is speed in MHz
    return    ((float)cyclesStop - (float)cyclesStart) / 1000000;
}

string getCpuBrand()
{
    // Get extended ids.
    int CPUInfo[4] = { -1 };
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if (i == 0x80000002)
        {
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        }
        else if (i == 0x80000003)
        {
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        }
        else if (i == 0x80000004)
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }

    return CPUBrandString;
}

double ProcSpeedRead()
{
    string sMHz;
    wchar_t Buffer[_MAX_PATH];
    DWORD BufSize = _MAX_PATH;
    DWORD dwMHz = _MAX_PATH;
    HKEY hKey;

    // open the key where the proc speed is hidden:
    long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_READ,
        &hKey);

    if (lError != ERROR_SUCCESS)
    {
        // if the key is not found, tell the user why:
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            lError,
            0,
            Buffer,
            _MAX_PATH,
            0);

        wprintf(Buffer);
        return 0;
    }

    // query the key:
    RegQueryValueEx(hKey, L"~MHz", NULL, NULL, (LPBYTE)&dwMHz, &BufSize);

    // convert the DWORD to a CString:
    //sMHz.Format("%i", dwMHz);

    return (double)dwMHz;
}

CpuInfo getCpuInfo()
{
    CpuInfo cpuInfo;

    LPFN_GLPI glpi;
    BOOL done = FALSE;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
    DWORD returnLength = 0;
    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    DWORD byteOffset = 0;
    PCACHE_DESCRIPTOR Cache;

    glpi = (LPFN_GLPI)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),
        "GetLogicalProcessorInformation");
    if (NULL == glpi)
    {
        _tprintf(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
    }

    while (!done)
    {
        DWORD rc = glpi(buffer, &returnLength);

        if (FALSE == rc)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer)
                    free(buffer);

                buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
                    returnLength);

                if (NULL == buffer)
                {
                    _tprintf(TEXT("\nError: Allocation failure\n"));
                }
            }
            else
            {
                _tprintf(TEXT("\nError %d\n"), GetLastError());
            }
        }
        else
        {
            done = TRUE;
        }
    }

    ptr = buffer;

    while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
    {
        switch (ptr->Relationship)
        {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;

        case RelationProcessorCore:
            processorCoreCount++;

            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += countSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            Cache = &ptr->Cache;
            if (Cache->Level == 1)
            {
                processorL1CacheCount++;
            }
            else if (Cache->Level == 2)
            {
                processorL2CacheCount++;
            }
            else if (Cache->Level == 3)
            {
                processorL3CacheCount++;
            }
            break;

        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            _tprintf(TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
            break;
        }
        byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
        ptr++;
    }

    SYSTEM_INFO siSysInfo;
    // Copy the hardware information to the SYSTEM_INFO structure. 
    GetSystemInfo(&siSysInfo);

    cpuInfo.m_logicalProcessorCount = logicalProcessorCount;
    cpuInfo.m_numaNodeCount = numaNodeCount;
    cpuInfo.m_processorCoreCount = processorCoreCount;
    cpuInfo.m_processorPackageCount = processorPackageCount;
    cpuInfo.m_processorL1CacheCount = processorL1CacheCount;
    cpuInfo.m_processorL2CacheCount = processorL2CacheCount;
    cpuInfo.m_processorL3CacheCount = processorL3CacheCount;

    cpuInfo.m_processorType = siSysInfo.dwProcessorType;
    cpuInfo.m_processSpeed = ProcSpeedCalc();
    DWORD speed = ProcSpeedRead();

    return cpuInfo;
}