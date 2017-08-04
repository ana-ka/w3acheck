#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <map>
#include <stdio.h>
#include <Wininet.h>
#include <tinyxml2.h>
#include <chrono>




using namespace std;


// global variables
string szWC3Path = "";
string szWC3FileName = "";

int check_NET_framwork();
int check_wc3(string &szWC3Path, string &szWC3FileName);
int check_w3a();
int check_DEP_status(string szWC3Path, string szWC3FileName);
int check_connection();
int check_vm();
int check_sandbox();

// result variables:
// -1 = no info
// 0 = all okay
// 1 = warning
// 2 = error

int nNETStatus = -1;
int nWC3 = -1;
int nDEPSystem = -1;
int nDEPWC3 = -1;
int nW3A = -1;
int nConnection = -1;
int nVM = -1;
int nSandBox = -1;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
// console color
#define normal 7
#define green 10
#define w3ablue 11
#define red 12
#define yellow 14
#define white 15


int main()
{

    system("CLS");
    SetConsoleTextAttribute(hConsole, 139);
    cout << "W3A";
    SetConsoleTextAttribute(hConsole, 143);
    cout << "ssist" << endl;
    SetConsoleTextAttribute(hConsole, normal);
    cout << "--------" << endl;
    cout << "checking .NET framework.." << endl;
    nNETStatus = check_NET_framwork();
    cout << endl;
    cout << "checking WC3.." << endl;
    nWC3 = check_wc3(szWC3Path, szWC3FileName);
    cout << endl;
    cout << "checkin W3A.." << endl;
    //nW3A = check_w3a();
    cout << endl;
    cout << "checking DEP.." << endl;
    check_DEP_status(szWC3Path, szWC3FileName);
    cout << endl;
    cout << "checking internet connection.." << endl;
    nConnection = check_connection();
    cout << endl;
    cout << "checking VM status..";
    nVM = check_vm();
    cout << endl;
    cout << "checking sandbox..";
    nSandBox = check_sandbox();
    cout << endl;
    CloseHandle(hConsole);
    cin.get();
    return 0;
}

int check_NET_framwork()
{
    HKEY hk;
    //.NET registry keys
    map <string,char*> mNetKeys = { {"version 1", (char*)"Software\\Microsoft\\.NETFramework\\Policy\\v1.0"},
                                    {"version 1.1", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v1.1.4322"},
                                    {"version 2", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v2.0.50727"},
                                    {"version 3", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v3.0"},
                                    {"version 3.5", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v3.5"},
                                    {"version 4 c", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Client"},
                                    {"version 4 f", (char*)"Software\\Microsoft\\NET Framework Setup\\NDP\\v4\\Full"}
    };
    //iterate over keys and check install value
    for (map<string,char*>::iterator it = mNetKeys.begin(); it != mNetKeys.end(); ++it)
    {
        std::cout << "\t" << it->first << ":" << "\t";
        char* cKey = it->second;
        long lErrorK = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKey, 0, KEY_QUERY_VALUE, &hk);
        if (lErrorK == ERROR_SUCCESS)
        {
                DWORD dwType;
                byte byData[4];
                DWORD dwSize = sizeof(byData);
                long lErrorV = RegQueryValueEx(hk, TEXT("Install"), NULL, &dwType, byData, &dwSize);
                if (lErrorV == ERROR_SUCCESS)
                {
                    if (*((unsigned long*)byData) == 1)
                    {
                        SetConsoleTextAttribute(hConsole, green);
                        cout << "pass" << endl;
                        SetConsoleTextAttribute(hConsole, normal);
                    }
                }
            else
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
            }
        }
        else
        {
            SetConsoleTextAttribute(hConsole, red);
            cout << "fail" << endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
    }
    RegCloseKey(hk);
    return 0;
}

int check_wc3(string &szWC3Path, string &szWC3FileName)
{
    HKEY hk;
    char* cKey = (char*)"Software\\Blizzard Entertainment\\Warcraft III";
    long lErrorK = RegOpenKeyEx(HKEY_CURRENT_USER, cKey, 0, KEY_QUERY_VALUE, &hk);
    if (lErrorK == ERROR_SUCCESS)
    {
        DWORD dwType;
        byte byData[1024];
        DWORD dwSize = sizeof(byData);
        long lErrorV = RegQueryValueEx(hk, TEXT("InstallPath"), NULL, &dwType, (LPBYTE)byData, &dwSize);
        szWC3Path = (reinterpret_cast< char const* >(byData));
        cout << "\t" << "path: " << szWC3Path;
        if (lErrorV == ERROR_SUCCESS)
        {
            size_t special = szWC3Path.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ: /\\()");
            if (special == string::npos)
            {
                SetConsoleTextAttribute(hConsole, green);
                cout << "\t" << "pass" << endl;
                SetConsoleTextAttribute(hConsole, normal);
            }
            else
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "\t" << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
            }

            //Get WC3 Version Info
            cout << "\t" << "version:" << "\t";
            szWC3FileName = (szWC3Path += "\\war3.exe").c_str();

            DWORD dwHandle, sz = GetFileVersionInfoSizeA( szWC3FileName.c_str(), & dwHandle );
            if ( 0 == sz )
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "\t" << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
                return 2;
            }
            char *buf = new char[sz];
            if ( !GetFileVersionInfoA( szWC3FileName.c_str(), dwHandle, sz, & buf[ 0 ] ) )
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "\t" << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
                delete buf;
                return 2;
            }
            VS_FIXEDFILEINFO * pvi;


            sz = sizeof( VS_FIXEDFILEINFO );
            if ( !VerQueryValueA( & buf[ 0 ], "\\", (LPVOID*)&pvi, (unsigned int*)&sz ) )
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "\t" << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
                delete buf;
                return 2;
            }
            cout << HIWORD(pvi->dwProductVersionMS);
            cout << ".";
            cout << LOWORD(pvi->dwFileVersionMS);
            cout << ".";
            cout << HIWORD(pvi->dwFileVersionLS);
            cout << ".";
            cout << LOWORD(pvi->dwFileVersionLS);
            cout << "\t";

            if (HIWORD(pvi->dwProductVersionMS) == 1 && LOWORD(pvi->dwFileVersionMS) == 26
                && HIWORD(pvi->dwFileVersionLS) == 0 && LOWORD(pvi->dwFileVersionLS) == 6401)
            {
                SetConsoleTextAttribute(hConsole, green);
                cout << "\t" << "pass" << endl;
                SetConsoleTextAttribute(hConsole, normal);
                delete buf;
                return 0;
            }
            else
            {
                SetConsoleTextAttribute(hConsole, red);
                cout << "\t" << "fail" << endl;
                SetConsoleTextAttribute(hConsole, normal);
                RegCloseKey(hk);
                return 2;
                delete buf;
            }
            delete buf;

        }
        else
        {
            SetConsoleTextAttribute(hConsole, red);
            cout << "\t" << "fail" << endl;
            SetConsoleTextAttribute(hConsole, normal);
            return 2;
            RegCloseKey(hk);
        }
    }
    else
    {
        cout << "\t" << "path not found";
        SetConsoleTextAttribute(hConsole, red);
        cout << "\t" << "fail" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        RegCloseKey(hk);
        return 2;
    }
    RegCloseKey(hk);
    return 0;
}

