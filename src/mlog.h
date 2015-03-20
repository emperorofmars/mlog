/*
**	Author:		Martin Schwarz
**	Name:		mlog.h
**	Project:	Mars Logging
**	Compile:	include in other project
*/

#ifndef MLOG_H
#define MLOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

#define DEFAULT_ALIAS "default"
#ifndef MLOG_DISABLE
	#define MLOG_DISABLE 0
#endif

enum logLevel{
	MLOG_ERROR = 0,
	MLOG_WARNING = 1,
	MLOG_INFO = 2,
	MLOG_TRACE = 3,
};

//static logging class
class mLog{
public:
	static int setOutputFile(const char *path, const char *alias);
	static int setLogLevel(int consoleLog, int fileLog);
	static int setLogLevelForFile(int fileLog, const char *alias);

	template<typename T, typename... Args>
		static int log(int logLevel, const char *alias, T t, Args... args);
	template<typename T, typename... Args>
		static int log(int logLevel, T t, Args... args);

private:
	//mLog();
	//~mLog();
	//mLog(const mLog &) = delete;
	//void operator =(const mLog &) = delete;

	template<typename T>
		static int realLog(int logLevel, T &t);
	template<typename T, typename... Args>
		static int realLog(int logLevel, T t, Args... args);

    static std::string getTimeStamp();

	static int searchOutput(const char *alias);

	struct output{
		std::string mFileName;
		std::string mAlias;
		std::ofstream mFile;
		int logLevel;
	};
	std::vector<std::shared_ptr<output>> mOutput;

	std::vector<std::vector<std::string>> mBuffer;

	static std::mutex mMutex;
	static int mLogLevelConsole;
	static int mLogLevelFile;
};

#ifndef MLOG_VARIABLES
#define MLOG_VARIABLES
	std::mutex mLog::mMutex;
	int mLog::mLogLevelConsole = 2;
	int mLog::mLogLevelFile = 2;
#endif // MLOG_VARIABLES

template<typename T, typename... Args>
int mLog::log(int logLevel, const char *alias, T t, Args... args){
	mMutex.lock();
	std::cerr << getTimeStamp() << " : ";
	realLog(logLevel, t);
	realLog(logLevel, args...);
	std::cerr << std::endl;
	mMutex.unlock();
	return 0;
}

template<typename T, typename... Args>
int mLog::log(int logLevel, T t, Args... args){
	return log(logLevel, DEFAULT_ALIAS, t, args...);
}

template<typename T>
int mLog::realLog(int logLevel, T &t){
	std::cerr << t;
	return 0;
};

template<typename T, typename... Args>
int mLog::realLog(int logLevel, T t, Args... args){
	realLog(logLevel, t);
	realLog(logLevel, args...);
	return 0;
};

std::string mLog::getTimeStamp(){
	time_t t = time(0);
	struct tm ts;
	char buf[20];
	ts = *localtime(&t);
	strftime(buf, sizeof(buf), "%d.%m.%Y-%X", &ts);
	std::string ret(buf);
	return ret;
}

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

#if MLOG_DISABLE == 1

#define LOG(level, output, ...) mLog::log(level, output, ...)
#define LOG_D(level, ...) mLog::log(level, ...)

#define LOG_ERROR(output, ...) mLog::log(MLOG_ERROR, output, ...)
#define LOG_WARNING(output, ...) mLog::log(MLOG_WARNING, output, ...)
#define LOG_INFO(output, ...) mLog::log(MLOG_INFO, output, ...)
#define LOG_TRACE(output, ...) mLog::log(MLOG_TRACE, output, ...)

#define LOG_D_ERROR(...) mLog::log(MLOG_ERROR, ...)
#define LOG_D_WARNING(...) mLog::log(MLOG_WARNING, ...)
#define LOG_D_INFO(...) mLog::log(MLOG_INFO, ...)
#define LOG_D_TRACE(...) mLog::log(MLOG_TRACE, ...)

#el

#define LOG(level, output, ...)
#define LOG_D(level, ...)

#define LOG_ERROR(output, ...)
#define LOG_WARNING(output, ...)
#define LOG_INFO(output, ...)
#define LOG_TRACE(output, ...)

#define LOG_D_ERROR(...)
#define LOG_D_WARNING(...)
#define LOG_D_INFO(...)
#define LOG_D_TRACE(...)

#endif // MLOG_DISABLE

#endif // MLOG_H






