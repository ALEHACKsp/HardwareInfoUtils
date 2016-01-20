// WindowState.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "WinSystemInfo.h"
#include <iostream>
#include "MyDiskInfo.h"
#include <atlstr.h>

using namespace std;
using namespace WINSERV_STATE;

typedef struct PartitionInfo
{
    char                    chDrive;
    PARTITION_INFORMATION   info;
} PartitionInfo, *LPPartitionInfo;


typedef struct DiskInfo
{
    int                        iPartitionSize;
    PPARTITION_INFORMATION     pPartitions;
} DiskInfo, *LPDiskInfo;

int getAllDiskPartitionInfo(LPDiskInfo* lpDisks)
{
    HKEY hKEY;
    long lRet;
    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum",
        0,
        KEY_READ,
        &hKEY);

    if (lRet != ERROR_SUCCESS)
    {
        return 0;
    }

    int size = 0;
    DWORD dwType;
    DWORD dwValue;
    DWORD dwBufLen = sizeof(DWORD);
    __try
    {
        lRet = ::RegQueryValueEx(hKEY, L"Count", NULL, &dwType, (BYTE*)&dwValue, &dwBufLen);

        if (lRet != ERROR_SUCCESS)
        {
            __leave;//失败
        }

        *lpDisks = (LPDiskInfo)malloc(dwValue * 8/*sizeof(DiskInfo)*/);

        for (UINT i = 0; i < dwValue; i++)
        {
            wchar_t szDiskPos[128] = { 0 };
            wsprintf(szDiskPos, L"\\\\.\\PHYSICALDRIVE%u", i);

            HANDLE   hDevice = NULL;
            DWORD    nDiskBytesRead = 0;//预设为0，当缓冲区的长度不够时，该值为所需的缓冲区的长度
            DWORD    nDiskBufferSize = sizeof(DRIVE_LAYOUT_INFORMATION) + sizeof(PARTITION_INFORMATION) * 104;//26*4
            PDRIVE_LAYOUT_INFORMATION lpDiskPartInfo = (PDRIVE_LAYOUT_INFORMATION)malloc(nDiskBufferSize);

            if (lpDiskPartInfo == NULL)
            {
                free(lpDiskPartInfo);
                continue;
            }

            //将缓冲区lpDiskPartInfo的内容设为nDiskBufferSize个NULL
            memset(lpDiskPartInfo, 0, nDiskBufferSize);

            //////////////////////获得所有分区的信息///////////////////////////////////////
            hDevice = CreateFile(szDiskPos,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

            if (hDevice == NULL)
            {
                free(lpDiskPartInfo);
                continue;
            }

            /////////////获得某磁盘上的所有分区信息/////////////////////////
            BOOL fRet = DeviceIoControl(
                hDevice,
                IOCTL_DISK_GET_DRIVE_LAYOUT,
                NULL,
                0,
                (LPVOID)lpDiskPartInfo,
                (DWORD)nDiskBufferSize,
                (LPDWORD)&nDiskBytesRead,
                NULL
                );

            if (!fRet)
            {
                LPVOID lpMsgBuf;
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    GetLastError(),
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&lpMsgBuf,
                    0,
                    NULL
                    );
                LocalFree(lpMsgBuf);
                free(lpDiskPartInfo);
                CloseHandle(hDevice);
                continue;
            }

            //////////////////////////////导出分区信息///////////////////////////////////////
            DWORD dwPartitionCount = lpDiskPartInfo->PartitionCount;
            int iPartitions = dwPartitionCount / 4;
            (*lpDisks)[size].pPartitions = (PPARTITION_INFORMATION)malloc(iPartitions * sizeof(PARTITION_INFORMATION));
            (*lpDisks)[size].iPartitionSize = 0;

            //永远是实际的分区数的4倍,不能用的分区将会显示类型PARTITION_ENTRY_UNUSED,即分区类型为0
            ///////////////////依次获取导出某分区信息,并与目的驱动器进行比较///////////////////////////////////
            for (UINT j = 0; j < dwPartitionCount; j += 4)//+4是因为只有下标为4的整数倍的值才是正确的引用
            {
                memcpy(&((*lpDisks)[size].pPartitions[(*lpDisks)[size].iPartitionSize++]), &lpDiskPartInfo->PartitionEntry[j], sizeof(PARTITION_INFORMATION));
            }
            free(lpDiskPartInfo);
            CloseHandle(hDevice);
            ++size;
        }
    }
    __finally
    {
        if (hKEY != NULL)
        {
            RegCloseKey(hKEY);
        }
    }

    return size;
}

