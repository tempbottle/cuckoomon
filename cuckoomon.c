/*
Cuckoo Sandbox - Automated Malware Analysis
Copyright (C) 2010-2012 Cuckoo Sandbox Developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <windows.h>
#include "ntapi.h"
#include "misc.h"
#include "hooking.h"
#include "hooks.h"
#include "log.h"

#define HOOK(library, funcname) {L###library, #funcname, NULL, \
    &New_##funcname, (void **) &Old_##funcname}

#define HOOK2(library, funcname, recursion) {L###library, #funcname, NULL, \
    &New2_##funcname, (void **) &Old2_##funcname, recursion}

static hook_t g_hooks[] = {

    //
    // Special Hooks
    //
    // NOTE: due to the fact that the "special" hooks don't use a hook count
    // (whereas the "normal" hooks, those with allow_hook_recursion set to
    // zero, do) we have to hook the "special" hooks first. Otherwise the
    // execution flow will end up in an infinite loop, because of hook count
    // and whatnot.
    //
    // In other words, do *NOT* place "special" hooks behind "normal" hooks.
    //

    HOOK2(ntdll, NtResumeThread, TRUE),
    HOOK2(ntdll, LdrLoadDll, TRUE),

    //
    // File Hooks
    //

    HOOK(ntdll, NtCreateFile),
    HOOK(ntdll, NtOpenFile),
    HOOK(ntdll, NtReadFile),
    HOOK(ntdll, NtWriteFile),
    HOOK(ntdll, NtDeleteFile),
    HOOK(ntdll, NtDeviceIoControlFile),
    HOOK(ntdll, NtQueryDirectoryFile),
    HOOK(ntdll, NtQueryInformationFile),
    HOOK(ntdll, NtSetInformationFile),
    HOOK(ntdll, NtCreateDirectoryObject),

    // CreateDirectoryExA calls CreateDirectoryExW
    // CreateDirectoryW does not call CreateDirectoryExW
    HOOK(kernel32, CreateDirectoryW),
    HOOK(kernel32, CreateDirectoryExW),

    HOOK(kernel32, RemoveDirectoryA),
    HOOK(kernel32, RemoveDirectoryW),

    // lowest variant of MoveFile()
    HOOK(kernel32, MoveFileWithProgressW),

    HOOK(kernel32, FindFirstFileExA),
    HOOK(kernel32, FindFirstFileExW),

    // Covered by NtCreateFile() but still grap this information
    HOOK(kernel32, CopyFileA),
    HOOK(kernel32, CopyFileW),
    HOOK(kernel32, CopyFileExW),

    // Covered by NtSetInformationFile() but still grap this information
    HOOK(kernel32, DeleteFileA),
    HOOK(kernel32, DeleteFileW),

    //
    // Registry Hooks
    //
    // Note: Most, if not all, of the Registry API go natively from both the
    // A as well as the W versions. In other words, we have to hook all the
    // ascii *and* unicode APIs of those functions.
    //

    HOOK(ntdll, NtCreateKey),
    HOOK(ntdll, NtOpenKey),
    HOOK(ntdll, NtOpenKeyEx),
    HOOK(ntdll, NtRenameKey),
    HOOK(ntdll, NtReplaceKey),
    HOOK(ntdll, NtEnumerateKey),
    HOOK(ntdll, NtEnumerateValueKey),
    HOOK(ntdll, NtSetValueKey),
    HOOK(ntdll, NtQueryValueKey),
    HOOK(ntdll, NtQueryMultipleValueKey),
    HOOK(ntdll, NtDeleteKey),
    HOOK(ntdll, NtDeleteValueKey),
    HOOK(ntdll, NtLoadKey),
    HOOK(ntdll, NtLoadKey2),
    HOOK(ntdll, NtLoadKeyEx),
    HOOK(ntdll, NtQueryKey),
    HOOK(ntdll, NtSaveKey),
    HOOK(ntdll, NtSaveKeyEx),
    HOOK(advapi32, RegQueryInfoKeyA),
    HOOK(advapi32, RegQueryInfoKeyW),

    //
    // Window Hooks
    //

    HOOK(user32, FindWindowA),
    HOOK(user32, FindWindowW),
    HOOK(user32, FindWindowExA),
    HOOK(user32, FindWindowExW),

    //
    // Sync Hooks
    //

    HOOK(ntdll, NtCreateMutant),
    HOOK(ntdll, NtOpenMutant),

    //
    // Process Hooks
    //

    HOOK(ntdll, NtCreateProcess),
    HOOK(ntdll, NtCreateProcessEx),
    HOOK(ntdll, NtOpenProcess),
    HOOK(ntdll, NtTerminateProcess),
    HOOK(ntdll, NtCreateSection),
    HOOK(ntdll, NtOpenSection),
    HOOK(kernel32, CreateProcessInternalW),
    HOOK(kernel32, ExitProcess),

    // all variants of ShellExecute end up in ShellExecuteExW
    HOOK(shell32, ShellExecuteExW),
    HOOK(ntdll, NtAllocateVirtualMemory),
    HOOK(ntdll, NtReadVirtualMemory),
    HOOK(ntdll, NtWriteVirtualMemory),
    HOOK(kernel32, VirtualProtectEx),
    HOOK(kernel32, VirtualFreeEx),

    //
    // Thread Hooks
    //

    HOOK(ntdll, NtCreateThread),
    HOOK(ntdll, NtOpenThread),
    HOOK(ntdll, NtGetContextThread),
    HOOK(ntdll, NtSetContextThread),
    HOOK(ntdll, NtSuspendThread),
    HOOK(ntdll, NtResumeThread),
    HOOK(ntdll, NtTerminateThread),
    HOOK(kernel32, CreateThread),
    HOOK(kernel32, CreateRemoteThread),
    HOOK(kernel32, ExitThread),

    //
    // Misc Hooks
    //

    HOOK(user32, SetWindowsHookExA),
    HOOK(user32, SetWindowsHookExW),
    HOOK(user32, UnhookWindowsHookEx),
    HOOK(ntdll, LdrLoadDll),
    HOOK(ntdll, LdrGetDllHandle),
    HOOK(ntdll, LdrGetProcedureAddress),
    HOOK(kernel32, DeviceIoControl),
    HOOK(ntdll, NtDelayExecution),
    HOOK(user32, ExitWindowsEx),
    HOOK(kernel32, IsDebuggerPresent),
    HOOK(advapi32, LookupPrivilegeValueW),
    HOOK(ntdll, NtClose),

    //
    // Network Hooks
    //

    HOOK(urlmon, URLDownloadToFileW),
    HOOK(wininet, InternetOpenUrlA),
    HOOK(wininet, InternetOpenUrlW),
    HOOK(wininet, HttpOpenRequestA),
    HOOK(wininet, HttpOpenRequestW),
    HOOK(wininet, HttpSendRequestA),
    HOOK(wininet, HttpSendRequestW),

    HOOK(dnsapi, DnsQuery_A),
    HOOK(dnsapi, DnsQuery_UTF8),
    HOOK(dnsapi, DnsQuery_W),
    HOOK(ws2_32, getaddrinfo),
    HOOK(ws2_32, GetAddrInfoW),

    //
    // Service Hooks
    //

    HOOK(advapi32, OpenSCManagerA),
    HOOK(advapi32, OpenSCManagerW),
    HOOK(advapi32, CreateServiceA),
    HOOK(advapi32, CreateServiceW),
    HOOK(advapi32, OpenServiceA),
    HOOK(advapi32, OpenServiceW),
    HOOK(advapi32, StartServiceA),
    HOOK(advapi32, StartServiceW),
    HOOK(advapi32, ControlService),
    HOOK(advapi32, DeleteService),
};

// get a random hooking method, except for hook_jmp_direct
#define HOOKTYPE randint(HOOK_JMP_DIRECT + 1, HOOK_TECHNIQUE_MAXTYPE - 1)

void set_hooks_dll(const wchar_t *library, int len)
{
    for (int i = 0; i < ARRAYSIZE(g_hooks); i++) {
        if(!wcsnicmp(g_hooks[i].library, library, len)) {
            hook_api(&g_hooks[i], HOOKTYPE);
        }
    }
}

void set_hooks()
{
    // the hooks contain the gates as well, so they have to be RWX
    DWORD old_protect;
    VirtualProtect(g_hooks, sizeof(g_hooks), PAGE_EXECUTE_READWRITE,
        &old_protect);

    hook_disable();

    // now, hook each api :)
    for (int i = 0; i < ARRAYSIZE(g_hooks); i++) {
        hook_api(&g_hooks[i], HOOKTYPE);
    }

    hook_enable();
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    if(dwReason == DLL_PROCESS_ATTACH) {
        // make sure advapi32 is loaded
        LoadLibrary("advapi32");

        log_init();
        set_hooks();
    }
    else if(dwReason == DLL_PROCESS_DETACH) {
        log_free();
    }

    return TRUE;
}
