#include "QDiskInfoUtils.h"
#include <atlstr.h>

#include <memory>

namespace QDiskInfo
{

    QString getSystemVolumeName()
    {
        WCHAR str[MAX_PATH] = {0};
        char sysDiskName[MAX_PATH * 2 + 1] = {0};
        GetSystemDirectory(str, MAX_PATH);
        WideCharToMultiByte(CP_ACP,
                            0,
                            str,
                            -1,
                            sysDiskName,
                            sizeof(sysDiskName),
                            NULL,
                            NULL);
        return QString(sysDiskName[0]);
    }

    QString getCurrentVolumeName()
    {
        /* Path of Module */
        WCHAR szModulePath[MAX_PATH] = { 0 };

        char curDiskName[MAX_PATH * 2 + 1] = { 0 };

        // Get current module handle
        HMODULE module = GetModuleHandle(0);

        // Get current file path
        GetModuleFileName(module,
                          szModulePath,
                          sizeof(szModulePath));

        WideCharToMultiByte(CP_ACP,
                            0,
                            szModulePath,
                            -1,
                            curDiskName,
                            sizeof(curDiskName),
                            NULL,
                            NULL);

        return QString(curDiskName[0]);
    }

    QString getPhysicalDriveSerialNumber()
    {
        DWORD dwResult = NO_ERROR;
        UINT nDriveNumber = 0;
        CString strSerialNumber;
        strSerialNumber.Empty();

        // Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
        CString strDrivePath;
        strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), nDriveNumber);

        // call CreateFile to get a handle to physical drive
        HANDLE hDevice = ::CreateFile(strDrivePath,
                                      0,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      NULL,
                                      OPEN_EXISTING,
                                      0,
                                      NULL);

        std::shared_ptr<void> spHandle(hDevice, CloseHandle);

        if (INVALID_HANDLE_VALUE == spHandle.get())
        {
            return "";
        }

        // set the input STORAGE_PROPERTY_QUERY data structure
        STORAGE_PROPERTY_QUERY storagePropertyQuery;
        ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
        storagePropertyQuery.PropertyId = StorageDeviceProperty;
        storagePropertyQuery.QueryType = PropertyStandardQuery;

        // get the necessary output buffer size
        STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
        DWORD dwBytesReturned = 0;
        if (!::DeviceIoControl(spHandle.get(), IOCTL_STORAGE_QUERY_PROPERTY,
            &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
            &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
            &dwBytesReturned, NULL))
        {
            dwResult = ::GetLastError();
            return "";
        }

        // allocate the necessary memory for the output buffer
        const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
        BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
        ZeroMemory(pOutBuffer, dwOutBufferSize);

        // get the storage device descriptor
        if (!::DeviceIoControl(spHandle.get(), IOCTL_STORAGE_QUERY_PROPERTY,
            &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
            pOutBuffer, dwOutBufferSize,
            &dwBytesReturned, NULL))
        {
            dwResult = ::GetLastError();
            delete[]pOutBuffer;
            return "";
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
        delete[] pOutBuffer;

        std::basic_string<TCHAR> intermediate((LPCTSTR)strSerialNumber);
        return QString::fromStdWString(intermediate).trimmed();
    }

    QVector<DiskInfomation> getAllVolumeInfo()
    {
        QVector<DiskInfomation> diskInfoVect;
        QVector<QString> drvNameVct;

        if (getAllVolumeName(drvNameVct))
        {
            QVector<QString>::const_iterator iter = drvNameVct.begin();
            for (; iter != drvNameVct.end(); ++iter)
            {
                //deal every driver
                DiskInfomation dskinf;
                //add disk name
                dskinf.m_strDiskName = *iter;
                //add disk type
                dskinf.m_strTypeName = getVolumeTypeItem(*iter);
                //add filesystem type
                switch (getFileSystemType(*iter))
                {
                case FAT32:
                    dskinf.m_strFileSystem = "FAT32";
                    break;
                case NTFS:
                    dskinf.m_strFileSystem = "NTFS";
                    break;
                default:
                    dskinf.m_strFileSystem = "OTHER_FORMAT";
                    break;
                }
                //add space information
                if (dskinf.m_strTypeName == QStringLiteral("磁盘分区") && dskinf.m_strFileSystem != "OTHER_FORMAT")
                {
                    qulonglong i64TotalBytes = 0;
                    qulonglong i64FreeBytes = 0;

                    if (getVolumeSpace(*iter, i64FreeBytes, i64TotalBytes))
                    {
                        dskinf.m_dwFreeMBytes = i64FreeBytes;
                        dskinf.m_dwTotalMBytes = i64TotalBytes;
                    }
                    else
                    {
                        dskinf.m_dwFreeMBytes = 0;
                        dskinf.m_dwTotalMBytes = 0;
                    }

                    diskInfoVect.push_back(dskinf);
                }
            }
        }

        return diskInfoVect;
    }

    ulong getVolumeNum()
    {
        ulong diskCount = 0;
        DWORD DiskInfo = GetLogicalDrives();
        //利用GetLogicalDrives()函数可以获取系统中逻辑驱动器的数量，函数返回的是一个32位无符号整型数据。
        while(DiskInfo)//通过循环操作查看每一位数据是否为1，如果为1则磁盘为真,如果为0则磁盘不存在。
        {
            if(DiskInfo & 1)//通过位运算的逻辑与操作，判断是否为1
            {
                ++diskCount;
            }

            DiskInfo = DiskInfo >> 1;//通过位运算的右移操作保证每循环一次所检查的位置向右移动一位。
        }

        return diskCount;
    }

    //n个driver，以A：\null的形式存放的话，需4n个字符的数组，猜想，实际获得4n＋1个字符的数组，可见末尾是以nullnull结束字符数组
    bool getAllVolumeName(QVector<QString> & volumeNameVct)
    {
        ulong dwDrvNum = getVolumeNum();

        //通过GetLogicalDriveStrings()函数获取所有驱动器字符串信息长度。
        DWORD dwLength = GetLogicalDriveStringsA(0,NULL);

        //用获取的长度在堆区创建一个c风格的字符串数组
        char* DStr = new char[dwLength];
        dwLength = GetLogicalDriveStringsA(dwLength, (LPSTR)DStr);
        if (!dwLength)
        {
            delete [] DStr;
            return false;
        }

        DWORD dwIndex = 0;

        while (dwIndex < dwDrvNum)
        {
            QString tmp(DStr + 4 * dwIndex);
            volumeNameVct.push_back(tmp);
            dwIndex++;
        }

        delete [] DStr;
        return true;
    }

    VOLUMETYPE getVolumeTypeItem(const QString& dir)
    {
        UINT uiType = GetDriveTypeA(dir.toStdString().c_str());
        switch (uiType)
        {
        case DRIVE_UNKNOWN:
            return UNKNOWNDEVICE;
        case DRIVE_REMOVABLE:
            return REMOVABLEDEVICE;
        case DRIVE_FIXED:
            return FIXEDDEVICE;
        case DRIVE_REMOTE:
            return REMOTEDEVICE;
        case DRIVE_CDROM:
            return CDROMDEVICE;
        case DRIVE_RAMDISK:
            return RAMDISKDEVICE;
        default:
            return INVALIDPATH;
        }
    }

    FS getFileSystemType(const QString &dir)
    {
        //对于光驱信息的需排除之
        QString temp = dir[0];
        temp += ":";
        temp += "\\";

        char Volumelabel[255];
        DWORD SerialNumber;
        DWORD MaxCLength;
        DWORD FileSysFlag;
        char FileSysName[10];
        GetVolumeInformationA(temp.toStdString().c_str(), Volumelabel, 255, &SerialNumber,
                              &MaxCLength, &FileSysFlag, FileSysName,255);

        if (strcmp(FileSysName, "NTFS") == 0 )
        {
            return NTFS;
        }
        else if (strcmp(FileSysName, "FAT32") == 0 )
        {
            return FAT32;
        }
        else
        {
            return OTHER_FORMAT;
        }
    }

    bool getVolumeSpace(const QString& dir, qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes)
    {
        typedef bool (WINAPI *PGETDISKFREESPACEEX)(LPCSTR,
            PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);

        PGETDISKFREESPACEEX pGetDiskFreeSpaceEx = NULL;
        qulonglong i64FreeBytes;
        bool fResult = false;

        pGetDiskFreeSpaceEx = (PGETDISKFREESPACEEX) GetProcAddress(
            GetModuleHandleA("kernel32.dll"),
            "GetDiskFreeSpaceExA");

        if (pGetDiskFreeSpaceEx)
        {
            fResult = pGetDiskFreeSpaceEx(dir.toStdString().c_str(),
                                          (PULARGE_INTEGER)&ri64FreeBytesToCaller,
                                          (PULARGE_INTEGER)&ri64TotalBytes,
                                          (PULARGE_INTEGER)&i64FreeBytes);

            if (fResult)
            {
                ri64FreeBytesToCaller = (DWORD)(ri64FreeBytesToCaller / 1024 / 1024);
                ri64TotalBytes = (DWORD)(ri64TotalBytes / 1024 / 1024);
            }
        }
        else
        {
            DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
            fResult = GetDiskFreeSpaceA(dir.toStdString().c_str(),
                                        &dwSectPerClust,
                                        &dwBytesPerSect,
                                        &dwFreeClusters,
                                        &dwTotalClusters);
            if (fResult)
            {
                ri64TotalBytes = (qulonglong)(dwTotalClusters * dwSectPerClust * dwBytesPerSect /(1024 * 1024));
                ri64FreeBytesToCaller = (qulonglong)(dwFreeClusters * dwSectPerClust * dwBytesPerSect / (1024 * 1024));
            }
        }

        return fResult;
    }

    bool getDiskSpaceInfo(qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes)
    {
        ri64FreeBytesToCaller = 0;
        ri64TotalBytes = 0;
        QVector<DiskInfomation> diskInfoVct = getAllVolumeInfo();
        if (0 == diskInfoVct.size())
        {
            return false;
        }

        foreach(DiskInfomation diskInfo, diskInfoVct)
        {
            ri64FreeBytesToCaller += diskInfo.m_dwFreeMBytes;
            ri64TotalBytes += diskInfo.m_dwTotalMBytes;
        }

        return true;
    }

    ulong getDiskNum()
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
            return -1;
        }

        DWORD dwType;
        DWORD dwValue;
        DWORD dwBufLen = sizeof(DWORD);

        lRet = ::RegQueryValueEx (hKEY, L"Count", NULL, &dwType, (BYTE*)&dwValue, &dwBufLen);

        if(lRet != ERROR_SUCCESS)
        {
            return -1;
        }

        return dwValue;
    }

}


