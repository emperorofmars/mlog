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
#include <sstream>
#include <vector>
#include <memory>
#include <mutex>

#ifndef DEFAULT_ALIAS
	#define DEFAULT_ALIAS "default"
#endif	// DEFAULT_ALIAS

#ifndef MLOG_DISABLE
	#define MLOG_DISABLE 0
#endif	// MLOG_DISABLE

enum logLevel{
	MLOG_ERROR = 0,
	MLOG_WARNING = 1,
	MLOG_INFO = 2,
	MLOG_TRACE = 3,
};

enum logInfo{
	MLOG_DATETIME = 0b00000001,
	MLOG_FILE = 	0b00000010,
	MLOG_LINE = 	0b00000100,
};

//############################################################### static logging class

class mLog{
public:
	static int setOutputFile(const char *path, const char *alias);
	static int setLogLevel(int consoleLog, int fileLog);
	static int setLogLevelForFile(int fileLog, const char *alias);

	template<typename T, typename... Args>
		static int log(int logLevel, const char *alias, T t, Args... args);
	template<typename T>
		static int log(int logLevel, const char *alias, T t);

private:
	//mLog();
	//~mLog();
	//mLog(const mLog &) = delete;
	//void operator =(const mLog &) = delete;

	template<typename T, typename... Args>
		static int realLog(int logLevel, int output, T t, Args... args);
	template<typename T>
		static int realLog(int logLevel, int output, T &t);

	static int searchOutput(const char *alias);

    static std::string printTimeStamp();
    static std::string printLogLevel(int logLevel);

	struct output{
		std::string mFileName;
		std::string mAlias;
		std::ofstream mFile;
		int logLevel;
	};
	static std::vector<std::shared_ptr<output>> mOutput;

	static std::vector<std::vector<std::string>> mBuffer;

	static std::mutex mMutex;
	static int mLogLevelConsole;
	static int mLogLevelFile;
};

#ifndef MLOG_VARIABLES
#define MLOG_VARIABLES
	std::vector<std::shared_ptr<mLog::output>> mLog::mOutput;
	std::vector<std::vector<std::string>> mLog::mBuffer;

	std::mutex mLog::mMutex;
	int mLog::mLogLevelConsole = 2;
	int mLog::mLogLevelFile = 2;
#endif // MLOG_VARIABLES

//############################################################### set functions

int mLog::setOutputFile(const char *path, const char *alias){
	mMutex.lock();
	int ret = -1;
	int i = searchOutput(alias);
	if(i >= 0){
		mOutput[i]->mFileName = path;
		mOutput[i]->mFile.open(path, std::ofstream::out | std::ofstream::app);
		if(mOutput[i]->mFile.is_open()) ret = 0;
	}
	else{
		mOutput.push_back(std::shared_ptr<output> (new output));
		i = mOutput.size() - 1;
		if(mOutput[i] != NULL){
			mOutput[i]->mFileName = path;
			mOutput[i]->mAlias = alias;
			mOutput[i]->mFile.open(path, std::ofstream::out | std::ofstream::app);
			if(mOutput[i]->mFile.is_open()) ret = 0;
		}
	}
	mMutex.unlock();
	return ret;
}

int mLog::setLogLevel(int consoleLog, int fileLog){
	mMutex.lock();

	mMutex.unlock();
	return 0;
}

int mLog::setLogLevelForFile(int fileLog, const char *alias){
	mMutex.lock();

	mMutex.unlock();
	return 0;
}

//############################################################### public: log

template<typename T, typename... Args>
int mLog::log(int logLevel, const char *alias, T t, Args... args){
	mMutex.lock();

	int i = searchOutput(alias);

	std::stringstream ss;
	std::string msg;
	ss << printTimeStamp() << printLogLevel(logLevel);
	msg = ss.str();
	realLog<std::string> (logLevel, i, msg);

	realLog(logLevel, i, t);
	realLog(logLevel, i, args...);

	ss.str("");
	ss << std::endl;
	msg = ss.str();
	realLog<std::string> (logLevel, i, msg);

	mMutex.unlock();
	return 0;
}

template<typename T>
int mLog::log(int logLevel, const char *alias, T t){
	mMutex.lock();

	int i = searchOutput(alias);

	std::stringstream ss;
	std::string msg;
	ss << printTimeStamp() << printLogLevel(logLevel);
	msg = ss.str();
	realLog<std::string> (logLevel, i, msg);

	realLog(logLevel, i, t);

	ss.str("");
	ss << std::endl;
	msg = ss.str();
	realLog<std::string> (logLevel, i, msg);

	mMutex.unlock();
	return 0;
}

//############################################################### realLog

template<typename T, typename... Args>
int mLog::realLog(int logLevel, int output, T t, Args... args){
	realLog(logLevel, output, t);
	realLog(logLevel, output, args...);
	return 0;
}

template<typename T>
int mLog::realLog(int logLevel, int output, T &t){
	std::cerr << t;		//do the real stuff
	return 0;
}

//############################################################### utility

int mLog::searchOutput(const char *alias){
	int res = -1;
	std::string al(alias);

	for(unsigned int i = 0; i < mOutput.size(); i++){
		if(mOutput[i] == NULL){
			mOutput.erase(mOutput.begin() + i);
			i--;
		}
		if(mOutput[i]->mAlias == al){
			res = i;
			break;
		}
	}
	return res;
}

std::string mLog::printTimeStamp(){
	time_t t = time(0);
	struct tm ts;
	char buf[20];
	ts = *localtime(&t);
	strftime(buf, sizeof(buf), "%d.%m.%Y-%X", &ts);
	std::string ret(buf);
	ret.append(" : ");
	return ret;
}

std::string mLog::printLogLevel(int logLevel){
	std::string s;
	switch(logLevel){
		case MLOG_ERROR: 	s.append("ERROR   : "); break;
		case MLOG_WARNING:	s.append("WARNING : "); break;
		case MLOG_TRACE:	s.append("TRACE   : "); break;
		case MLOG_INFO: 	s.append("INFO    : "); break;
	}
	return s;
}

//############################################################### macros

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

#define LOG(level, output, ...) if(MLOG_DISABLE != 1) mLog::log(level, output, __VA_ARGS__)
#define LOG_D(level, ...) if(MLOG_DISABLE != 1) mLog::log(level, DEFAULT_ALIAS, __VA_ARGS__)

#define LOG_ERROR(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_ERROR, output, __VA_ARGS__)
#define LOG_WARNING(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_WARNING, output, __VA_ARGS__)
#define LOG_INFO(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_INFO, output, __VA_ARGS__)
#define LOG_TRACE(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_TRACE, output, __VA_ARGS__)

#define LOG_D_ERROR(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_ERROR, DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_WARNING(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_WARNING, DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_INFO(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_INFO, DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_TRACE(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_TRACE, DEFAULT_ALIAS, __VA_ARGS__)

#endif // MLOG_H






