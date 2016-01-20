#ifndef GLDDISKINFOUTILS_H
#define GLDDISKINFOUTILS_H

#include <windows.h>
#include <Winioctl.h>

#include <bitset>
#include <QVector>
#include <QString>

using namespace std;

namespace QDiskInfo
{

    enum FS
    {
        OTHER_FORMAT = 0,
        FAT32        = 1,
        NTFS         = 2
    };

    enum VOLUMETYPE
    {
        UNKNOWNDEVICE,   // 未知设备
        REMOVABLEDEVICE, // 可移动设备
        FIXEDDEVICE,     // 磁盘分区
        REMOTEDEVICE,    // 网络磁盘
        CDROMDEVICE,     // 光驱
        RAMDISKDEVICE,   // 虚拟磁盘
        INVALIDPATH      // 无效路径
    };

    typedef struct DiskInfomation
    {
        DiskInfomation()
        {
            m_dwFreeMBytes = 0;
            m_dwTotalMBytes = 0;
        }

        QString      m_strDiskName;   // 分区名(盘符)
        QString      m_strTypeName;   // 分区类型
        QString      m_strFileSystem; // 分区格式
        qulonglong   m_dwTotalMBytes; // 总空间
        qulonglong   m_dwFreeMBytes;  // 可用空间
    }DiskInfomation;

    /**
     * @brief 获取系统盘符
     * @return
     */
    QString getSystemVolumeName();

    /**
     * @brief 获取当前盘符
     * @return
     */
    QString getCurrentVolumeName();

    /**
     * @brief 获取硬盘序列号
     * @return
     */
    QString getPhysicalDriveSerialNumber();

    /**
     * @brief 获取分区个数
     * @return
     */
    ulong getVolumeNum();

    /**
     * @brief 获取所有分区信息
     * @return
     */
    QVector<DiskInfomation> getAllVolumeInfo();

    /**
     * @brief 获取分区格式
     * @param dir  分区名
     * @return
     */
    FS getFileSystemType(const QString& dir);

    /**
     * @brief 获取分区类型
     * @param dir   分区名
     * @return
     */
    VOLUMETYPE getVolumeTypeItem(const QString& dir);

    /**
     * @brief 获取分区空间信息
     * @param dir                      分区名
     * @param ri64FreeBytesToCaller    可用空间
     * @param ri64TotalBytes           分区总空间
     * @return
     */
    bool getVolumeSpace(const QString& dir, qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes);

    /**
     * @brief 获取硬盘个数
     * @return
     */
    ulong getDiskNum();

    /**
    * @brief 获取硬盘空间信息
    * @param ri64FreeBytesToCaller    硬盘可用空间
    * @param ri64TotalBytes           硬盘总空间
    * @return
    */
    bool getDiskSpaceInfo(qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes);

    /**
    * @brief 获取所有分区的名字
    * @param volumeNameVct    存放分区名的容器
    * @return
    */
    bool getAllVolumeName(QVector<QString> & volumeNameVct);

}

#endif // GLDDISKINFOUTILS_H
