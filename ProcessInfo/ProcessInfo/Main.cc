#include "Info.h"

int main()
{
    wchar_t *strEXEName = L"notepad++.exe";

    ProcInfo* pProcInfo = new ProcInfo(strEXEName);

    if (NULL == pProcInfo)
    {
        printf("new ProcInfo Error\n");
        return -1;
    }

    bool bFlag = false;
    while(!pProcInfo->checkHandle())
    {
        if (!bFlag)
        {
            wprintf(L"wait for Process %s start\n", strEXEName);
            bFlag = true;
        }

        Sleep(1000);
    }

    wprintf(L"\n\nProcess %s already started\n\n", strEXEName);

    printf("CPU\t\t Mem\t\tVMem\t\tRead\t\t   Write\t\t\n");
    while(pProcInfo->checkHandle())
    {
        int cpu = 0;
        uint64_t mem, vmem, r, w;

        cpu = pProcInfo->get_cpu_usage();

        pProcInfo->get_memory_usage(&mem, &vmem);
        pProcInfo->get_io_bytes(&r, &w);

        printf("%10u",cpu);
        printf(" %10u k", mem/1024);
        printf("  %10u k", vmem/1024);
        printf("   %10u byte", r);
        printf("    %10u byte\n", w);

        Sleep(1000);
    }

    wprintf(L"\n\nProcess %s closed\n\n", strEXEName);

    if (NULL != pProcInfo)
    {
        delete pProcInfo;
        pProcInfo = NULL;
    }

    strEXEName = NULL;

    return 0;
}