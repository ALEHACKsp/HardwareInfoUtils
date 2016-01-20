#ifndef GLDCPUINFOUTILS_H
#define GLDCPUINFOUTILS_H

#include <windows.h>
#include <QString>

namespace QCpuInfo
{

    typedef struct CpuCoreCount
    {
        CpuCoreCount()
            : m_cpuLogicalCoreCount(-1)
            , m_cpuCoreCount(-1)
        {

        }

        int m_cpuLogicalCoreCount;   // CPU核心数
        int m_cpuCoreCount;          // 逻辑CPU数
    } CoreCount;

    struct CpuInfo
    {
        QString   m_cpuBrand;              // CPU商标
        int       m_processorPackageCount; // 物理cpu个数
        CoreCount m_coreCount;             // 核数
        ulong     m_processSpeed;          // cpu最大速度(单位MHZ)
    };

    /**
     * @brief 获取CPU商标
     * @return
     */
    QString getCpuBrand();

    /**
     * @brief 获取CPU速度,即频率
     * @return
     */
    ulong getCpuSpeed();

    /**
     * @brief 获取CPU物理核数
     * @return
     */
    ulong getCpuCount();

    /**
     * @brief 获取CPU核数,包括物理核数和逻辑核数
     * @return
     */
    CoreCount getCpuCoreCount();

    /**
     * @brief 获取CPU信息
     * @return
     */
    CpuInfo getCpuInfo();

}
#endif // GLDCPUINFOUTILS_H
