// CPUInfoUtils.cpp : �������̨Ӧ�ó������ڵ㡣
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

    printf("    WindowsϵͳCPU�ڴ���������ͳ�Ƶڶ�ƪ CPU CPU����ʹ����\n");
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
    //        printf("\r��ǰCPUʹ����Ϊ��%4d%%", cpuUseRate.GetCPUUseRate());
    //    }
    //}


    CpuInfo cpuInfo = getCpuInfo();

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    cout << setw(20) << "����������: " << systemInfo.dwActiveProcessorMask << endl;
    cout << setw(20) << "����������: " << systemInfo.dwNumberOfProcessors << endl;
    cout << setw(20) << "��������ҳ��С: " << systemInfo.dwPageSize << endl;
    cout << setw(20) << "����������: " << systemInfo.dwProcessorType << endl;
    cout << setw(20) << "���Ѱַ��Ԫ: " << systemInfo.lpMaximumApplicationAddress << endl;
    cout << setw(20) << "��СѰַ��Ԫ: " << systemInfo.lpMinimumApplicationAddress << endl;
    cout << setw(20) << "�������ȼ�: " << systemInfo.wProcessorLevel << endl;
    cout << setw(20) << "�������汾: " << systemInfo.wProcessorRevision << endl;



    MEMORYSTATUS memoryInfo;
    GlobalMemoryStatus(&memoryInfo);

    cout << setw(20) << "�ڴ�ʹ����" << memoryInfo.dwMemoryLoad << endl;
    cout << setw(20) << "�ܵ������ڴ�" << memoryInfo.dwTotalPhys << endl;

    cout << setw(20) << "���������ڴ�" << memoryInfo.dwAvailPhys << endl;
    cout << setw(20) << "���õ�ַ�ռ�" << memoryInfo.dwTotalVirtual << endl;
    cout << setw(20) << "���е�ַ�ռ�" << memoryInfo.dwAvailVirtual << endl;

    cout << setw(20) << "�����ļ��ܵĴ�С" << memoryInfo.dwTotalPageFile << endl;
    cout << setw(20) << "���ý����ļ���С" << memoryInfo.dwAvailVirtual << endl;


    return 0;
}

