// SystemVersion.cpp : Defines the entry point for the console application.
//

#include "SystemInfo.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    SystemInfo sysInfo;
    TCHAR szServicePack[128] = { 0 };

    switch (sysInfo.getWindowsVersion())
    {
    case Windows:
        cout << "Windows" << endl;
        break;

    case Windows32s:
        cout << "Windows 32s" << endl;
        break;

    case Windows95:
        cout << "Windows 95" << endl;
        break;

    case Windows95OSR2:
        cout << "Windows 95 SR2" << endl;
        break;

    case Windows98:
        cout << "Windows 98" << endl;
        break;

    case Windows98SE:
        cout << "Windows 98 SE" << endl;
        break;

    case WindowsMillennium:
        cout << "Windows Me" << endl;
        break;

    case WindowsNT351:
        cout << "Windows NT 3.51" << endl;
        break;

    case WindowsNT40:
        cout << "Windows NT 4.0" << endl;
        break;

    case WindowsNT40Server:
        cout << "Windows NT 4.0 Server" << endl;
        break;

    case Windows2000:
        cout << "Windows 2000" << endl;
        break;

    case WindowsXP:
        cout << "Windows XP" << endl;
        break;

    case WindowsXPProfessionalx64:
        cout << "Windows XP Professional x64" << endl;
        break;

    case WindowsHomeServer:
        cout << "Windows Home Server" << endl;
        break;

    case WindowsServer2003:
        cout << "Windows Server 2003" << endl;
        break;

    case WindowsServer2003R2:
        cout << "Windows Server 2003 R2" << endl;
        break;

    case WindowsVista:
        cout << "Windows Vista" << endl;
        break;

    case WindowsServer2008:
        cout << "Windows Server 2008" << endl;
        break;

    case WindowsServer2008R2:
        cout << "Windows Server 2008 R2" << endl;
        break;

    case Windows7:
        cout << "Windows 7" << endl;
        break;
    case Windows8Point1:
        cout << "Windows 8.1" << endl;

    case Windows10:
        cout << "Windows 10 ";
        break;

    case WindowsServer2016TechnicalPreview:
        cout << "Windows 10 Server" << endl;
        break;
    }

    switch (sysInfo.getWindowsEdition())
    {
    case EditionUnknown:
        cout << "Edition unknown Edition" << endl;
        break;

    case Workstation:
        cout << "Workstation Edition" << endl;
        break;

    case Server:
        cout << "Server Edition" << endl;
        break;

    case AdvancedServer:
        cout << "Advanced Server Edition" << endl;
        break;

    case Home:
        cout << "Home Edition" << endl;
        break;

    case Ultimate:
        cout << "Ultimate Edition" << endl;
        break;

    case HomeBasic:
        cout << "Home Basic Edition" << endl;
        break;

    case HomePremium:
        cout << "Home Premium Edition" << endl;
        break;

    case Enterprise:
        cout << "Enterprise Edition" << endl;
        break;

    case Professional:
        cout << "Professional Edition" << endl;
        break;

    case HomeBasic_N:
        cout << "Home Basic N Edition" << endl;
        break;

    case Business:
        cout << "Business Edition" << endl;
        break;

    case StandardServer:
        cout << "Standard Server Edition" << endl;
        break;

    case EnterpriseServerCore:
        cout << "Enterprise Server Core Edition" << endl;
        break;

    case EnterpriseServerIA64:
        cout << "Enterprise Server IA64 Edition" << endl;
        break;

    case Business_N:
        cout << "Business N Edition" << endl;
        break;

    case WebServer:
        cout << "Web Server Edition" << endl;
        break;

    case ClusterServer:
        cout << "Cluster Server Edition" << endl;
        break;

    case HomeServer:
        cout << "Home Server Edition" << endl;
        break;
    }

    cout << "Platform type: ";
    if (sysInfo.isNTPlatform())
    {
        cout << "NT" << endl;
    }
    else if (sysInfo.isWindowsPlatform())
    {
        cout << "Windows" << endl;
    }
    else if (sysInfo.isWin32sPlatform())
    {
        cout << "Win32s" << endl;
    }
    else
    {
        cout << "Unknown" << endl;
    }

    cout << "Major version: " << sysInfo.getMajorVersion() << endl;
    cout << "Minor version: " << sysInfo.getMinorVersion() << endl;
    cout << "Build number: " << sysInfo.getBuildNumber() << endl;

    sysInfo.getServicePackInfo(szServicePack);
    cout << "Service Pack info: " << szServicePack << endl;

    cout << "32-bit platform: " << std::boolalpha << sysInfo.is32bitPlatform() << endl;
    cout << "64-bit platform: " << std::boolalpha << sysInfo.is64bitPlatform() << endl;

    std::string softWareName1 = "360云盘";
    std::string softWareName2 = "Microsoft Visual Studio Professional 2015 - 简体中文"; 
    std::string softWareName = "支付宝安全控件 5.3.0.3807";
    cout << checkIsInstalled(softWareName);

    return 0;
}
