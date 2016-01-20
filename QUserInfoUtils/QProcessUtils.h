#ifndef GLDPROCESSFUNC_H
#define GLDPROCESSFUNC_H

#include <qt_windows.h>
#include <WinNT.h>

#include <QList>
#include <QString>
#include <QStringList>

namespace QProcessInfo
{

    /**
     * @brief ��ȡĳ�����̵�CPUʹ����
     * @param processID  ����PID
     * @return
     */
    ULONGLONG getCpuUsage(DWORD processID);

    /**
     * @brief ��ȡĳ�����̵�CPUʹ����
     * @param processID  ������
     * @return
     */
    ULONGLONG getCpuUsage(const QString &processName);

    /**
     * @brief ��ǰָ�����̵�ռ�õĹ�����(�ڴ�),KBΪ��Ԫ
     * @param processID  ����PID
     * @return
     */
    ulong getCurrentWorkingSetByPID(DWORD processID);

    /**
     * @brief ��ǰָ�����̵�ռ�õĹ�����(�ڴ�),KBΪ��Ԫ
     * @param processID  ������
     * @return
     */
    ulong getCurrentWorkingSetByName(const QString &processName);

    /**
     * @brief ��ǰָ�����̵�ռ�õķ�ֵ������(�ڴ�),KBΪ��Ԫ
     * @param processID  ����PID
     * @return
     */
    ulong getPeekWorkingSetByPID(DWORD processID);

    /**
     * @brief ��ǰָ�����̵�ռ�õķ�ֵ������(�ڴ�),KBΪ��Ԫ
     * @param processID  ������
     * @return
     */
    ulong getPeekWorkingSetByName(const QString &processName);

    /**
     * @brief ��ǰָ�����̵�ռ�õ�ר�ù�����(�ڴ�),KBΪ��Ԫ
     * @param processID  ����PID
     * @return
     */
    ulong getPrivateWorkingSetByPID(DWORD processID);

    /**
     * @brief ��ǰָ�����̵�ռ�õ�ר�ù�����(�ڴ�),KBΪ��Ԫ
     * @param processName  ������
     * @return
     */
    ulong getPrivateWorkingSetByName(const QString &processName);

    /**
     * @brief ��ǰָ�����̵�ռ�õĹ�������(�ڴ�),KBΪ��Ԫ
     * @param processID  ����PID
     * @return
     */
    ulong getSharedWorkingSetByPID(DWORD processID);

    /**
     * @brief ��ǰָ�����̵�ռ�õĹ�������(�ڴ�),KBΪ��Ԫ
     * @param processID  ������
     * @return
     */
    ulong getSharedWorkingSetByName(const QString &processName);

    /**
     * @brief ���ݽ���PID��ȡ������
     * @param processID  ����PID
     * @return
     */
    QString getNameByID(DWORD processID);

    /**
     * @brief ���ݽ�������ȡ����PID
     * @param processID  ������
     * @return
     */
    ulong getIDByName(const QString &processName);

    /**
     * @brief ���ݾ����ȡ��ǰ������
     * @param handle  ���
     * @return
     */
    ulong getCurrentWorkingSetByHandle(HANDLE handle);

    /**
     * @brief ���ݾ����ȡ��ֵ������
     * @param handle  ���
     * @return
     */
    ulong getPeekWorkingSetByHandle(HANDLE handle);

    /**
     * @brief ���ݽ�������ȡ���
     * @param processName  ������
     * @return
     */
    HANDLE getHandleByName(const QString &processName);

    /**
     * @brief ���ݽ���PID��ȡ���
     * @param processId  ����PID
     * @return
     */
    HANDLE getHandleByID(DWORD processId);

    /**
     * @brief ��ȡ��ǰ���̾��
     * @return
     */
    HANDLE getCurrentID();

}


#endif // GLDPROCESSFUNC_H
