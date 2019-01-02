// lgu2dir.cpp : définit le point d'entrée pour l'application console.
//

#include <stdio.h>
#include <direct.h>
#include <iostream>
#include <Windows.h>
#include <Shlwapi.h>
#include <vector>
#include <time.h>
#include <tchar.h>

using namespace std;

#include "header.h"
#include "xorArray.h"
#include "zip.h"

#define WRITEBUFFERSIZE (8192)

uLong filetime(char *f, tm_zip *tmzip, uLong *dt)
{
	int ret = 0;

	FILETIME ftLocal;
	HANDLE hFind;
	WIN32_FIND_DATAA ff32;

	hFind = FindFirstFileA(f, &ff32);
	if (hFind != INVALID_HANDLE_VALUE) {
		FileTimeToLocalFileTime(&(ff32.ftLastWriteTime),&ftLocal);
		FileTimeToDosDateTime(&ftLocal,((LPWORD)dt)+1,((LPWORD)dt)+0);
		FindClose(hFind);
		ret = 1;
	}
	
	return ret;
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
int getFileCrc(const char* filenameinzip, void* buf, unsigned long size_buf, unsigned long* result_crc)
{
   unsigned long calculate_crc = 0;
   int err = ZIP_OK;
   FILE * fin = fopen(filenameinzip,"rb");
   unsigned long size_read = 0;
   unsigned long total_read = 0;

   if (fin == NULL)
       err = ZIP_ERRNO;

    if (err == ZIP_OK)
        do {
            err = ZIP_OK;
            size_read = (int)fread(buf,1,size_buf,fin);
            if (size_read < size_buf) {
				if (feof(fin)==0) {
					printf("error in reading %s\n",filenameinzip);
					err = ZIP_ERRNO;
				}
			}

            if (size_read>0)
                calculate_crc = crc32(calculate_crc, (const Bytef*)buf, size_read);

            total_read += size_read;
        } while ((err == ZIP_OK) && (size_read>0));

    if (fin)
        fclose(fin);

    *result_crc=calculate_crc;
    printf("file %s crc %lx\n",filenameinzip,calculate_crc);
    return err;
}

void listFilesRecursively(char* lpFolder, vector<string> *folderList, char* lpFilePattern)
{
    char szFullPattern[MAX_PATH];
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFindFile;

	// now we are going to look for the matching files
    PathCombineA(szFullPattern, lpFolder, lpFilePattern);
    hFindFile = FindFirstFileA(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE) {
        do {
			if((strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0))
				continue;

            if(!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                // found a file; do something with it
                PathCombineA(szFullPattern, lpFolder, FindFileData.cFileName);
				folderList->push_back(szFullPattern);
            }
        } while(FindNextFileA(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }

    // first we are going to process any subdirectories
    PathCombineA(szFullPattern, lpFolder, "*");
    hFindFile = FindFirstFileA(szFullPattern, &FindFileData);
    if(hFindFile != INVALID_HANDLE_VALUE) {
        do {
			if((strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0))
				continue;

            if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                // found a subdirectory; recurse into it
                PathCombineA(szFullPattern, lpFolder, FindFileData.cFileName);
                listFilesRecursively(szFullPattern, folderList, lpFilePattern);
            }
        } while(FindNextFileA(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

void fileToZip(zipFile zf, char* lpFilePath, char *password)
{
	FILE * fin;
	int size_read;
	const char *savefilenameinzip;
	zip_fileinfo zi;
	unsigned long crcFile=0;
	int err = ZIP_OK;

	int size_buf = WRITEBUFFERSIZE;
    void* buf = (void*)malloc(size_buf);

	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;
	filetime(lpFilePath, &zi.tmz_date, &zi.dosDate);

	if ((password != NULL) && (err == ZIP_OK))
		err = getFileCrc(lpFilePath, buf, size_buf, &crcFile);

	/*the path name saved, should not include a leading slash. */
	/*if it did, windows/xp and dynazip couldn't read the zip file. */
	savefilenameinzip = lpFilePath;
	while( savefilenameinzip[0] == '\\' || savefilenameinzip[0] == '/' )
		savefilenameinzip++;

	err = zipOpenNewFileInZip3(zf, savefilenameinzip, &zi,
		NULL, 0, NULL, 0, NULL /* comment*/,
		Z_DEFLATED, 5 /* compression level */, 0,
		/* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
		-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
		password, crcFile);

	if (err != ZIP_OK) {
		printf("error in opening %s in zipfile\n", lpFilePath);
	} else {
		fin = fopen(lpFilePath, "rb");
		if (fin==NULL) {
			err = ZIP_ERRNO;
			printf("error in opening %s for reading\n", lpFilePath);
		}
	}

	if (err == ZIP_OK) {
		do {
			err = ZIP_OK;
			size_read = fread(buf, 1, size_buf, fin);
			if (size_read < size_buf) {
				if (feof(fin) == 0) {
					printf("error in reading %s\n",lpFilePath);
					err = ZIP_ERRNO;
				}
			}

			if (size_read > 0) {
				err = zipWriteInFileInZip(zf, buf, size_read);

				if (err < 0)
					printf("error in writing %s in the zipfile\n", lpFilePath);
			}
		} while ((err == ZIP_OK) && (size_read > 0));

		if (fin)
			fclose(fin);

		if (err<0) {
			err=ZIP_ERRNO;
		} else {
			err = zipCloseFileInZip(zf);
			if (err!=ZIP_OK)
				printf("error in closing %s in the zipfile\n", lpFilePath);
		}
	}

	free(buf);
}

int createZip(char* outputFile, int isULC2)
{
	zipFile zf = zipOpen(outputFile, 0);

	if (zf == NULL) {
		printf("error opening %s\n", outputFile);
		return -1;
	} else {
		printf("creating %s\n", outputFile);
	}

	vector<string> filesList;
	listFilesRecursively("", &filesList, "*");

	char *password;
	if (isULC2 == 1)
		password = "u8u8l^-^8@E4g_H2kq8X_pack";
	else
		password = "I_LOVE_LG^^";

	for (vector<string>::iterator it = filesList.begin() ; it != filesList.end(); ++it)
		fileToZip(zf, (char*)it->c_str(), password);

    if (zipClose(zf, NULL) != ZIP_OK) {
		printf("error in closing %s\n", outputFile);
		return -1;
	}

	return 0;
}

BOOL DirectoryExists(LPCWSTR dirName_in)
{
	DWORD attribs = GetFileAttributes(dirName_in);
	if (attribs == INVALID_FILE_ATTRIBUTES)
		return false;
	
	return (attribs & FILE_ATTRIBUTE_DIRECTORY);
}

void usage()
{
	printf("Usage: dir2lgu [options] <content name> <in folder> <out lgu file>\n");
	printf(" -n <name>	name label (\"      \" by default)\n");
	printf(" -l		lgu0 format (default)\n");
	printf(" -u		ulc2 format\n");
	printf(" -p <m1|m2>	preset medianav 1 or 2\n");
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 4) {
		usage();
		return -1;
	}

	int isULC2 = 0;
	wstring label_name = _T("        ");
	int index = 1;

	if (argc > 4) {
		bool stop = false;
		do {
			if (argv[index][0] == '-') {
				switch (argv[index][1]) {
				case 'n':
					if (argc > index+1) {
						label_name = argv[index+1];
					} else {
						usage();
						return -1;
					}

					index += 2;
					break;
				case 'p':
					if (argc > index+1) {
						LPCWSTR preset = argv[index+1];
						if (wcscmp(preset, _T("m1")) == 0) {
							label_name = _T("*MEDIA-NAV*");
						} else if (wcscmp(preset, _T("m2")) == 0) {
							label_name = _T("*MEDIA-NAV2*");
							isULC2 = 1;
						}
					} else {
						usage();
						return -1;
					}

					index += 2;
					break;
				case 'u':
					isULC2 = 1;
				case 'l':
					index ++;
					break;
				default:
					usage();
					return -1;
				}
			} else {
				stop = true;
			}
		} while (!stop && argc > 3+index);
	}

	if (argc < 3+index) {
		usage();
		return -1;
	}

	char tempPath[MAX_PATH];
	char tempFileName[MAX_PATH];
	GetTempPathA(MAX_PATH, tempPath);
	GetTempFileNameA(tempPath, "TFR", 0, tempFileName);

	FILE *newLgu = _wfopen(argv[2+index], _T("wb"));
	if (!newLgu) {
		wprintf(_T("Failed to create file %s\n"), argv[2+index]);
		return -1;
	}

	if (!DirectoryExists(argv[1+index])) {
		wprintf(_T("Input directory not found %s\n"), argv[1+index]);
		fclose(newLgu);
		return -1;
	}

	_wchdir(argv[1+index]);

	if (createZip(tempFileName, isULC2) < 0) {
		wprintf(_T("Failed to convert input folder %s\n"), argv[1+index]);
		fclose(newLgu);
		return -1;
	}

	int size_buf = WRITEBUFFERSIZE;
    void* buf = (void*)malloc(size_buf);

	unsigned long crc = 0;
	int err = getFileCrc(tempFileName, buf, size_buf, &crc);

	free(buf);

	FILE *tempFile = fopen(tempFileName, "rb");
	if (!tempFile) {
		printf("Failed to open file %s\n", tempFileName);
		remove(tempFileName);
		fclose(newLgu);
		return -1;
	}

	fseek(tempFile, 0, SEEK_END);
	unsigned long len = ftell(tempFile) + 1024;
	fseek(tempFile, 0, 0);

	unsigned char *newHeader = new unsigned char[1024];
	memcpy(newHeader, headerArr, 1024);

	if (isULC2 == 1)
		memcpy(newHeader, "ULC2", 4);

	memcpy(&newHeader[0xc], &len, 4);
	memcpy(&newHeader[0x18], &crc, 4);
	memset(&newHeader[0x1c], 0, 4);

	memcpy(&newHeader[0x2a0], label_name.c_str(), label_name.size()*2+2);	// label_name
	memcpy(&newHeader[0x2dc], argv[index], wcslen(argv[index])*2+2);		// content

	__time64_t time = _time64(0);
	memcpy(&newHeader[0x3f4], &time, 4);
	
	fwrite(newHeader, 1, 1024, newLgu);
	delete[] newHeader;

	char buffer[0x1000];
	int read = 0, countXor = 0;

	unsigned char *xorArr;
	if (isULC2 == 1)
		xorArr = xorArrULC2;
	else
		xorArr = xorArrLGU0;

	while ((read = fread(buffer, 1u, 0x1000u, tempFile)) > 0) {
		if (read) {
			for (int i=0;i<read;i++)
				buffer[i] ^= xorArr[countXor++ & 0x3FF];
		}

		fwrite(buffer, read, 1u, newLgu);
	}

	fclose(tempFile);
	fclose(newLgu);
	remove(tempFileName);

	return 0;
}