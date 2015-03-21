/*
** Copyright (C) 2015 Martin Schwarz - All Rights Reserved
** You may use, distribute and modify this code under the
** terms of the MIT license.
**
** You should have received a copy of the MIT license with
** this file. If not, please write to: martin.schwarz10@gmail.com
*/

/*
** Author:	Martin Schwarz
** File:	mlog.h
** Project:	Mars Logging
** Compile:	include in other project
*/

#pragma once

#ifndef MLOG_H
#define MLOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <memory>
#include <mutex>

#ifndef MLOG_DEFAULT_ALIAS
	#define MLOG_DEFAULT_ALIAS "default"
#endif	// MLOG_DEFAULT_ALIAS
#ifndef MLOG_DISABLE
	#define MLOG_DISABLE 0
#endif	// MLOG_DISABLE
#ifndef MLOG_DISABLE_CONSOLE_LOG
	#define MLOG_DISABLE_CONSOLE_LOG 0
#endif	// MLOG_DISABLE_CONSOLE_LOG
#ifndef MLOG_DISABLE_FILE_LOG
	#define MLOG_DISABLE_FILE_LOG 0
#endif	// MLOG_DISABLE_FILE_LOG

enum logLevel{
	MLOG_ERROR = 0,
	MLOG_WARNING = 1,
	MLOG_INFO = 2,
	MLOG_TRACE = 3,
	MLOG_ERROR_SIZE = 4,
};

enum logInfo{
	MLOG_DATE = 0b00000001,
	MLOG_TIME = 0b00000010,
	MLOG_FILE = 0b00000100,
	MLOG_LINE = 0b00001000,
};

//############################################################### singleton logging class

class mLog{
public:
	static mLog *Instance();

	int init(const char *defaultFile, bool append = true);

	int setOutputFile(const char *path, const char *alias, bool append = true);
	int setLogLevel(int consoleLog, int fileLog);
	int setLogLevelForFile(int fileLog, const char *alias);
	int setFormat(int format);

	template<typename T, typename... Args>
		int log(int logLevel, const char *alias, T t, Args... args);
	template<typename T>
		int log(int logLevel, const char *alias, T t);

private:
	mLog();
	~mLog();
	mLog(const mLog &) = delete;
	void operator =(const mLog &) = delete;

	template<typename T, typename... Args>
		int realLog(int logLevel, int output, T t, Args... args);
	template<typename T>
		int realLog(int logLevel, int output, T &t);

	int searchOutput(const char *alias);
	int searchFile(const char *path);

	std::string printTimeStamp(int info);
	std::string printLogLevel(int logLevel);
	std::string printEndl();

	struct output{
		std::string mFileName;
		std::vector<std::string> mAlias;
		std::ofstream mFile;
		int mLogLevel;

		std::vector<std::vector<std::string>> mBuffer;
		std::vector<int> mBufferLogLevel;
	};
	std::vector<std::shared_ptr<output>> mOutput;

	std::mutex mMutex;
	int mLogLevelConsole;
	int mLogLevelFile;
	int mFormat;

	static bool mInited;
	static mLog mInstance;
};

//############################################################### macros

#undef LOG_ALL
#undef LOG_INIT

#undef LOG
#undef LOG_D

#undef LOG_ERROR
#undef LOG_WARNING
#undef LOG_INFO
#undef LOG_TRACE

#undef LOG_D_ERROR
#undef LOG_D_WARNING
#undef LOG_D_INFO
#undef LOG_D_TRACE

#define LOG_ALL (*mLog::Instance())
#define LOG_INIT mLog::Instance()->init

#define LOG(level, output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(level, output, __VA_ARGS__)
#define LOG_D(level, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(level, MLOG_DEFAULT_ALIAS, __VA_ARGS__)

#define LOG_ERROR(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_ERROR, output, __VA_ARGS__)
#define LOG_WARNING(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_WARNING, output, __VA_ARGS__)
#define LOG_INFO(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_INFO, output, __VA_ARGS__)
#define LOG_TRACE(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_TRACE, output, __VA_ARGS__)

#define LOG_D_ERROR(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_ERROR, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_WARNING(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_WARNING, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_INFO(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_INFO, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_TRACE(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_TRACE, MLOG_DEFAULT_ALIAS, __VA_ARGS__)

#endif // MLOG_H






