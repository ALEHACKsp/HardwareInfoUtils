#include "QMemoryInfoUtils.h"

namespace QMemoryInfo
{

    PhysMemInfo getPhysMemInfo()
    {
        PhysMemInfo memInfo;
        MEMORYSTATUSEX mymem;
        mymem.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mymem);
        memset(&memInfo, 0, sizeof(memInfo));
        memInfo.m_totalPhys = (mymem.ullTotalPhys) / 1024 / 1024;
        memInfo.m_availPhys = (mymem.ullAvailPhys) / 1024 / 1024;
        memInfo.m_usedPhys = mymem.dwMemoryLoad;
        return memInfo;
    }

    VirMemInfo getVirMemInfo()
    {
        VirMemInfo memInfo;
        MEMORYSTATUSEX mymem;
        mymem.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&mymem);
        memset(&memInfo, 0, sizeof(memInfo));
        memInfo.m_totalVirtual = (mymem.ullTotalVirtual) / 1024 / 1024;
        memInfo.m_availVirtual = (mymem.ullAvailVirtual) / 1024 / 1024;
        return memInfo;
    }

}
