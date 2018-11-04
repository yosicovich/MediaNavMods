/*******************************************************************************
********************************   Team AT4RE   ********************************
********************************************************************************
*******************  PLEASE DON'T CHANGE/REMOVE THIS HEADER  *******************
********************************************************************************
**                                                                            **
**	Title:		PEFile class.                                                 **
**	Desc:		A handy class to manipulate pe files.                         **
**	Author:		MohammadHi [ in4matics at hotmail dot com ]                   **
**	WwW:		AT4RE      [ http://www.at4re.com ]                           **
**	Date:		2008-01-28                                                    **
**                                                                            **
********************************************************************************
*******************************************************************************/

/*
[  PE File Format   ]
---------------------
|    DOS Header     |
---------------------
|     DOS Stub      |
---------------------
|     PE Header     |
---------------------
|   Section Table   |
---------------------
|      Padding      |
---------------------
|     Section 1     |
---------------------
|     Section 2     |
---------------------
|	     ...        |
---------------------
|     Section n     |
---------------------*/

//==============================================================================
#pragma once
#pragma pack(1)
//==============================================================================
#include <windows.h>
#include <string>
#include <vector>
//==============================================================================
#define MAX_SECTION_COUNT		64
#define SECTION_IMPORT			"@.import"
#define SECTION_RESERV			"@.reserv"
//==============================================================================
struct PE_DOS_HEADER {
	WORD   Signature;
	WORD   LastPageBytes;
	WORD   NumberOfPages;
	WORD   Relocations;
	WORD   HeaderSize;
	WORD   MinMemory;
	WORD   MaxMemory;
	WORD   InitialSS;
	WORD   InitialSP;
	WORD   Checksum;
	WORD   InitialIP;
	WORD   InitialCS;
	WORD   RelocTableOffset;
	WORD   Overlay;
	WORD   Reserved1[4];
	WORD   OemId;
	WORD   OemInfo;
	WORD   Reserved2[10];
	LONG   PEHeaderOffset;
};
struct PE_DOS_STUB {
	char*   RawData;
	DWORD   Size;
};
struct PE_SECTION_DATA {
	DWORD   Offset;
	char*   RawData;
	DWORD   Size;
};
struct PE_IMPORT_FUNCTION {
    PE_IMPORT_FUNCTION()
        :FunctionId(0), VirtualAddress(0)
    {

    }
	std::string			FunctionName;
	int					FunctionId;
    DWORD               VirtualAddress;
};
struct PE_IMPORT_DLL {
	std::string				DllName;
    std::vector<PE_IMPORT_FUNCTION> Functions;
};
//==============================================================================
typedef IMAGE_NT_HEADERS PE_NT_HEADERS;
typedef IMAGE_SECTION_HEADER PE_SECTION_HEADER;
//==============================================================================
class PEFile {
public:
	PE_DOS_HEADER		dosHeader;
	PE_DOS_STUB			dosStub;
	PE_NT_HEADERS		peHeaders;
	PE_SECTION_HEADER	sectionTable[MAX_SECTION_COUNT];
	PE_SECTION_DATA		reservedData;
	PE_SECTION_DATA		sections[MAX_SECTION_COUNT];
	std::vector<PE_IMPORT_DLL>		importTable;
	std::vector<PE_IMPORT_DLL>		newImports;

	PEFile();
	PEFile(const char* filePath);
	~PEFile();
	bool				loadFromFile(const char* filePath);
	bool				loadFromMemory(char* memoryAddress);
	bool				saveToFile(const char* filePath);
	int					addSection(const char* name, DWORD size, bool isExecutable);
	void				addImport(const std::string& dllName, std::vector<std::string> functions);
	void				commit();
    void                setNoSectionTruncate(bool val) { noSectionTruncate = val; }
    size_t              getSectionsCount();
    size_t              findSection(const std::string& name);
    bool patchSection(DWORD va, const void* patchBuf, size_t size);
    bool saveAndReload(const std::string& filePath);

    DWORD getImportFunctionVA(const std::string& dllName, const std::string& funcName);


private:
	char*				peMemory;
    bool                noSectionTruncate;
	
	void				init();
	bool				readFileData(const char* filePath);
	bool				checkValidity();
	bool				readHeaders();
	bool				readBody();
	bool				readImportTable();
	bool				writePadding(HANDLE fileHandle, long paddingSize);
	void				unloadFile();

	void				buildImportTable();
	char*				buildNewImports(DWORD baseRVA);
	DWORD				calcNewImportsSize(DWORD &sizeDlls, DWORD &sizeFunctions, DWORD &sizeStrings);

	DWORD				alignNumber(DWORD number, DWORD alignment);
    DWORD				vaToOffset(DWORD va);
    DWORD				rvaToOffset(DWORD rva);
	DWORD				offsetToRVA(DWORD offset);
	
	void				fixReservedData();
	void				fixHeaders();
	void				fixSectionTable();

    std::vector<void*>  globalsToFree;
	
};
//==============================================================================
