// lgu2dir.cpp : définit le point d'entrée pour l'application console.
//

#include <stdio.h>
#include <direct.h>
#include <iostream>
#include <Windows.h>
#include <tchar.h>

#include "xorArray.h"
#include "unzip.h"

#define WRITEBUFFERSIZE (8192)

void change_file_date(const char *filename, uLong dosdate, tm_unz tmu_date)
{
	HANDLE hFile;
	FILETIME ftm, ftLocal, ftCreate, ftLastAcc, ftLastWrite;

	hFile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	GetFileTime(hFile, &ftCreate, &ftLastAcc, &ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16), (WORD)dosdate, &ftLocal);
	LocalFileTimeToFileTime(&ftLocal, &ftm);
	SetFileTime(hFile, &ftm, &ftLastAcc, &ftm);
	CloseHandle(hFile);
}

int makedir(char *newdir)
{
	char *buffer ;
	char *p;
	int  len = (int)strlen(newdir);

	if (len <= 0)
		return 0;

	buffer = (char*)malloc(len+1);
	if (buffer==NULL) {
		printf("Error allocating memory\n");
		return UNZ_INTERNALERROR;
	}
	strcpy(buffer,newdir);

	if (buffer[len-1] == '/')
		buffer[len-1] = '\0';

	if (mkdir(buffer) == 0) {
		free(buffer);
		return 1;
	}

	p = buffer+1;
	while (1) {
		char hold;

		while(*p && *p != '\\' && *p != '/')
			p++;
		hold = *p;
		*p = 0;
		if ((mkdir(buffer) == -1) && (errno == ENOENT)) {
			printf("couldn't create directory %s\n", buffer);
			free(buffer);
			return 0;
		}
		if (hold == 0)
			break;
		*p++ = hold;
	}
	free(buffer);
	return 1;
}

int extractZip(const char *lguPath, const char *tempFileName) 
{
	int result = 0;

	FILE *lguFile = 0;
	fopen_s(&lguFile, lguPath, "rb");

	if (!lguFile)
		return -1;

	unsigned int magic;
	if (fread(&magic, 1u, 4u, lguFile) != 4) {
		fclose(lguFile);
		return -2;
	}
	
	switch (magic) {
		case 0x3055474C: // LGU0
			result = 0;
			break;
		case 0x32434C55: // ULC2
			result = 1;
			break;
		default:
			fclose(lguFile);
			return -2;
	}
	
	fseek(lguFile, 0, SEEK_END);
	if (ftell(lguFile) < 1032 || fseek(lguFile, 1024, SEEK_SET) || ftell(lguFile) != 1024) {
		fclose(lguFile);
		return -3;
	}
	
	FILE *tmpFile = fopen(tempFileName, "wb");
	if (!tmpFile) {
		fclose(lguFile);
		return -4;
	}
	
	char buffer[0x1000];
	int read = 0, countXor = 0;
	unsigned char *xorArr = 0;

	if (result == 1)
		xorArr = xorArrULC2;
	else
		xorArr = xorArrLGU0;

	while ((read = fread(buffer, 1u, 0x1000u, lguFile)) > 0) {
		if (read) {
			for (int i=0;i<read;i++)
				buffer[i] ^= xorArr[countXor++ & 0x3FF];
		}

		fwrite(buffer, read, 1u, tmpFile);
	}
		
	fclose(tmpFile);
	fclose(lguFile);

	return result;
}