BOOL GetDiskAndPartitionNumber(int &DiskNumber, int &PartitionNumber)
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    int nDiskNum = 0;
    do
    {
        nDiskNum;
        TCHAR szDrive[1024] = { 0 };
        swprintf(szDrive, 1024, L"\\\\.\\PhysicalDrive%d", nDiskNum);
        hDevice = CreateFile(szDrive,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hDevice == INVALID_HANDLE_VALUE)
        {
            break;
        }
        DRIVE_LAYOUT_INFORMATION_EX layOutInfo[20];
        DWORD bytesReturned;

        memset(&layOutInfo, 0, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 20);

        if (DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &layOutInfo, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * 20, &bytesReturned, NULL) == 0)
        {
            CloseHandle(hDevice);
            return -1;
        }

        int partitionCount = layOutInfo[0].PartitionCount;
        cout << "Number of partitions:" << layOutInfo[0].PartitionCount / 4 << endl;

        ++nDiskNum;
    } while (hDevice != INVALID_HANDLE_VALUE);

    CloseHandle(hDevice);
    DiskNumber = nDiskNum;
    return nDiskNum ? TRUE : FALSE;
}

DWORD GetPhysicalDriveSerialNumber(UINT nDriveNumber, CString& strSerialNumber)
{
    DWORD dwResult = NO_ERROR;
    strSerialNumber.Empty();

    // Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
    CString strDrivePath;
    strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), nDriveNumber);

    // call CreateFile to get a handle to physical drive
    HANDLE hDevice = ::CreateFile(strDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == hDevice)
        return ::GetLastError();

    // set the input STORAGE_PROPERTY_QUERY data structure
    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
    storagePropertyQuery.PropertyId = StorageDeviceProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;

    // get the necessary output buffer size
    STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
    DWORD dwBytesReturned = 0;
    if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
        &dwBytesReturned, NULL))
    {
        dwResult = ::GetLastError();
        ::CloseHandle(hDevice);
        return dwResult;
    }

    // allocate the necessary memory for the output buffer
    const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
    BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
    ZeroMemory(pOutBuffer, dwOutBufferSize);

    // get the storage device descriptor
    if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        pOutBuffer, dwOutBufferSize,
        &dwBytesReturned, NULL))
    {
        dwResult = ::GetLastError();
        delete[]pOutBuffer;
        ::CloseHandle(hDevice);
        return dwResult;
    }

    // Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
    // followed by additional info like vendor ID, product ID, serial number, and so on.
    STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
    const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
    if (dwSerialNumberOffset != 0)
    {
        // finally, get the serial number
        strSerialNumber = CString(pOutBuffer + dwSerialNumberOffset);
    }

    // perform cleanup and return
    delete[]pOutBuffer;
    ::CloseHandle(hDevice);
    return dwResult;
}


int _tmain(int argc, _TCHAR* argv[])
{
    WinSystemInfo sysInfo;
    std::cout << "cpu usage is " << sysInfo.GetCpuInfo() << std::endl;
    std::cout << "cpu usage is " << sysInfo.getDiskAvailable() << std::endl;

    sys_mem_info menInfo;
    sysInfo.GetMemInfo(menInfo);
    std::cout << "cpu usage is " << menInfo.free << "   " << menInfo.total << "" << menInfo.used;

    sys_vir_mem_info virmenInfo;
    sysInfo.getVirMemInfo(virmenInfo);
    std::cout << "cpu usage is " << virmenInfo.free << "   " << virmenInfo.total << "" << virmenInfo.used;

    int diskNum;
    int particp;
    GetDiskAndPartitionNumber(diskNum, particp);

    LPDiskInfo lpDisks = NULL;
    LPPartitionInfo lpPartitions = NULL;
    int iDisks = getAllDiskPartitionInfo(&lpDisks);

    CMyDiskInfo diskInfo;
    diskInfo.GetDiskInfo();


    UINT nDriveNumber = 0;
    CString strSerialNumber;
    DWORD dwResult = GetPhysicalDriveSerialNumber(nDriveNumber, strSerialNumber);
    CString strReport;
    if (NO_ERROR == dwResult)
    {
        strReport.Format(_T("Drive #%u serial number: '%s'"), nDriveNumber, strSerialNumber);
    }
    else
    {
        strReport.Format(_T("GetPhysicalDriveSerialNumber failed. Error: %u"), dwResult);
    }
    ::MessageBox(NULL, strReport, _T("Test"), MB_OK);

    return 0;
}

