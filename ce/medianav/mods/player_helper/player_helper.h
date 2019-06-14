#pragma once
#include "stdafx.h"
#include "utils.h"

#define MODS_ROOT_PATH TEXT("\\Storage Card\\System\\mods\\")

static void globalEnvInit();
static void startOnce();

bool fixCodecsPath();
int extCheckMediaFilesExtList(const LPWSTR extValue);
int extCheckMediaFileMatch(const LPWSTR fileName);
int extCheckMediaFileMatch2(const LPWSTR fileName);

