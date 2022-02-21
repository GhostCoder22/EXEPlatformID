#include <windows.h>
#include <strsafe.h>

#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include "EXEPlatformID.h"
#include "MZImageParse/MZImageParse.h"

NTSYSAPI NTSTATUS NTAPI RtlGetVersion(IN OUT PRTL_OSVERSIONINFOW lpVersionInformation);

int FormatMessageBoxA(HWND hWnd, LPCSTR lpszCaption, UINT uType, LPCSTR lpszMessage, ...)
{
  int     result = 0;
  char	  buffer[255];
  va_list args;

  va_start(args, lpszMessage);
  result = StringCbVPrintfA(buffer, 255, lpszMessage, args);
  va_end(args);
  if (result == S_OK)
    return MessageBoxA(hWnd, buffer, lpszCaption, uType);
  SetLastError(ERROR_INVALID_PARAMETER);
  return ERROR_INVALID_PARAMETER;
}

int ShowFileError(LPCSTR szFileName)
{
  DWORD   nErrorCode = GetLastError();
  LPWSTR szError     = NULL;

  FormatMessageA(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR) &szError, 0, NULL);
  FormatMessageBoxA(NULL, "EXE Platform Identifier", MB_OK | MB_ICONEXCLAMATION,
                   "Error on file: %s\nCause: %s (%d)", szFileName, szError, nErrorCode);
  LocalFree(szError);
  return 0;
}


BOOL EPID_ShowInfo(LPCSTR lpszFileName)
{
  IMAGE_MZ_HEADERS MZHeaders;
  if (MZImageParse(&MZHeaders, lpszFileName) > 0)
    ShowFileError(lpszFileName);
  else
  {
    #define BUFFER_SIZE 500
    char buffer[BUFFER_SIZE];
    switch (MZHeaders.Type)
    {
      case IMZH_DOS: StringCbPrintfA(buffer, BUFFER_SIZE, "DOS"); break;
      case IMZH_WINDOS: StringCbPrintfA(buffer, BUFFER_SIZE, "Windows for DOS"); break;
      case IMZH_WIN32S: StringCbPrintfA(buffer, BUFFER_SIZE, "32-bit Windows Subsystem (win32s)"); break;
      case IMZH_NT:
      {
        RTL_OSVERSIONINFOW osvi;
        char architecture[10], version[50];
        ZeroMemory(&osvi, sizeof(RTL_OSVERSIONINFOW));
        osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
				RtlGetVersion(&osvi);
        switch (MZHeaders.NTHeaders.FileHeader.Machine)
        {
          case IMAGE_FILE_MACHINE_I386:  StringCbPrintfA(architecture, 10, "i386"); break;
          case IMAGE_FILE_MACHINE_IA64:  StringCbPrintfA(architecture, 10, "IA64"); break;
          case IMAGE_FILE_MACHINE_AMD64: StringCbPrintfA(architecture, 10, "AMD64"); break;
				}
        NTHeaderVersionToString(&MZHeaders.NTHeaders, version, BUFFER_SIZE);
        StringCbPrintfA(buffer, BUFFER_SIZE,
                        "Windows %d.%d.%d\nMinimum OS Required: %s (%d-bit)\nCPU Architecture: %s",
                        osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber, version,
                        MZHeaders.BitLevel, architecture);
     }
     break;
     case IMZH_NE: StringCbPrintfA(buffer, BUFFER_SIZE, "<unidentified NE signature>"); break;
     case IMZH_OS2: StringCbPrintfA(buffer, BUFFER_SIZE, "OS/2"); break;
     case IMZH_OS2_LE: StringCbPrintfA(buffer, BUFFER_SIZE, "OS/2 LE"); break;
     case IMZH_DOS4: StringCbPrintfA(buffer, BUFFER_SIZE, "European DOS 4.x"); break;
     case IMZH_BOSS: StringCbPrintfA(buffer, BUFFER_SIZE, "Borland Operating System Services (BOSS)"); break;
     case IMZH_UNKNOWN: StringCbPrintfA(buffer, BUFFER_SIZE, "Unknown"); break;
     case IMZH_ERROR: StringCbPrintfA(buffer, BUFFER_SIZE, "<Error>");
   }
   if (MZHeaders.Type != IMZH_ERROR)
   {
     if (MZHeaders.Type == IMZH_UNKNOWN)
       FormatMessageBoxA(NULL, "EXE Platform Identifier", MB_OK, "EXE File: %s\nPlatform ID: <unknown>", lpszFileName);
     else
       FormatMessageBoxA(NULL, "EXE Platform Identifier", MB_OK, "EXE File: %s\nPlatform ID: %s (%d-bit)", lpszFileName, buffer, MZHeaders.BitLevel);
   }
   return TRUE;
 }
 return FALSE;
}

BOOL EPID_BrowseFiles(HWND hWnd)
{
  OPENFILENAMEA ofn;
  char          szFileName[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner   = hWnd;
  ofn.lpstrTitle  = "Browse for EXE";
  ofn.lpstrFilter = "EXE Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrDefExt = "exe";
  ofn.nMaxFile    = MAX_PATH;
  ofn.lpstrFile   = szFileName;
  ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

  if (GetOpenFileNameA(&ofn))
    EPID_ShowInfo(ofn.lpstrFile);
  return TRUE;
}

INT_PTR CALLBACK EPID_DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDBROWSE:
          return EPID_BrowseFiles(hwndDlg);
        case IDCANCEL:
          return EndDialog(hwndDlg, IDCANCEL);
      }
    break;
    default:
      return FALSE;
  }
  return TRUE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
  DialogBoxA(hInstance, MAKEINTRESOURCEA(IDD_EPID), NULL, EPID_DialogProc);
  return 0;
}
