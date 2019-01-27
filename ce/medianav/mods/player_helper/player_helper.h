// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PLAYER_HELPER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PLAYER_HELPER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PLAYER_HELPER_EXPORTS
#define PLAYER_HELPER_API __declspec(dllexport)
#else
#define PLAYER_HELPER_API __declspec(dllimport)
#endif

//#define TESTMODE
#define STABLE_ONLY

#ifdef TESTMODE
#ifdef STABLE_ONLY
#undef STABLE_ONLY
#endif
#endif

#include "stdafx.h"
#include "utils.h"
//#include <pwinuser.h>

static void globalEnvInit();


PLAYER_HELPER_API bool fixCodecsPath();
PLAYER_HELPER_API void test();
PLAYER_HELPER_API int extCheckMediaFilesExtList(const LPWSTR extValue);

PLAYER_HELPER_API int extCheckMediaFileMatch(const LPWSTR fileName);
PLAYER_HELPER_API int extCheckMediaFileMatch2(const LPWSTR fileName);


/*class ModsWindowsHook : public Utils::SystemWideUniqueInstance
{
public:
    ModsWindowsHook();
    virtual ~ModsWindowsHook();
private:
    void WindowProcHook(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK WindowProcHookLink(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK hHook_;
    static ModsWindowsHook* self;
};
*/