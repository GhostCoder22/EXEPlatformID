/*
MZ Platform Identifier
----------------------
MZImageParser.c

This internal library has only one function, MZParseHeaders, which is able to properly read in
.EXE/.DLL files and obtain internal header information from those files. It first does this by
examining file header's identifier. If this identifier is "MZ" (the initials of Mark Zbikowski,
one the leading MS-DOS developers), then a DOS stub is assumed and a search for additional
identification "PE" / "NE" markers conducted near the end of the DOSS stub. If any of these
identification markers is present, the header of the matching identification marker is accessed
to determine the bit level of the .EXE/.DLL. If none of these additional identification markers
are present, then DOS 16-bit is assumed.
*/
#include "MZImageParse.h"

/*
While the following disabled functional code is incomplete, I'm purposely leaving as a reference
to show how to properly parse executable/DLL file headers using the CRT functions "fopen", "fclose",
"fseek" and "memcpy".

DWORD MZImageParse(PIMAGE_EXE_HEADERS pHeaders, LPCSTR lpszFilename)
{
  ZeroMemory(pHeaders, sizeof(IMAGE_EXE_HEADERS));
	if ((pHeaders == NULL) || (lpszFilename == NULL))
	  SetLastError(ERROR_INVALID_PARAMETER);
	else
	{
		FILE* FileHandle = fopen(lpszFilename, "rb");
		if (FileHandle == NULL)
		 { SetLastError(ERROR_FILE_INVALID); pHeaders->Type = IMZH_ERROR; }
		else
		{
			IMAGE_DOS_HEADER DOSStub;
			fseek(FileHandle, 0, SEEK_SET);
			fread(&DOSStub, sizeof(IMAGE_DOS_HEADER), 1, FileHandle);
			if (DOSStub.e_magic != IMAGE_DOS_SIGNATURE)
			 { SetLastError(ERROR_BAD_EXE_FORMAT); pHeaders->Type = IMZH_UNKNOWN; }
			else
			{
				if (DOSStub.e_lfarlc < 0x0040)
				 { pHeaders->Type = IMZH_DOS; pHeaders->BitLevel = 16; }
				else
				{
					IMAGE_NT_HEADERS NTHeaders;
					memcpy(&pHeaders->DOSStub, &DOSStub, sizeof(IMAGE_DOS_HEADER));
					fseek(FileHandle, DOSStub.e_lfanew, SEEK_SET);
					fread(&NTHeaders, sizeof(IMAGE_NT_HEADERS), 1, FileHandle);
					if (NTHeaders.Signature == IMAGE_NT_SIGNATURE)
					{
						pHeaders->Type = IMZH_NT;
						memcpy(&pHeaders->NTHeaders, &NTHeaders, sizeof(IMAGE_NT_HEADERS));
						switch (pHeaders->NTHeaders.OptionalHeader.Magic)
						{
							case IMAGE_NT_OPTIONAL_HDR32_MAGIC: pHeaders->BitLevel = 32; break;
							case IMAGE_NT_OPTIONAL_HDR64_MAGIC: pHeaders->BitLevel = 64; break;
							case IMAGE_ROM_OPTIONAL_HDR_MAGIC: pHeaders->BitLevel = 1; break;
							default: pHeaders->BitLevel = 0;
						}
						pHeaders->IsROM = (pHeaders->BitLevel == 1 ? TRUE : FALSE);
					}
				}
			}
			fclose(FileHandle);
		}
	}
	return GetLastError();
}
*/

DWORD MZImageParse(PIMAGE_MZ_HEADERS pHeaders, LPCSTR lpszFilename)
{
  SetLastError(0);
  ZeroMemory(pHeaders, sizeof(IMAGE_MZ_HEADERS));
	if ((pHeaders == NULL) || (lpszFilename == NULL))
	  SetLastError(ERROR_INVALID_PARAMETER);
	else
	{
    HANDLE FileHandle = CreateFileA(lpszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    pHeaders->Type = IMZH_ERROR;
    if (FileHandle != NULL)
    {
			HANDLE FileMapping = CreateFileMappingA(FileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
			if (FileMapping == NULL)
				CloseHandle(FileHandle);
			else
  		{
				LPVOID FileBase = MapViewOfFile(FileMapping, FILE_MAP_READ, 0, 0, 0);
				if (FileBase == NULL)
			   { CloseHandle(FileMapping); CloseHandle(FileHandle); }
				else
				{
					PIMAGE_DOS_HEADER DOSStub = (PIMAGE_DOS_HEADER) FileBase;
					if (DOSStub->e_magic != IMAGE_DOS_SIGNATURE)
						SetLastError(ERROR_BAD_EXE_FORMAT);
					else
					{
						CopyMemory(&pHeaders->DOSStub, DOSStub, sizeof(IMAGE_DOS_HEADER));
						if (DOSStub->e_lfarlc < 0x0040)
						 { pHeaders->Type = IMZH_DOS; pHeaders->BitLevel = 16; }
						else
						{
							PIMAGE_NT_HEADERS NTHeaders = (PIMAGE_NT_HEADERS) ((UCHAR*)DOSStub+DOSStub->e_lfanew);
							PIMAGE_OS2_HEADER NEHeader = (PIMAGE_OS2_HEADER) ((UCHAR*)DOSStub+DOSStub->e_lfanew);
							if (NTHeaders->Signature == IMAGE_NT_SIGNATURE)
							{
								pHeaders->Type = IMZH_NT;
								CopyMemory(&pHeaders->NTHeaders, NTHeaders, sizeof(IMAGE_NT_HEADERS));
								switch (pHeaders->NTHeaders.OptionalHeader.Magic)
								{
									case IMAGE_NT_OPTIONAL_HDR32_MAGIC: pHeaders->BitLevel = 32; break;
									case IMAGE_NT_OPTIONAL_HDR64_MAGIC: pHeaders->BitLevel = 64; break;
									case IMAGE_ROM_OPTIONAL_HDR_MAGIC: pHeaders->BitLevel = 1; break;
									default: pHeaders->BitLevel = 0;
								}
								pHeaders->IsROM = (pHeaders->BitLevel == 1 ? TRUE : FALSE);
							}
							else if (NEHeader->ne_magic == IMAGE_OS2_SIGNATURE)
							{
								pHeaders->BitLevel = 16;
								CopyMemory(&pHeaders->NEHeader, NEHeader, sizeof(IMAGE_OS2_HEADER));
								switch (NEHeader->ne_exetyp)
								{
									case 0: pHeaders->Type = IMZH_NE; break;
									case 1: pHeaders->Type = IMZH_OS2; break;
									case 2: pHeaders->Type = IMZH_WINDOS; break;
									case 3: pHeaders->Type = IMZH_DOS4; break;
									case 4: pHeaders->Type = IMZH_WIN32S; break;
									case 5: pHeaders->Type = IMZH_BOSS;
								}
								if (pHeaders->Type == IMZH_WIN32S)
									pHeaders->BitLevel = 32;
							}
						}
						UnmapViewOfFile(FileBase);
						CloseHandle(FileMapping);
						CloseHandle(FileHandle);
						SetLastError(0);
					}
				}
      }
    }
	}
	return GetLastError();
}

