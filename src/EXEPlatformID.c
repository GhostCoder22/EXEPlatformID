#include <windows.h>
#include <strsafe.h>

#include <stdio.h>
#include <stdarg.h>

#include "EXEPlatformID.h"

int FormatMessageBoxW(HWND hWnd, LPCWSTR lpszCaption, UINT uType, LPCWSTR lpszMessage, ...)
{
  int      result = 0;
  wchar_t  buffer[255];
  va_list  args;

  va_start(args, lpszMessage);
  if ((result = StringCbVPrintfW(buffer, 255, lpszMessage, args) != S_OK))
  { va_end(args); result = -1; }
  else
  {
    result = MessageBoxW(hWnd, buffer, lpszCaption, uType);
    va_end(args);
  }
  return result;
}

int ShowFileError(LPCWSTR szFileName)
{
  DWORD   nErrorCode = GetLastError();
  LPWSTR szError     = NULL;

  FormatMessageW(FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, nErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR) &szError, 0, NULL);
  FormatMessageBoxW(NULL, L"EXE Platform Identifier", MB_OK | MB_ICONEXCLAMATION,
                   L"Error on file: %s\nCause: %s (%d)", szFileName, szError, nErrorCode);
  LocalFree(szError);
  return 0;
}

BOOL DoShowEXEPlatformID(HWND hWnd, LPCWSTR lpszFileName)
{
  BOOL result = FALSE;

  HANDLE hFile = CreateFileW(lpszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
		goto showError;
	else
	{
		HANDLE hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hFileMapping == 0)
		{
			CloseHandle(hFile);
			goto showError;
		}
		else
		{
			LPVOID lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
			if (lpFileBase == NULL)
			{
				CloseHandle(hFileMapping); CloseHandle(hFile);
				goto showError;
			}
		  else
			{
			  #define BUFFER_SIZE 500
			  PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER) lpFileBase;
				if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
			  {
			  	PIMAGE_NT_HEADERS  peHeader = (PIMAGE_NT_HEADERS) ((UCHAR*) dosHeader + dosHeader->e_lfanew);
			  	if (peHeader->Signature == IMAGE_NT_SIGNATURE)
					{
						wchar_t buffer[BUFFER_SIZE];
			  		switch (peHeader->FileHeader.Machine)
			  		{
			  			case IMAGE_FILE_MACHINE_I386:  StringCbPrintfW(buffer, BUFFER_SIZE, L"Windows 32-bit"); break;
			  			case IMAGE_FILE_MACHINE_IA64:  StringCbPrintfW(buffer, BUFFER_SIZE, L"Windows Intel Itanium"); break;
			  			case IMAGE_FILE_MACHINE_AMD64: StringCbPrintfW(buffer, BUFFER_SIZE, L"Windows 64-bit"); break;
			  			default:
								StringCbPrintfW(buffer, BUFFER_SIZE, L"Unknown");
			  		}
			  		if (peHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_UNKNOWN)
         		  FormatMessageBoxW(hWnd, L"EXE Platform Identifier", MB_OK,
					        							L"EXE File: %s\nPlatform ID: %s\nMinimum required OS version: %d.%d",
					        							lpszFileName, buffer,
  															peHeader->OptionalHeader.MajorOperatingSystemVersion,
	  														peHeader->OptionalHeader.MinorOperatingSystemVersion);
					}
					else
     		  FormatMessageBoxW(hWnd, L"EXE Platform Identifier", MB_OK,
					     							L"EXE File: %s\nPlatform ID: DOS", lpszFileName);
			  }

       /*
        LPCWSTR platformID;
				switch (dosHeader->e_magic)
				{
					case IMAGE_DOS_SIGNATURE:    platformID = L"DOS"; break;
					case IMAGE_NT_SIGNATURE:     platformID = L"Windows NT"; break;
				  case IMAGE_OS2_SIGNATURE:    platformID = L"OS/2 Warp"; break;
				  case IMAGE_OS2_SIGNATURE_LE: platformID = L"OS/2 Warp (Little Endian)"; break;
				  default:
						platformID = L"Unknown"; break;
				}
				FormatMessageBoxW(hWnd, L"EXE Platform Identifier", MB_OK,
													L"EXE File: %s\nPlatform ID: %s", lpszFileName, platformID); */
				UnmapViewOfFile(lpFileBase);
        CloseHandle(hFileMapping);
        CloseHandle(hFile);
				return TRUE;
			}
		}
	}

  showError:
	{
		ShowFileError(lpszFileName);
		return GetLastError();
	}

 /*
  DWORD dwfvHandle   = 0;
  DWORD dwfvInfoSize = GetFileVersionInfoSizeW(lpszFileName, &dwfvHandle);
  VS_FIXEDFILEINFO vsFileInfo;
  wchar_t fviBuffer[dwfvInfoSize];
  UINT fviDataSize = 0;

  if ((dwfvInfoSize == 0) ||
			(!GetFileVersionInfoW(lpszFileName, dwfvHandle, dwfvInfoSize, fviBuffer)) ||
			(!VerQueryValueW(&fviBuffer, L"\\", (LPVOID) &vsFileInfo, &fviDataSize)))
		ShowFileError(lpszFileName);
	else
	{
		wchar_t buffer[255];
		switch (vsFileInfo.dwFileOS)
		{
			case VOS_NT: StringCbPrintfW(buffer, 255, L"Windows NT"); break;
			case VOS_NT_WINDOWS32: StringCbPrintfW(buffer, 255, L"Windows NT 32-bit"); break;
			case VOS_DOS: StringCbPrintfW(buffer, 255, L"DOS"); break;
			case VOS_DOS_WINDOWS16: StringCbPrintfW(buffer, 255, L"Windows 3.11 or lower"); break;
			case VOS_DOS_WINDOWS32: StringCbPrintfW(buffer, 255, L"Windows 32-bit for DOS"); break;
			case VOS_OS216: StringCbPrintfW(buffer, 255, L"OS/2 Warp 16-bit"); break;
		//	case VOS_OS216_PM16: StringCbPrintfW(buffer, 255, L"OS/2 Warp 16-bit Presentation Manager"); break;
			case VOS_OS232: StringCbPrintfW(buffer, 255, L"OS/2 Warp 32-bit"); break;
		//	case VOS_OS216_PM32: StringCbPrintfW(buffer, 255, L"OS/2 Warp 32-bit Presentation Manager"); break;
			case VOS_UNKNOWN: StringCbPrintfW(buffer, 255, L"Unknown");
		}
		FormatMessageBoxW(hWnd, L"EXE Platform Identification", MB_OK, L"EXE File Name: %s\nOS: %s", lpszFileName, buffer);
		result = TRUE;
	}
 */
	return result;
}


BOOL DoBrowseFiles(HWND hWnd)
{
  OPENFILENAMEW ofn;
  wchar_t       szFileName[MAX_PATH];

  ZeroMemory(&ofn, sizeof(ofn));
  szFileName[0] = 0;

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner   = hWnd;
  ofn.lpstrTitle  = L"Browse for EXE";
  ofn.lpstrFilter = L"EXE Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0\0";
  ofn.lpstrDefExt = L"exe";
  ofn.nMaxFile    = MAX_PATH;
  ofn.lpstrFile   = szFileName;
  ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

  if (GetOpenFileName(&ofn))
    DoShowEXEPlatformID(hWnd, ofn.lpstrFile);
  return TRUE;
}

INT_PTR CALLBACK DlgProc_EXEPFID(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDBROWSE:
          return DoBrowseFiles(hwndDlg);
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
  DialogBox(hInstance, MAKEINTRESOURCE(IDD_EXEPFID), NULL, DlgProc_EXEPFID);
  return 0;
}