/*
int check_w3a()
{
    char* szPrograms = NULL;
    cout << GetEnvironmentVariable(TEXT("SYSTEM"), szPrograms, sizeof(szPrograms)) << endl;
    cout << "test" << endl;
    return 0;
}
*/

int check_DEP_status(string szWC3Path, string szWC3FileName)
{
    //check system DEP policy
    typedef DWORD (WINAPI * GetSystemDEPPolicy_t)(void);
    HMODULE hKernel = LoadLibrary((LPCSTR)"kernel32.dll");
    GetSystemDEPPolicy_t pfnGetSystemDEPPolicy = (GetSystemDEPPolicy_t)GetProcAddress(hKernel, "GetSystemDEPPolicy");
    int deptype = (int)pfnGetSystemDEPPolicy();
    cout << "\t" << "system:";
    if (deptype == 1)
    {
        cout << "\t" << "AlwaysOn" << "\t";
        SetConsoleTextAttribute(hConsole, red);
        cout << "fail" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        //return 2;
    }
    if (deptype == 0)
    {
        cout << "\t" << "AlwaysOff" << "\t";
        SetConsoleTextAttribute(hConsole, green);
        cout << "pass" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        //return 0;
    }
    if (deptype == 2)
    {
        cout << "\t" << "OptIn" << "\t";
        SetConsoleTextAttribute(hConsole, yellow);
        cout << "caution" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        //return 1;
    }
    if (deptype ==3)
    {
        cout << "\t" << "OptOut" << "\t";
        SetConsoleTextAttribute(hConsole, yellow);
        cout << "caution" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        //return 1;
    }
    //return 0;

    //check WC3 DEP-status
    cout << "\t" << "WC3: " << "\t";


    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HMODULE hProcess;
    DWORD lpFlags = NULL;
    BOOL lpPermanent = NULL;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    CreateProcess( NULL,   // No module name (use command line)
        (LPSTR)szWC3FileName.c_str(),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi );           // Pointer to PROCESS_INFORMATION structure


    typedef int (__cdecl *MYPROC)(HANDLE, LPDWORD, PBOOL);
    MYPROC ProcAdd;

    if (hKernel != NULL)
    {
        ProcAdd = (MYPROC) GetProcAddress(hKernel,"GetProcessDEPPolicy");
        if (ProcAdd != NULL)
        {
            if (pi.hProcess != NULL)
            {
                (ProcAdd) (pi.hProcess, &lpFlags, &lpPermanent);
                if ((int)lpFlags == 0)
                {
                    cout << "disabled" << "\t";
                    SetConsoleTextAttribute(hConsole, green);
                    cout << "pass" << endl;
                    SetConsoleTextAttribute(hConsole, normal);
                    TerminateProcess(pi.hProcess, 0);
                    return 0;

                }
                else
                {
                    cout << "enabled" << "\t";
                    SetConsoleTextAttribute(hConsole, red);
                    cout << "fail" << endl;
                    SetConsoleTextAttribute(hConsole, normal);
                    TerminateProcess(pi.hProcess, 0);
                    return 2;
                }

            }
        }
    }
    else
    {
        cout << "n/a" << "\t" << endl;
        SetConsoleTextAttribute(hConsole, yellow);
        cout << "fail" << endl;
        SetConsoleTextAttribute(hConsole, normal);
        TerminateProcess(pi.hProcess, 0);
        return 1;
    }

    /*
    typedef BOOL (WINAPI * GetProcessDEPPolicy_t)(HANDLE, LPDWORD, PBOOL);
    GetProcessDEPPolicy_t pfnGetProcessDEPPolicy = (GetProcessDEPPolicy_t)GetProcAddress(hKernel, "GetProcessDEPPolicy");
    cout << "we are here" << endl;
    cout << pfnGetProcessDEPPolicy(pi.hProcess, lpFlags, lpPermanent);


    //GetProcessDEPPolicy(pi.hProcess, lpFlags, lpPermanent);

    TerminateProcess(pi.hProcess, 0);
    return 0;

    //GetProcessDEPPolicy(pi.hProcess, lpFlags, lpPermanent);
    */
}


