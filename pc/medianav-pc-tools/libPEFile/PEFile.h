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
#include <map>
#include <memory>

static const int cRelocationOffsetBits = 12;

struct RelocationType
{
    RelocationType(int slots, BYTE recType)
        :slots(slots), recType(recType)
    {}

    virtual void fillSlot(DWORD va, WORD* slot)
    {
        *slot = (WORD)(va & ((1 << cRelocationOffsetBits) - 1)) | (WORD)recType << cRelocationOffsetBits;
    }

    bool isAbsolute()
    {
        return recType == IMAGE_REL_BASED_ABSOLUTE;
    }

    int slots;
    BYTE recType;
};

struct RelocationTypeGeneric : public RelocationType
{
    RelocationTypeGeneric(BYTE relType)
        :RelocationType(1, relType)
    {

    }
};

struct RelocationTypeAbsolute : public RelocationType
{
    RelocationTypeAbsolute()
        :RelocationType(1, IMAGE_REL_BASED_ABSOLUTE)
    {

    }
};

struct RelocationTypeHighAdj : public RelocationType
{
    RelocationTypeHighAdj(WORD lowWord)
        :RelocationType(2, IMAGE_REL_BASED_HIGHADJ), lowWord(lowWord)
    {

    }

    virtual void fillSlot(DWORD va, WORD* slot)
    {
        RelocationType::fillSlot(va, slot);
        *(++slot) = lowWord;
    }

    WORD lowWord;

};

typedef std::shared_ptr<RelocationType> RelocationTypePtr;
struct RelocationRecord
{
    RelocationRecord(BYTE relocType, WORD* slot)
    {
        if (relocType == IMAGE_REL_BASED_HIGHADJ)
        {
            pRelocType.reset(new RelocationTypeHighAdj(*(++slot)));
        }
        else
        {
            pRelocType.reset(new RelocationTypeGeneric(relocType));
        }
    }

    RelocationRecord(const RelocationTypePtr& relocType)
        :pRelocType(relocType)
    {

    }
    RelocationTypePtr pRelocType;
};
typedef std::map<DWORD, RelocationRecord> NewRelocationsData;
typedef std::vector<DWORD> DeleteRelocationsData;
typedef std::map<DWORD, RelocationRecord> RelocationsSegmentData;
typedef std::map<DWORD, RelocationsSegmentData> RelocationsTable;

class PatchCase
{
public:
    size_t getDataSize() const
    {
        return bufSize;
    }

    const uint8_t * getData() const
    {
        return reinterpret_cast<const uint8_t *>(dataBuf.get());
    }

    const DWORD getApplyVA() const
    {
        return applyVA_;
    }

    const NewRelocationsData& getNewRelocations() const
    {
        return newRelocations_;
    }

    const DeleteRelocationsData& getDeleteRelocations() const
    {
        return deleteRelocations_;
    }

protected:
    PatchCase(DWORD applyVA, size_t patchSize)
        :applyVA_(applyVA), bufSize(patchSize), dataBuf(new uint8_t[bufSize])
    {
    }

    void clearRelocations()
    {
        for (size_t i = 0; i < bufSize / 4; ++i)
            deleteRelocations_.push_back(applyVA_ + i * sizeof(DWORD)); // Must be DWORD aligned
    }

    void clearRelocations(const DeleteRelocationsData& relocations)
    {
        deleteRelocations_.insert(deleteRelocations_.end(), relocations.begin(), relocations.end());
    }

    bool addRelocations(const NewRelocationsData& newRelocations)
    {
        //newRelocations_.insert(newRelocations.begin(), newRelocations.end());
        for (auto rec : newRelocations)
        {
            if (!newRelocations_.insert(rec).second)
                return false;
        }
        return true;
    }

    bool addRelocation(DWORD va, const RelocationTypePtr& pRelocType)
    {
        return newRelocations_.insert(std::make_pair(va, RelocationRecord(pRelocType))).second;
    }

    NewRelocationsData newRelocations_;
    DeleteRelocationsData deleteRelocations_;
    DWORD applyVA_;
    size_t bufSize;
    std::unique_ptr<uint8_t> dataBuf;
};

class CodePatchCase: public PatchCase
{
protected:
    CodePatchCase(DWORD applyVA, size_t patchSize)
        :PatchCase(applyVA, patchSize)
    {
        clearRelocations();
    }
};


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
    RelocationsTable relocationTable;

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
    bool patch(const PatchCase& patchCase);
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
    bool				readRelocationTable();
    bool				writePadding(HANDLE fileHandle, long paddingSize);
	void				unloadFile();

	void				buildImportTable();
    bool				buildRelocationTable();
    char*				buildNewImports(DWORD baseRVA);
	DWORD				calcNewImportsSize(DWORD &sizeDlls, DWORD &sizeFunctions, DWORD &sizeStrings);

	DWORD				alignNumber(DWORD number, DWORD alignment);
    DWORD				vaToOffset(DWORD va);
    DWORD				rvaToOffset(DWORD rva);
	DWORD				offsetToRVA(DWORD offset);

    DWORD               rvaToVA(DWORD rva);
    DWORD               vaToRVA(DWORD va);
	
	void				fixReservedData();
	void				fixHeaders();
	void				fixSectionTable();

    inline DWORD        getRelocationSegmentBase(DWORD VA)
    {
        return VA & ~(((WORD)1 << cRelocationOffsetBits) - 1);
    }

    inline DWORD        getRelocationSegmentOffset(DWORD VA)
    {
        return VA & (((WORD)1 << cRelocationOffsetBits) - 1);
    }

    std::vector<void*>  globalsToFree;
    bool rebuildRelocations;
	
};
//==============================================================================
