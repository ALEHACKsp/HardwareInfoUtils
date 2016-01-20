#include <QCoreApplication>
#include "QCpuInfoUtils.h"
#include "QDiskInfoUtils.h"
#include "QMemoryInfoUtils.h"
#include "QProcessUtils.h"
#include <windows.h>
#include <Psapi.h>

#include <QDebug>

using namespace QCpuInfo;
using namespace QDiskInfo;
using namespace QMemoryInfo;
using namespace QProcessInfo;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "CPU:======================================================";
    QString cpuBrand = getCpuBrand();
    qDebug() << "cpuBrand is" << cpuBrand;

    ulong speed = getCpuSpeed();
    qDebug() << "speed is" << speed;

    ulong count = getCpuCount();
    qDebug() << "count is" << count;

    CoreCount coreCount = getCpuCoreCount();
    qDebug() << "coreCount's cpuCoreCount is" << coreCount.m_cpuCoreCount;
    qDebug() << "coreCount's cpuLogicalCoreCount is" << coreCount.m_cpuLogicalCoreCount;


    CpuInfo cpuInfo = getCpuInfo();
    qDebug() << "cpubrand of cpuInfo is" << cpuInfo.m_cpuBrand;
    qDebug() << "physical core of cpuInfo is" << cpuInfo.m_coreCount.m_cpuCoreCount;
    qDebug() << "logical core of cpuInfo is" << cpuInfo.m_coreCount.m_cpuLogicalCoreCount;
    qDebug() << "physical cpu number of cpuInfo is" << cpuInfo.m_processorPackageCount;
    qDebug() << "speed of cpuInfo is" << cpuInfo.m_processSpeed;


    qDebug() << "DISK:======================================================";
    FS fileSystem = getFileSystemType("C:\\");
    qDebug() << "C's FS is" << fileSystem;

    QString voluName = getSystemVolumeName();
    qDebug() << "system volume name is" << voluName;

    QString currentVolName = getCurrentVolumeName();
    qDebug() << "current volume name is" << currentVolName;

    QString serialNumber = getPhysicalDriveSerialNumber();
    qDebug() << "Disk serialNumber is" << serialNumber;

    ulong volNumber = getVolumeNum();
    qDebug() << "volNumber is" << volNumber;

    VOLUMETYPE volType = getVolumeTypeItem("D:\\");
    qDebug() << "D's volType is" << volType;

    qulonglong ri64FreeBytesToCaller;
    qulonglong ri64TotalBytes;
    getVolumeSpace("E:\\", ri64FreeBytesToCaller, ri64TotalBytes);
    qDebug() << "E volume's free space is" << ri64FreeBytesToCaller;
    qDebug() << "E volume's totle space is" << ri64TotalBytes;

    qulonglong diskFreeBytesToCaller;
    qulonglong diskTotalBytes;
    getDiskSpaceInfo(diskFreeBytesToCaller, diskTotalBytes);
    qDebug() << "Disk free space is" << diskFreeBytesToCaller;
    qDebug() << "Disk totle space is" << diskTotalBytes;

    QVector<DiskInfomation> diskInfoVet = getAllVolumeInfo();
    foreach(DiskInfomation disk, diskInfoVet)
    {
        qDebug() << "disk name is" << disk.m_strDiskName;
        qDebug() << "file system is " << disk.m_strFileSystem;

        qDebug() << "disk type name is" << disk.m_strTypeName;
        qDebug() << "totle space is" << disk.m_dwTotalMBytes;
        qDebug() << "free space is" << disk.m_dwFreeMBytes;
    }

    qDebug() << "Disk number is" << getDiskNum();

    qDebug() << "Memory:======================================================";
    PhysMemInfo memInfo = getPhysMemInfo();
    qDebug() << "memInfo's m_totalPhys is" << memInfo.m_totalPhys;
    qDebug() << "memInfo's m_availPhys is " << memInfo.m_availPhys;
    qDebug() << "memInfo's m_usedPhys is " << memInfo.m_usedPhys;

    VirMemInfo virMemInfo = getVirMemInfo();
    qDebug() << "virMemInfo's m_totalVirtual is" << virMemInfo.m_totalVirtual;
    qDebug() << "virMemInfo's m_availVirtual is " << virMemInfo.m_availVirtual;

    // 此处检查电脑上notepad++.exe应用程序占用内存
    QString processName = "notepad++.exe";
    ulong currentWS = getCurrentWorkingSetByName(processName);
    qDebug() << "Current WS is" << currentWS;
    ulong peekWS = getPeekWorkingSetByName(processName);
    qDebug() << "Peek WS is" << peekWS;
    ulong privateWS = getPrivateWorkingSetByName(processName);
    qDebug() << "Private WS is" << privateWS;
    ulong sharedWS = getSharedWorkingSetByName(processName);
    qDebug() << "Shared WS is" << sharedWS;

    return a.exec();
}