int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite, const char* password)
{
    char filename_inzip[256];
    char* filename_withoutpath;
    char* p;
    int err=UNZ_OK;
    FILE *fout=NULL;
    void* buf;
    uInt size_buf;

    unz_file_info file_info;
    uLong ratio=0;
    err = unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

    if (err!=UNZ_OK) {
        printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
        return err;
    }

    size_buf = WRITEBUFFERSIZE;
    buf = (void*)malloc(size_buf);
    if (buf==NULL) {
        printf("Error allocating memory\n");
        return UNZ_INTERNALERROR;
    }

    p = filename_withoutpath = filename_inzip;
    while ((*p) != '\0') {
        if (((*p)=='/') || ((*p)=='\\'))
            filename_withoutpath = p+1;
        p++;
    }

    if ((*filename_withoutpath)=='\0') {
        if ((*popt_extract_without_path)==0) {
            printf("creating directory: %s\n",filename_inzip);
            mkdir(filename_inzip);
        }
    } else {
        const char* write_filename;
        int skip=0;

        if ((*popt_extract_without_path)==0)
            write_filename = filename_inzip;
        else
            write_filename = filename_withoutpath;

        err = unzOpenCurrentFilePassword(uf,password);
        if (err!=UNZ_OK)
            printf("error %d with zipfile in unzOpenCurrentFilePassword\n",err);

        if (((*popt_overwrite)==0) && (err==UNZ_OK)) {
            char rep=0;
            FILE* ftestexist;
            ftestexist = fopen(write_filename,"rb");
            if (ftestexist!=NULL) {
                fclose(ftestexist);
                do {
                    char answer[128];
                    int ret;

                    printf("The file %s exists. Overwrite ? [y]es, [n]o, [A]ll: ",write_filename);
                    ret = scanf("%1s",answer);
                    if (ret != 1) 
                       exit(EXIT_FAILURE);

                    rep = answer[0] ;
                    if ((rep>='a') && (rep<='z'))
                        rep -= 0x20;
                } while ((rep!='Y') && (rep!='N') && (rep!='A'));
            }

            if (rep == 'N')
                skip = 1;

            if (rep == 'A')
                *popt_overwrite=1;
        }

        if ((skip==0) && (err==UNZ_OK)) {
            fout = fopen(write_filename,"wb");

            /* some zipfile don't contain directory alone before file */
            if ((fout==NULL) && ((*popt_extract_without_path)==0) && (filename_withoutpath!=(char*)filename_inzip)) {
                char c = *(filename_withoutpath-1);
                *(filename_withoutpath-1)='\0';
                makedir((char*)write_filename);
                *(filename_withoutpath-1) = c;
                fout=fopen(write_filename, "wb");
            }

            if (fout==NULL)
                printf("error opening %s\n", write_filename);
        }

        if (fout!=NULL) {
            printf(" extracting: %s\n", write_filename);

            do {
                err = unzReadCurrentFile(uf, buf, size_buf);
                if (err < 0) {
                    printf("error %d with zipfile in unzReadCurrentFile\n",err);
                    break;
                }

                if (err > 0) {
                    if (fwrite(buf,err,1,fout) != 1) {
                        printf("error in writing extracted file\n");
                        err=UNZ_ERRNO;
                        break;
                    }
				}
            } while (err>0);

            if (fout)
                    fclose(fout);

            if (err==0)
                change_file_date(write_filename,file_info.dosDate, file_info.tmu_date);
        }

        if (err==UNZ_OK) {
            err = unzCloseCurrentFile (uf);
            if (err!=UNZ_OK)
                printf("error %d with zipfile in unzCloseCurrentFile\n",err);
        } else {
            unzCloseCurrentFile(uf); /* don't lose the error */
		}
    }

    free(buf);
    return err;
}

int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite, const char *password)
{
    uLong i;
    unz_global_info gi;
    int err;
    FILE* fout = NULL;

    err = unzGetGlobalInfo(uf,&gi);
    if (err != UNZ_OK)
        printf("error %d with zipfile in unzGetGlobalInfo \n",err);

    for (i=0;i<gi.number_entry;i++) {
        if (do_extract_currentfile(uf, &opt_extract_without_path, &opt_overwrite, password) != UNZ_OK)
            break;

        if ((i+1)<gi.number_entry) {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK) {
                printf("error %d with zipfile in unzGoToNextFile\n",err);
                break;
            }
        }
    }

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3) {
		printf("Usage: lgu2dir <lgu file> <out folder>\n");
		return -1;
	}

	char tempPath[MAX_PATH];
	char tempFileName[MAX_PATH];
	GetTempPathA(MAX_PATH, tempPath);
	GetTempFileNameA(tempPath, "TFR", 0, tempFileName);

	char lguPath[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, argv[1], wcslen(argv[1])+1, lguPath, MAX_PATH, NULL, NULL);

	int isULC2 = extractZip(lguPath, tempFileName);
	if (isULC2 < 0) {
		printf("Failed to open lgu file %s\n", lguPath);
		return -1;
	}

	unzFile zipfile = unzOpen(tempFileName);
	if (!zipfile) {
		printf("Failed to open archive %s\n", tempFileName);
		remove(tempFileName);
		return -1;
	}

	char outputPath[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, argv[2], wcslen(argv[2])+1, outputPath, MAX_PATH, NULL, NULL);

	if (outputPath[strlen(outputPath)-1] != '\\')
		strcat(outputPath, "\\");

	makedir(outputPath);
	_chdir(outputPath);

	if (isULC2 == 1)
		do_extract(zipfile, 0, 1, "u8u8l^-^8@E4g_H2kq8X_pack");
	else
		do_extract(zipfile, 0, 1, "I_LOVE_LG^^");

	unzClose(zipfile);
	remove(tempFileName);
	
	return 0;
}