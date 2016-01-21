/*
* copyright (c) 2005 - 2010 Marius Bancila
* http://www.mariusbancila.ro
* http://www.mariusbancila.ro/blog
* http://www.codexpert.ro
* http://www.codeguru.com
* http://www.sharparena.com
*/

//////////////////////////////////////////////////////////////////////
// SystemInfo.h: interface for the SystemInfo class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include <Windows.h>
#include <string>

bool checkIsInstalled(std::string & name);

typedef enum WindowsVersion
{
    Windows,                  // windows
    Windows32s,               // 
    Windows95,
    Windows95OSR2,
    Windows98,
    Windows98SE,
    WindowsMillennium,
    WindowsNT351,
    WindowsNT40,
    WindowsNT40Server,
    Windows2000,
    WindowsXP,
    WindowsXPProfessionalx64,
    WindowsHomeServer,
    WindowsServer2003,
    WindowsServer2003R2,
    WindowsVista,
    WindowsServer2008,
    WindowsServer2008R2,
    Windows7,
    Windows8,
    WindowsServer2012,
    Windows8Point1,
    WindowsServer2012R2,
    Windows10,
    WindowsServer2016TechnicalPreview
};

typedef enum WindowsEdition
{
    EditionUnknown,

    Workstation,
    Server,
    AdvancedServer,
    Home,

    Ultimate,
    HomeBasic,
    HomePremium,
    Enterprise,
    HomeBasic_N,
    Business,
    StandardServer,
    DatacenterServer,
    SmallBusinessServer,
    EnterpriseServer,
    Starter,
    DatacenterServerCore,
    StandardServerCore,
    EnterpriseServerCore,
    EnterpriseServerIA64,
    Business_N,
    WebServer,
    ClusterServer,
    HomeServer,
    StorageExpressServer,
    StorageStandardServer,
    StorageWorkgroupServer,
    StorageEnterpriseServer,
    ServerForSmallBusiness,
    SmallBusinessServerPremium,
    HomePremium_N,
    Enterprise_N,
    Ultimate_N,
    WebServerCore,
    MediumBusinessServerManagement,
    MediumBusinessServerSecurity,
    MediumBusinessServerMessaging,
    ServerFoundation,
    HomePremiumServer,
    ServerForSmallBusiness_V,
    StandardServer_V,
    DatacenterServer_V,
    EnterpriseServer_V,
    DatacenterServerCore_V,
    StandardServerCore_V,
    EnterpriseServerCore_V,
    HyperV,
    StorageExpressServerCore,
    StorageStandardServerCore,
    StorageWorkgroupServerCore,
    StorageEnterpriseServerCore,
    Starter_N,
    Professional,
    Professional_N,
    SBSolutionServer,
    ServerForSBSolution,
    StandardServerSolutions,
    StandardServerSolutionsCore,
    SBSolutionServer_EM,
    ServerForSBSolution_EM,
    SolutionEmbeddedServer,
    SolutionEmbeddedServerCore,
    SmallBusinessServerPremiumCore,
    EssentialBusinessServerMGMT,
    EssentialBusinessServerADDL,
    EssentialBusinessServerMGMTSVC,
    EssentialBusinessServerADDLSVC,
    ClusterServer_V,
    Embedded,
    Starter_E,
    HomeBasic_E,
    HomePremium_E,
    Professional_E,
    Enterprise_E,
    Ultimate_E
};

class SystemInfo
{
public:
    SystemInfo();
    virtual ~SystemInfo();

    /**
     * @brief returns the windows version
     * @return
     */
    WindowsVersion getWindowsVersion() const;

    /**
     * @brief returns the windows edition
     * @return
     */
    WindowsEdition getWindowsEdition() const;

    /**
     * @brief true if NT platform
     * @return
     */
    bool isNTPlatform() const;

    /**
     * @brief true is Windows platform
     * @return
     */
    bool isWindowsPlatform() const;

    /**
     * @brief true is Win32s platform
     * @return
     */
    bool isWin32sPlatform() const;

    /**
     * @brief returns major version
     * @return
     */
    DWORD getMajorVersion() const;

    /**
     * @brief returns minor version
     * @return
     */
    DWORD getMinorVersion() const;

    /**
     * @brief returns build number
     * @return
     */
    DWORD getBuildNumber() const;

    /**
     * @brief returns platform ID
     * @return
     */
    DWORD getPlatformID() const;

    /**
     * @brief additional information about service pack
     * @param szServicePack
     */
    void getServicePackInfo(TCHAR* szServicePack) const;

    /**
     * @brief true if platform is 32-bit
     * @return
     */
    bool is32bitPlatform() const;

    /**
     * @brief true if platform is 64-bit
     * @return
     */
    bool is64bitPlatform() const;

private:
    void detectWindowsVersion();
    void detectWindowsEdition();
    void detectWindowsServicePack();
    DWORD detectProductInfo();
    bool getWinMajorMinorVersion(DWORD& major, DWORD& minor);

private:
    WindowsVersion     m_nWinVersion;
    WindowsEdition     m_nWinEdition;        // windows°æ±¾
    TCHAR              m_szServicePack[128];
    OSVERSIONINFOEX    m_osvi;
    SYSTEM_INFO        m_SysInfo;
    BOOL               m_bOsVersionInfoEx;
};
