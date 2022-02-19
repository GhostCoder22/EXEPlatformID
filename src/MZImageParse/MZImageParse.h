#ifndef IMAGE_MZ_PARSE_H
#define IMAGE_MZ_PARSE_H

 #ifndef RC_INVOKED
  #include <windows.h>

  typedef enum _IMAGE_MZ_HEADERS_TYPE
  {   IMZH_UNKNOWN = 0x0000,
      IMZH_DOS     = 0x0001, // [16-bit] DOS
      IMZH_NT      = 0x0002, // [32/64-bit] Windows NT
      IMZH_WINDOS  = 0x0004, // [16-bit] Windows for DOS
      IMZH_WIN32S  = 0x0008, // [16-bit] Win32 Subsystem for Windows 3.x
      IMZH_DOS4    = 0x0010, // [16-bit] European DOS 4.x
      IMZH_OS2     = 0x0020, // [32-bit] OS/2 Warp
      IMZH_OS2_LE  = 0x002A, // [32-bit] OS/2 Warp Little Endian (LE)
      IMZH_BOSS    = 0x0040, // [16-bit] Borland Operating System Services (BOSS)
      IMZH_NE      = 0x0080, // [16-bit] Unknown PE ID
      IMZH_ERROR   = 0xFFFF
  } IMAGE_MZ_HEADERS_TYPE;

  typedef struct _IMAGE_MZ_HEADERS
  {
      IMAGE_DOS_HEADER       DOSStub;
      IMAGE_MZ_HEADERS_TYPE  Type;
      BYTE                   BitLevel;
      BOOL                   IsROM;
      union
      {
    	  IMAGE_NT_HEADERS  NTHeaders;
    	  IMAGE_OS2_HEADER  NEHeader;
      };
  } IMAGE_MZ_HEADERS,*PIMAGE_MZ_HEADERS;

  DWORD MZImageParse(PIMAGE_MZ_HEADERS pHeaders, LPCSTR lpszFilename);
 #endif // RC_INVOKED

#endif // IMAGE_MZ_PARSE_H