int check_connection()

{
    cout << "\t" << "google:" << "\t";
    auto start_time = std::chrono::high_resolution_clock::now();
    if(InternetCheckConnection((LPCSTR)"http://www.google.com",1,0))
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
        SetConsoleTextAttribute(hConsole, green);
        cout << "pass ";
        if (time <= 100) {
            SetConsoleTextAttribute(hConsole, green);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
        if ((100 < time) && (time <= 250)){
            SetConsoleTextAttribute(hConsole, yellow);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
        if (time >250) {
            SetConsoleTextAttribute(hConsole, red);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }

    }
    else
    {
        SetConsoleTextAttribute(hConsole, red);
        cout << "fail" << endl;
        SetConsoleTextAttribute(hConsole, normal);
    }

    cout << "\t" << "W3A:" << "\t";
    start_time = std::chrono::high_resolution_clock::now();
    if(InternetCheckConnection((LPCSTR)"http://login.w3arena.net",1,0))
        {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
        SetConsoleTextAttribute(hConsole, green);
        cout << "pass ";
        if (time <= 100) {
            SetConsoleTextAttribute(hConsole, green);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
        if ((100 < time) && (time <= 250)){
            SetConsoleTextAttribute(hConsole, yellow);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
        if (time >250) {
            SetConsoleTextAttribute(hConsole, red);
            std::cout << "(" << time << "ms)" << std::endl;
            SetConsoleTextAttribute(hConsole, normal);
        }

    }
    else
    {
        SetConsoleTextAttribute(hConsole, red);
        cout << "fail" << endl;
        SetConsoleTextAttribute(hConsole, normal);
    }

    return 0;
}

int check_vm()
{
    int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
    {
        if(CreateFile("\\\\.\\VBoxMiniRdrDN",GENERIC_READ,FILE_SHARE_READ,
            NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL) != INVALID_HANDLE_VALUE)
        {
            SetConsoleTextAttribute(hConsole, red);
            cout << "\t" << "virtual machine detected" << endl;
            SetConsoleTextAttribute(hConsole, normal);
        }

        else
        {
            SetConsoleTextAttribute(hConsole, green);
            cout << "\t" << "pass" << endl;
            SetConsoleTextAttribute(hConsole, normal);
        }
    }
    return 0;
}

int check_sandbox()
{
    bool bSandboxed = false;
    char* sModule[] = { (char*)"SbieDll.dll", (char*)"dbghelp.dll" };
    for( int i = 0; i < ( sizeof( sModule ) / sizeof( char* ) ); i++ )
    {
        if( GetModuleHandle( sModule[ i ] ) )
        {
            bSandboxed = true;
        }
    }
    if (bSandboxed)
    {
        SetConsoleTextAttribute(hConsole, red);
        cout << "\t" << "sandbox detected" << endl;
        SetConsoleTextAttribute(hConsole, normal);
    }
    else
    {
        SetConsoleTextAttribute(hConsole, green);
        cout << "\t" << "pass" << endl;
        SetConsoleTextAttribute(hConsole, normal);
    }
    return 0;
}
