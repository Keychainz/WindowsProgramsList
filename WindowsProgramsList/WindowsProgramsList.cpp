#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <string>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

// Define the struct to store program information
struct ProgramInfo {
    std::wstring name;
    std::wstring installLocation;
    std::wstring publisher;
    bool isImportant;
};

// Function to determine if a program is important
/*This function checks the programs install location and publishers name to determine if its worth keeping or important*/
bool IsImportantProgram(const std::wstring& installLocation, const std::wstring& publisher) {
    return (installLocation.find(L"C:\\Windows\\") != std::wstring::npos ||
        installLocation.find(L"C:\\Program Files\\") != std::wstring::npos ||
        publisher.find(L"Microsoft") != std::wstring::npos);
}

void QueryInstalledPrograms(HKEY hKey, std::vector<ProgramInfo>& programsList)
{
    TCHAR achKey[MAX_KEY_LENGTH];   // buffer for subkey name
    DWORD cbName = 0;               // size of name string 
    FILETIME ftLastWriteTime;       // last write time 

    DWORD i = 0, retCode = 0;

    // Get the number of subkeys.
    DWORD cSubKeys = 0;
    RegQueryInfoKey(hKey, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    // Enumerate the subkeys
    if (cSubKeys)
    {
        for (i = 0; i < cSubKeys; i++)
        {
            cbName = MAX_KEY_LENGTH;
            retCode = RegEnumKeyEx(hKey, i,
                achKey,
                &cbName,
                NULL,
                NULL,
                NULL,
                &ftLastWriteTime);

            if (retCode == ERROR_SUCCESS)
            {
                HKEY hSubKey;
                if (RegOpenKeyEx(hKey, achKey, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
                {
                    ProgramInfo program;

                    // Get the DisplayName
                    TCHAR achValue[MAX_VALUE_NAME] = TEXT("");
                    DWORD cchValue = MAX_VALUE_NAME;
                    DWORD dwType = 0;

                    if (RegQueryValueEx(hSubKey, TEXT("DisplayName"), NULL, &dwType, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS)
                    {
                        program.name = achValue;
                    }

                    // Get the Publisher
                    cchValue = MAX_VALUE_NAME;
                    if (RegQueryValueEx(hSubKey, TEXT("Publisher"), NULL, &dwType, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS)
                    {
                        program.publisher = achValue;
                    }

                    // Get the InstallLocation
                    cchValue = MAX_VALUE_NAME;
                    if (RegQueryValueEx(hSubKey, TEXT("InstallLocation"), NULL, &dwType, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS)
                    {
                        program.installLocation = achValue;
                    }

                    // Determine if the program is important
                    program.isImportant = IsImportantProgram(program.installLocation, program.publisher);

                    // Add the program to the list
                    programsList.push_back(program);

                    RegCloseKey(hSubKey);
                }
            }
        }
    }
}

int main()
{
    HKEY hUninstallKey = NULL;
    std::vector<ProgramInfo> programsList;

    // Query installed programs for both HKLM and HKCU
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
        0,
        KEY_READ,
        &hUninstallKey) == ERROR_SUCCESS)
    {
        QueryInstalledPrograms(hUninstallKey, programsList);
        RegCloseKey(hUninstallKey);
    }

    if (RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
        0,
        KEY_READ,
        &hUninstallKey) == ERROR_SUCCESS)
    {
        QueryInstalledPrograms(hUninstallKey, programsList);
        RegCloseKey(hUninstallKey);
    }

    // Print out the information stored in the struct
    for (const auto& program : programsList)
    {
        wprintf(L"Program: %s\n", program.name.c_str());
        wprintf(L"Publisher: %s\n", program.publisher.c_str());
        wprintf(L"Install Location: %s\n", program.installLocation.c_str());
        wprintf(L"Important: %s\n\n", program.isImportant ? L"Yes" : L"No");
    }

    return 0;
}
