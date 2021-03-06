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

//
// Log API
//
// The Log takes a format string and parses the extra arguments accordingly
//
// The following Format Specifiers are available:
// s  -> (char *) -> zero-terminated string
// S  -> (int, char *) -> string with length
// u  -> (wchar_t *) -> zero-terminated unicode string
// U  -> (int, wchar_t *) -> unicode string with length
// b  -> (int, void *) -> memory with a given size (alias for S)
// B  -> (int *, void *) -> memory with a given size (value at integer)
// i  -> (int) -> integer
// l  -> (long) -> long integer
// L  -> (long *) -> pointer to a long integer
// p  -> (void *) -> pointer (alias for l)
// P  -> (void **) -> pointer to a handle (alias for L)
// o  -> (UNICODE_STRING *) -> unicode string
// O  -> (OBJECT_ATTRIBUTES *) -> wrapper around a unicode string
// a  -> (int, char **) -> array of string
// A  -> (int, wchar_t **) -> array of unicode strings
//
// Each of these format specifiers are prefixed with a zero-terminated key
// value, e.g.
//
// log("s", "key", "value");
//
// A format specifier can also be repeated for n times (with n in the range
// 2..9), e.g.
//
// loq("sss", "key1", "value", "key2", "value2", "key3", "value3");
// loq("3s", "key1", "value", "key2", "value2", "key3", "value3");
//

void loq(const char *fmt, ...);

void log_init();
void log_free();

#define LOQ(fmt, ...) loq(fmt, module_name, &__FUNCTION__[4], \
    is_success(ret), ret, ##__VA_ARGS__)

#define IS_SUCCESS_NTSTATUS() int is_success(NTSTATUS ret) { \
    return NT_SUCCESS(ret); }
#define IS_SUCCESS_BOOL() int is_success(BOOL ret) { \
    return ret != FALSE; }
#define IS_SUCCESS_HHOOK() int is_success(HHOOK ret) { \
    return ret != NULL; }
#define IS_SUCCESS_HINTERNET() int is_success(HINTERNET ret) { \
    return ret != NULL; }
#define IS_SUCCESS_HRESULT() int is_success(HRESULT ret) { \
    return ret == S_OK; }
#define IS_SUCCESS_HANDLE() int is_success(HANDLE ret) { \
    return ret != NULL; }
#define IS_SUCCESS_HANDLE2() int is_success(HANDLE ret) { \
    return ret != NULL && ret != INVALID_HANDLE_VALUE; }
#define IS_SUCCESS_VOID() int is_success(int ret) { return TRUE; }
#define IS_SUCCESS_LONGREG() int is_success(LONG ret) { \
    return ret == ERROR_SUCCESS; }
#define IS_SUCCESS_SCHANDLE() int is_success(SC_HANDLE ret) { \
    return ret != NULL; }
#define IS_SUCCESS_DWORDTHREAD() int is_success(DWORD ret) { \
    return ret != (DWORD) -1; }
#define IS_SUCCESS_HWND() int is_success(HWND ret) { \
    return ret != NULL; }
#define IS_SUCCESS_ZERO() int is_success(int ret) { \
    return ret == 0; }

#define ENSURE_ULONG(param) \
    ULONG _##param = 0; if(param == NULL) param = &_##param;
