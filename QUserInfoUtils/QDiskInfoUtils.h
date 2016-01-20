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
        UNKNOWNDEVICE,   // δ֪�豸
        REMOVABLEDEVICE, // ���ƶ��豸
        FIXEDDEVICE,     // ���̷���
        REMOTEDEVICE,    // �������
        CDROMDEVICE,     // ����
        RAMDISKDEVICE,   // �������
        INVALIDPATH      // ��Ч·��
    };

    typedef struct DiskInfomation
    {
        DiskInfomation()
        {
            m_dwFreeMBytes = 0;
            m_dwTotalMBytes = 0;
        }

        QString      m_strDiskName;   // ������(�̷�)
        QString      m_strTypeName;   // ��������
        QString      m_strFileSystem; // ������ʽ
        qulonglong   m_dwTotalMBytes; // �ܿռ�
        qulonglong   m_dwFreeMBytes;  // ���ÿռ�
    }DiskInfomation;

    /**
     * @brief ��ȡϵͳ�̷�
     * @return
     */
    QString getSystemVolumeName();

    /**
     * @brief ��ȡ��ǰ�̷�
     * @return
     */
    QString getCurrentVolumeName();

    /**
     * @brief ��ȡӲ�����к�
     * @return
     */
    QString getPhysicalDriveSerialNumber();

    /**
     * @brief ��ȡ��������
     * @return
     */
    ulong getVolumeNum();

    /**
     * @brief ��ȡ���з�����Ϣ
     * @return
     */
    QVector<DiskInfomation> getAllVolumeInfo();

    /**
     * @brief ��ȡ������ʽ
     * @param dir  ������
     * @return
     */
    FS getFileSystemType(const QString& dir);

    /**
     * @brief ��ȡ��������
     * @param dir   ������
     * @return
     */
    VOLUMETYPE getVolumeTypeItem(const QString& dir);

    /**
     * @brief ��ȡ�����ռ���Ϣ
     * @param dir                      ������
     * @param ri64FreeBytesToCaller    ���ÿռ�
     * @param ri64TotalBytes           �����ܿռ�
     * @return
     */
    bool getVolumeSpace(const QString& dir, qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes);

    /**
     * @brief ��ȡӲ�̸���
     * @return
     */
    ulong getDiskNum();

    /**
    * @brief ��ȡӲ�̿ռ���Ϣ
    * @param ri64FreeBytesToCaller    Ӳ�̿��ÿռ�
    * @param ri64TotalBytes           Ӳ���ܿռ�
    * @return
    */
    bool getDiskSpaceInfo(qulonglong& ri64FreeBytesToCaller, qulonglong& ri64TotalBytes);

    /**
    * @brief ��ȡ���з���������
    * @param volumeNameVct    ��ŷ�����������
    * @return
    */
    bool getAllVolumeName(QVector<QString> & volumeNameVct);

}

#endif // GLDDISKINFOUTILS_H
