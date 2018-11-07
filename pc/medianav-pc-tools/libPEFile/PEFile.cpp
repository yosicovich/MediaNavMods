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
**                                                                            **c
********************************************************************************
*******************************************************************************/

#include "PEFile.h"
#include <math.h>
#include <algorithm>
//==============================================================================
#define DEBUG_ENABLED true;
#ifdef DEBUG_ENABLED
	#define echo(x)			MessageBox(0, x, "DEBUG", MB_ICONERROR);
	#define echo2(x, y)		{ char v[256]; strcpy_s(v, 256, x); strcat_s(v, 256, y); echo(v); }
	#define echo3(x, y, z)	{ char w[256]; strcpy_s(w, 256, x); strcat_s(w, 256, y); echo2(w, z); }
#else
	#define echo(x) ;
	#define echo2(x, y) ;
	#define echo3(x, y, z) ;
#endif
//==============================================================================
PEFile::PEFile()
    : noSectionTruncate(false)
{
	init();
}
//==============================================================================
PEFile::PEFile(const char* filePath) 
    : noSectionTruncate(false)
{
	init();
	loadFromFile(filePath);
}
//==============================================================================
PEFile::~PEFile() {
	unloadFile();
}
//==============================================================================
void PEFile::init() {
	peMemory = NULL;
    newImports.clear();
    importTable.clear();
    memset(sections, 0, sizeof(sections));
    memset(sectionTable, 0, sizeof(sectionTable));
}
//==============================================================================
bool PEFile::readFileData(const char* filePath) {
	// open the file for read
	HANDLE fileHandle = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		echo3("Couldn't open file : [", filePath, "]");
		return false;
	}
	
	// get the file size
	DWORD fileSize = GetFileSize(fileHandle, 0);
	if (fileSize == 0) {
		CloseHandle(fileHandle);
		echo3("File size is ZeR0! : [", filePath, "]");
		return false;
	}
	
	// allocate memory to read the pe file (note that we used VirtualAlloc not GlobalAlloc!)
    peMemory = new char[fileSize];
	if (peMemory == NULL) {
		CloseHandle(fileHandle);
		echo("Couldn't allocate memory!");
		return false;
	}
	
	DWORD bytesRead;
	// read whole file data
	if (!ReadFile(fileHandle, peMemory, fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
		CloseHandle(fileHandle);
		echo3("Couldn't read file! : [", filePath, "]");
		return false;
	}

	// close the file
	CloseHandle(fileHandle);

	return true;
}
//==============================================================================
bool PEFile::checkValidity() {
	// 'dosHeader.Signature' must be "MZ" && 'peHeaders.Signature' must be "PE\0\0"
	if (dosHeader.Signature != IMAGE_DOS_SIGNATURE || peHeaders.Signature != IMAGE_NT_SIGNATURE) {
		unloadFile();
		echo("Invalid PE file!");
		return false;
	}

	if (peHeaders.FileHeader.NumberOfSections > MAX_SECTION_COUNT) {
		unloadFile();
		echo("Number of sections > MAX_SECTION_COUNT !");
		return false;
	}

	return true;
}
//==============================================================================
bool PEFile::readHeaders() {
	// read dos/pe headers
	memcpy(&dosHeader, peMemory, sizeof(PE_DOS_HEADER));
	dosStub.RawData = peMemory + sizeof(PE_DOS_HEADER);
	dosStub.Size = dosHeader.PEHeaderOffset - sizeof(PE_DOS_HEADER);
	memcpy(&peHeaders, peMemory + dosHeader.PEHeaderOffset, sizeof(PE_NT_HEADERS));

	// check validity of the file to ensure that we loaded a "PE File" not another thing!
	if (!checkValidity()) {
		return false;
	}

	// read section table
	memset(sectionTable, 0, sizeof(sectionTable));
	memcpy(sectionTable, peMemory + dosHeader.PEHeaderOffset + sizeof(PE_NT_HEADERS), 
		peHeaders.FileHeader.NumberOfSections * sizeof(PE_SECTION_HEADER));

	return true;
}
//==============================================================================
bool PEFile::readBody() {
	// read reserved data
	DWORD reservedDataOffset = dosHeader.PEHeaderOffset + sizeof(PE_NT_HEADERS) + 
		peHeaders.FileHeader.NumberOfSections * sizeof(PE_SECTION_HEADER);

	reservedData.Offset = reservedDataOffset;
	reservedData.RawData = peMemory + reservedDataOffset;
	/*reservedData.Size = peHeaders.OptionalHeader.SizeOfHeaders - reservedDataOffset;*/
	if (sectionTable[0].PointerToRawData > 0) {
		reservedData.Size = sectionTable[0].PointerToRawData - reservedDataOffset;
	} else {
		reservedData.Size = sectionTable[0].VirtualAddress - reservedDataOffset;
	}

	// read sections
	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		sections[i].Offset = sectionTable[i].PointerToRawData;
		sections[i].RawData = peMemory + sectionTable[i].PointerToRawData;
		sections[i].Size = sectionTable[i].SizeOfRawData;
	}

	return true;
}
//==============================================================================
bool PEFile::readImportTable() {
	DWORD tableRVA = peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD tableOffset = rvaToOffset(tableRVA);
	if (tableOffset == 0) {
		return false;
	}

    importTable.clear();

	IMAGE_IMPORT_DESCRIPTOR* importDesc = (IMAGE_IMPORT_DESCRIPTOR*)(peMemory + tableOffset);
	IMAGE_THUNK_DATA* importThunk;

	while (true) {
        PE_IMPORT_DLL importDll;
		importDll.DllName = (char*)(peMemory + rvaToOffset(importDesc->Name));
        if (importDesc->OriginalFirstThunk > 0) {
            importThunk = (IMAGE_THUNK_DATA*)(peMemory + rvaToOffset(importDesc->OriginalFirstThunk));
		} else {
            importThunk = (IMAGE_THUNK_DATA*)(peMemory + rvaToOffset(importDesc->FirstThunk));
		}
        DWORD importThunks = importDesc->FirstThunk;

        while (true) {
            PE_IMPORT_FUNCTION importFunction;
            if (importThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
				importFunction.FunctionId = IMAGE_ORDINAL32(importThunk->u1.Ordinal);
			} else {
				DWORD nameOffset = rvaToOffset(importThunk->u1.AddressOfData);
				importFunction.FunctionName = (char*)(peMemory + nameOffset + 2);
			}
            importFunction.VirtualAddress = importThunks + peHeaders.OptionalHeader.ImageBase;

            importDll.Functions.push_back(importFunction);

            importThunks += sizeof(IMAGE_THUNK_DATA);
            importThunk = (IMAGE_THUNK_DATA*)((char*)importThunk + sizeof(IMAGE_THUNK_DATA));
			if (importThunk->u1.AddressOfData == 0) {
				break;
			}
		}
        importTable.push_back(importDll);

		importDesc = (IMAGE_IMPORT_DESCRIPTOR*)((char*)importDesc + sizeof(IMAGE_IMPORT_DESCRIPTOR));
		if (importDesc->Name == 0) {
			break;
		}
	}

	return true;
}
//==============================================================================
bool PEFile::loadFromFile(const char* filePath) {
	unloadFile();

	return readFileData(filePath) &&
		   readHeaders() &&
		   readBody() &&
		   readImportTable();
}
//==============================================================================
bool PEFile::loadFromMemory(char* memoryAddress) {
	unloadFile();

	peMemory = memoryAddress;

	return readHeaders()/* &&
		   readBody() &&
		   readImportTable()*/;
}
//==============================================================================
bool PEFile::saveToFile(const char* filePath) {
	commit();
	buildImportTable();

	// create the output file
	HANDLE fileHandle = CreateFile(filePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		echo("Couldn't create file");
		return false;
	}

	DWORD bytesWritten;

	WriteFile(fileHandle, &dosHeader, sizeof(PE_DOS_HEADER), &bytesWritten, NULL);
	WriteFile(fileHandle, dosStub.RawData, dosStub.Size, &bytesWritten, NULL);
	writePadding(fileHandle, dosHeader.PEHeaderOffset - sizeof(PE_DOS_HEADER) - dosStub.Size);
	WriteFile(fileHandle, &peHeaders, sizeof(PE_NT_HEADERS), &bytesWritten, NULL);
	WriteFile(fileHandle, &sectionTable, peHeaders.FileHeader.NumberOfSections * sizeof(PE_SECTION_HEADER), &bytesWritten, NULL);
	WriteFile(fileHandle, reservedData.RawData, reservedData.Size, &bytesWritten, NULL);

	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		writePadding(fileHandle, sectionTable[i].PointerToRawData - GetFileSize(fileHandle, NULL));
		WriteFile(fileHandle, sections[i].RawData, sections[i].Size, &bytesWritten, NULL);
	}

	CloseHandle(fileHandle);

	return true;
}
//==============================================================================
bool PEFile::writePadding(HANDLE fileHandle, long paddingSize) {
	if (paddingSize <= 0)
		return false;
	
	DWORD bytesWritten;
	char* padding = new char[paddingSize];
	memset(padding, 0, paddingSize);
	WriteFile(fileHandle, padding, paddingSize, &bytesWritten, NULL);
	delete padding;

	return (bytesWritten == paddingSize);
}
//==============================================================================
void PEFile::unloadFile() {
	if (peMemory != NULL) {
		delete[] peMemory;
		peMemory = NULL;
	}

    for (auto item : globalsToFree)
    {
        GlobalFree(item);
    }

    globalsToFree.clear();

    newImports.clear();
}
//==============================================================================
void PEFile::buildImportTable() {
	
    if (!newImports.size())
        return;

    DWORD sizeDlls = 0;
	DWORD sizeFunctions = 0;
	DWORD sizeStrings = 0;
	DWORD newImportsSize = calcNewImportsSize(sizeDlls, sizeFunctions, sizeStrings);
	
	// we'll move the old dll list to the new import table, so we'll calc its size
    DWORD oldImportDllsSize = importTable.size() * sizeof(IMAGE_IMPORT_DESCRIPTOR);
	
	// add a new section to handle the new import table
	int index = addSection(SECTION_IMPORT, oldImportDllsSize + newImportsSize, false);

	// copy old import dll list
	DWORD oldImportTableRVA = peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD oldImportTableOffset = rvaToOffset(oldImportTableRVA);
	memcpy(sections[index].RawData, peMemory + oldImportTableOffset, oldImportDllsSize);
	
	// copy new imports
	char* newImportsData = buildNewImports(sectionTable[index].VirtualAddress + oldImportDllsSize);
	memcpy(sections[index].RawData + oldImportDllsSize, newImportsData, newImportsSize);
    delete[] newImportsData;
	
	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = sectionTable[index].VirtualAddress;
	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = sectionTable[index].SizeOfRawData;
	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0;
	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size = 0;
}
//==============================================================================
char* PEFile::buildNewImports(DWORD baseRVA) {
	commit();

	IMAGE_IMPORT_DESCRIPTOR importDesc;
	IMAGE_THUNK_DATA importThunk;

	DWORD sizeDlls = 0;
	DWORD sizeFunctions = 0;
	DWORD sizeStrings = 0;
	DWORD newImportsSize = calcNewImportsSize(sizeDlls, sizeFunctions, sizeStrings);
	DWORD offsetDlls = 0;
	DWORD offsetFunctions = sizeDlls;
	DWORD offsetStrings = sizeDlls + 2 * sizeFunctions;

	char* buffer = new char[newImportsSize];
	memset(buffer, 0, newImportsSize);
	
    for (auto importDll : newImports)
    {
        memset(&importDesc, 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        importDesc.OriginalFirstThunk = baseRVA + offsetFunctions;
        importDesc.FirstThunk = baseRVA + offsetFunctions + sizeFunctions;
        importDesc.Name = baseRVA + offsetStrings;
        memcpy(buffer + offsetStrings, importDll.DllName.c_str(), importDll.DllName.size());
        offsetStrings += alignNumber(importDll.DllName.size() + 1, 2);

        memcpy(buffer + offsetDlls, &importDesc, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        offsetDlls += sizeof(IMAGE_IMPORT_DESCRIPTOR);

        for (auto importFunction : importDll.Functions)
        {
            memset(&importThunk, 0, sizeof(IMAGE_THUNK_DATA));
            if (importFunction.FunctionId != 0) 
            {
                importThunk.u1.Ordinal = importFunction.FunctionId | IMAGE_ORDINAL_FLAG32;
            }
            else
            {
                importThunk.u1.AddressOfData = baseRVA + offsetStrings;
                memcpy(buffer + offsetStrings + 2, importFunction.FunctionName.c_str(), importFunction.FunctionName.size());
                offsetStrings += 2 + alignNumber(importFunction.FunctionName.size() + 1, 2);
            }

            memcpy(buffer + offsetFunctions, &importThunk, sizeof(IMAGE_THUNK_DATA));
            memcpy(buffer + offsetFunctions + sizeFunctions, &importThunk, sizeof(IMAGE_THUNK_DATA));
            offsetFunctions += sizeof(IMAGE_THUNK_DATA);
        }
        offsetFunctions += sizeof(IMAGE_THUNK_DATA);
    }
	return buffer;
}
//==============================================================================
DWORD PEFile::calcNewImportsSize(DWORD &sizeDlls, DWORD &sizeFunctions, DWORD &sizeStrings) {
	
    for (auto importDll : newImports)
    {
        sizeDlls += sizeof(IMAGE_IMPORT_DESCRIPTOR);
        sizeStrings += alignNumber(importDll.DllName.size() + 1, 2);
        for (auto importFunction : importDll.Functions)
        {
            sizeFunctions += sizeof(IMAGE_THUNK_DATA);
            if (importFunction.FunctionId == 0) 
            {
                sizeStrings += 2 + alignNumber(importFunction.FunctionName.size() + 1, 2);
            }
            sizeFunctions += sizeof(IMAGE_THUNK_DATA); // for the terminator thunk data
        }
    }
    sizeDlls += sizeof(IMAGE_IMPORT_DESCRIPTOR); // for the terminator import descriptor
    return sizeDlls + 2 * sizeFunctions + sizeStrings;
}
//==============================================================================
int PEFile::addSection(const char* name, DWORD size, bool isExecutable) {
	if (peHeaders.FileHeader.NumberOfSections == MAX_SECTION_COUNT) {
		return -1;
	}

	PE_SECTION_DATA &newSection = sections[peHeaders.FileHeader.NumberOfSections];
	PE_SECTION_HEADER &newSectionHeader = sectionTable[peHeaders.FileHeader.NumberOfSections];
	PE_SECTION_HEADER &lastSectionHeader = sectionTable[peHeaders.FileHeader.NumberOfSections - 1];
	
	DWORD sectionSize = alignNumber(size, noSectionTruncate ? peHeaders.OptionalHeader.SectionAlignment : peHeaders.OptionalHeader.FileAlignment);
	DWORD virtualSize = alignNumber(sectionSize, peHeaders.OptionalHeader.SectionAlignment);
	
	DWORD sectionOffset = alignNumber(lastSectionHeader.PointerToRawData + lastSectionHeader.SizeOfRawData, peHeaders.OptionalHeader.FileAlignment);
	DWORD virtualOffset = alignNumber(lastSectionHeader.VirtualAddress + lastSectionHeader.Misc.VirtualSize, peHeaders.OptionalHeader.SectionAlignment);
	
	ZeroMemory(&newSectionHeader, sizeof(IMAGE_SECTION_HEADER));
	CopyMemory(newSectionHeader.Name, name, (strlen(name) > 8 ? 8 : strlen(name)));
	
	newSectionHeader.PointerToRawData = sectionOffset;
	newSectionHeader.VirtualAddress = virtualOffset;
	newSectionHeader.SizeOfRawData = sectionSize;
	newSectionHeader.Misc.VirtualSize = virtualSize;
	newSectionHeader.Characteristics = //0xC0000040; 
		IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_CNT_INITIALIZED_DATA;
	
	if (isExecutable) {
		newSectionHeader.Characteristics |= IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE;
	}
	
	newSection.RawData = (char*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sectionSize);
    globalsToFree.push_back(newSection.RawData);
	newSection.Size = sectionSize;
	
	peHeaders.FileHeader.NumberOfSections++;
	if (reservedData.Size > 0) {
		reservedData.Size -= sizeof(IMAGE_SECTION_HEADER);
	}

	// return new section index
	return peHeaders.FileHeader.NumberOfSections - 1;
}
//==============================================================================
void PEFile::addImport(const std::string& dllName, std::vector<std::string> functions)
{
    PE_IMPORT_DLL importDll;
    importDll.DllName = dllName;
    for (auto importFunction : functions)
    {
        PE_IMPORT_FUNCTION newFunc;
        newFunc.FunctionName = importFunction;
        importDll.Functions.push_back(newFunc);
    }
    newImports.push_back(importDll);
}
//==============================================================================
DWORD PEFile::alignNumber(DWORD number, DWORD alignment) {
	return (DWORD)(ceil(number / (alignment + 0.0)) * alignment);
}

DWORD PEFile::vaToOffset(DWORD va)
{
    return rvaToOffset(va - peHeaders.OptionalHeader.ImageBase);
}

//==============================================================================
DWORD PEFile::rvaToOffset(DWORD rva) {
	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		if (rva >= sectionTable[i].VirtualAddress &&
			rva < sectionTable[i].VirtualAddress + sectionTable[i].Misc.VirtualSize) {
			return sectionTable[i].PointerToRawData + (rva - sectionTable[i].VirtualAddress);
		}
	}
	return 0;
}
//==============================================================================
DWORD PEFile::offsetToRVA(DWORD offset) {
	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		if (offset >= sectionTable[i].PointerToRawData &&
			offset < sectionTable[i].PointerToRawData + sectionTable[i].SizeOfRawData) {
			return sectionTable[i].VirtualAddress + (offset - sectionTable[i].PointerToRawData);
		}
	}
	return 0;
}
//==============================================================================
void PEFile::commit() {
	fixReservedData();
	fixHeaders();
	fixSectionTable();
}
//==============================================================================
void PEFile::fixReservedData() {
	DWORD dirIndex = 0;
	for	(dirIndex = 0; dirIndex < peHeaders.OptionalHeader.NumberOfRvaAndSizes; dirIndex++) {
		if (peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress > 0 && 
			peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress >= reservedData.Offset &&
			peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress < reservedData.Size) {
			break;
		}
	}

	if (dirIndex == peHeaders.OptionalHeader.NumberOfRvaAndSizes) {
		return;
	}

	int sectionIndex = addSection(SECTION_RESERV, reservedData.Size, false);
	memcpy(sections[sectionIndex].RawData, reservedData.RawData, reservedData.Size);

	for	(dirIndex = 0; dirIndex < peHeaders.OptionalHeader.NumberOfRvaAndSizes; dirIndex++) {
		if (peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress > 0 &&
			peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress >= reservedData.Offset &&
			peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress < reservedData.Size) {
			peHeaders.OptionalHeader.DataDirectory[dirIndex].VirtualAddress += 
				sectionTable[sectionIndex].VirtualAddress - reservedData.Offset;
		}
	}

	reservedData.Size = 0;
}
//==============================================================================
void PEFile::fixHeaders() {
	peHeaders.OptionalHeader.SizeOfHeaders = alignNumber(dosHeader.PEHeaderOffset + peHeaders.FileHeader.SizeOfOptionalHeader +
		peHeaders.FileHeader.NumberOfSections * sizeof(PE_SECTION_HEADER), peHeaders.OptionalHeader.FileAlignment);

	DWORD imageSize = peHeaders.OptionalHeader.SizeOfHeaders;
	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		imageSize += alignNumber(sectionTable[i].Misc.VirtualSize, peHeaders.OptionalHeader.SectionAlignment);
	}
	peHeaders.OptionalHeader.SizeOfImage = alignNumber(imageSize, peHeaders.OptionalHeader.SectionAlignment);

	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
	peHeaders.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
}
//==============================================================================
void PEFile::fixSectionTable() {
	DWORD offset = peHeaders.OptionalHeader.SizeOfHeaders;
	for	(int i = 0; i < peHeaders.FileHeader.NumberOfSections; i++) {
		//sectionTable[i].Characteristics |= IMAGE_SCN_MEM_WRITE; // this makes all sections writeable!
		offset = alignNumber(offset, peHeaders.OptionalHeader.FileAlignment);
		sectionTable[i].PointerToRawData = offset;
		//sectionTable[i].SizeOfRawData = alignNumber(offset + sectionTable[i].Misc.VirtualSize, peHeaders.OptionalHeader.FileAlignment);
		offset += sectionTable[i].SizeOfRawData;
	}
}
//==============================================================================

