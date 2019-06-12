#pragma once
#include "stdafx.h"
#include "utils.h"

static void globalEnvInit();
static void startOnce();

bool fixCodecsPath();
int extCheckMediaFilesExtList(const LPWSTR extValue);
int extCheckMediaFileMatch(const LPWSTR fileName);
int extCheckMediaFileMatch2(const LPWSTR fileName);

