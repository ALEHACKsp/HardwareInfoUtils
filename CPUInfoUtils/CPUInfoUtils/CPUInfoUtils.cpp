// CPUInfoUtils.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "GLDProcess.h"
#include "GLDCpuInfo.h"
#include <stdio.h>
#include <conio.h>

#include <iostream>
#include <iomanip>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{

    printf("    Windows系统CPU内存网络性能统计第二篇 CPU CPU整体使用率\n");
    printf(" - http://blog.csdn.net/morewindows/article/details/8678359 -\n");
    printf(" -- By MoreWindows( http://blog.csdn.net/MoreWindows ) --\n\n");

    //CCPUUseRate cpuUseRate;
    //if (!cpuUseRate.Initialize())
    //{
    //    printf("Error! %d\n", GetLastError());
    //    _getch();
    //    return -1;
    //}
    //else
    //{
    //    while (true)
    //    {
    //        Sleep(1000);
    //        printf("\r当前CPU使用率为：%4d%%", cpuUseRate.GetCPUUseRate());
    //    }
    //}


    CpuInfo cpuInfo = getCpuInfo();

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    cout << setw(20) << "处理器掩码: " << systemInfo.dwActiveProcessorMask << endl;
    cout << setw(20) << "处理器个数: " << systemInfo.dwNumberOfProcessors << endl;
    cout << setw(20) << "处理器分页大小: " << systemInfo.dwPageSize << endl;
    cout << setw(20) << "处理器类型: " << systemInfo.dwProcessorType << endl;
    cout << setw(20) << "最大寻址单元: " << systemInfo.lpMaximumApplicationAddress << endl;
    cout << setw(20) << "最小寻址单元: " << systemInfo.lpMinimumApplicationAddress << endl;
    cout << setw(20) << "处理器等级: " << systemInfo.wProcessorLevel << endl;
    cout << setw(20) << "处理器版本: " << systemInfo.wProcessorRevision << endl;



    MEMORYSTATUS memoryInfo;
    GlobalMemoryStatus(&memoryInfo);

    cout << setw(20) << "内存使用率" << memoryInfo.dwMemoryLoad << endl;
    cout << setw(20) << "总的物理内存" << memoryInfo.dwTotalPhys << endl;

    cout << setw(20) << "可用物理内存" << memoryInfo.dwAvailPhys << endl;
    cout << setw(20) << "可用地址空间" << memoryInfo.dwTotalVirtual << endl;
    cout << setw(20) << "空闲地址空间" << memoryInfo.dwAvailVirtual << endl;

    cout << setw(20) << "交换文件总的大小" << memoryInfo.dwTotalPageFile << endl;
    cout << setw(20) << "可用交换文件大小" << memoryInfo.dwAvailVirtual << endl;


    return 0;
}