size_t PEFile::getSectionsCount()
{
    return peHeaders.FileHeader.NumberOfSections;
}

size_t  PEFile::findSection(const std::string& name)
{
    for (size_t i = 0; i < getSectionsCount(); ++i)
    {
        std::string sectionName((char *)&(sectionTable[i].Name[0]), IMAGE_SIZEOF_SHORT_NAME, strnlen((char *)&(sectionTable[i].Name[0]), IMAGE_SIZEOF_SHORT_NAME));
        if (sectionName == name)
            return i;
    }

    return -1;
}

bool PEFile::patchSection(DWORD va, const void* patchBuf, size_t size)
{
    if (patchBuf == NULL && size != 0)
        return false;
    if (patchBuf == NULL)
        return true;
    DWORD offset = vaToOffset(va);
    if (offset == 0)
        return false;
    memcpy(&peMemory[offset], patchBuf, size);
    return true;
}

bool PEFile::saveAndReload(const std::string& filePath)
{
    if (!saveToFile(filePath.c_str()))
        return false;
    unloadFile();
    return loadFromFile(filePath.c_str());
}

DWORD PEFile::getImportFunctionVA(const std::string& dllName, const std::string& funcName)
{
    for (auto dll : importTable)
    {
        if (dll.DllName != dllName)
            continue;
        for (auto func : dll.Functions)
        {
            if (func.FunctionName == funcName)
                return func.VirtualAddress;
        }
    }
    throw std::exception((std::string("Unable to find requested import: ") + dllName + "/" + funcName).c_str());
}
