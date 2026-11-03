// LogSample.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"

#define WTLOG_ENABLED

#include "WT/System/Platform.h"
#include "WT/system/Logging/Log.h"

#ifdef _DEBUG
#pragma comment(lib, "..\\..\\..\\..\\..\\Build\\Debug\\WTPlatform1D.lib")
#else
#pragma comment(lib, "..\\..\\..\\..\\..\\Build\\Release\\WTPlatform1.lib")
#endif

using namespace WT;
using namespace System;
using namespace Logging;

int _tmain(int argc, _TCHAR* argv[])
{
	LOG_SET_WORDPAD(true, "log.doc");
	LOG_SET_FILTERLEVEL(99);
	LOG_SET_OPTIONS(ELogOption::LogLevel | ELogOption::LogFunction);

	LOG("Hello, World");
	LOG("Hello, World, %d", 2005);

	LOG(ELogColor::Blue, "Blue");
	LOG(ELogStyle::Bold, "Bold");
	LOG(ELogColor::Blue, ELogStyle::Bold, "Blue and Bold");
	LOG(ELogColor::Blue, ELogStyle::All, "Blue and Bold and Italic and Underline");

	LOG(0, "This is a level 0 message");
	LOG(1, "This is a level 1 message");
	LOG(2, "This is a level 2 message");
	LOG(2, ELogColor::Silver, "This is a level 2 message With Silver");
	LOG(3, ELogStyle::BoldAndUnderline, "More fancier message.");
	LOG(4, ELogColor::Red, ELogStyle::BoldAndUnderline, "Even more fancier message.");
	LOG(5, "Back to normal message. %.3f", 0.53434);

	return 0;
}

