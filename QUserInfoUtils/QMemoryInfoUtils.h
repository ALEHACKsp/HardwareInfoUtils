#ifndef GLDMEMORYINFOUTILS_H
#define GLDMEMORYINFOUTILS_H

#include <windows.h>
#include <qglobal.h>

namespace QMemoryInfo
{

    typedef struct PhysicalMemoryInfo
    {
        qint64     m_totalPhys;     // 总内存数,单位M
        qint64     m_availPhys;     // 可用内存数,单位M
        int        m_usedPhys;      // 已用内存百分比,范围:0-100
    } PhysMemInfo;

    typedef struct VirtualMemoryInfo
    {
        qint64     m_totalVirtual;  // 总虚拟内存数,单位M
        qint64     m_availVirtual;  // 可用虚拟内存数,单位M
    } VirMemInfo;

    /**
     * @brief 获取物理内存信息
     * @return
     */
    PhysMemInfo getPhysMemInfo();

    /**
     * @brief 获取虚拟内存信息
     * @return
     */
    VirMemInfo getVirMemInfo();

}

#endif // GLDMEMORYINFOUTILS_H
