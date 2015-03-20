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

//############################################################### static logging class

class mLog{
public:
	static int init();

	static int setOutputFile(const char *path, const char *alias, bool append = true);
	static int setLogLevel(int consoleLog, int fileLog);
	static int setLogLevelForFile(int fileLog, const char *alias);
	static int setFormat(int format);

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

    static std::string printTimeStamp(int info);
    static std::string printLogLevel(int logLevel);

	struct output{
		std::string mFileName;
		std::string mAlias;
		std::ofstream mFile;
		int mLogLevel;

		std::vector<std::vector<std::string>> mBuffer;
	};
	static std::vector<std::shared_ptr<output>> mOutput;

	static std::mutex mMutex;
	static int mLogLevelConsole;
	static int mLogLevelFile;
	static int mFormat;

	static bool mInited;
};

#ifndef MLOG_VARIABLES
#define MLOG_VARIABLES
	std::vector<std::shared_ptr<mLog::output>> mLog::mOutput;
	std::mutex mLog::mMutex;
	int mLog::mLogLevelConsole = MLOG_ERROR_SIZE;
	int mLog::mLogLevelFile = MLOG_ERROR_SIZE;
	int mLog::mFormat = MLOG_DATE | MLOG_TIME | MLOG_FILE | MLOG_LINE;

	bool mLog::mInited = false;
#endif // MLOG_VARIABLES

//############################################################### set functions

int mLog::init(){
	mMutex.lock();
/*
	for(unsigned int i = 0; i < mBuffer.size(); i++){
		for(unsigned int j = 0; j < mBuffer[i].size(); j++){
			//
		}
	}
*/
	mInited = true;
	mMutex.unlock();
	return 0;
}

int mLog::setOutputFile(const char *path, const char *alias, bool append){
	mMutex.lock();
	int ret = -1;
	int i = searchOutput(alias);
	if(i >= 0){
		mOutput[i]->mFileName = path;
		if(append) mOutput[i]->mFile.open(path, std::ofstream::out | std::ofstream::app);
		else mOutput[i]->mFile.open(path, std::ofstream::out);
		if(mOutput[i]->mFile.is_open()) ret = 0;
	}
	else{
		mOutput.push_back(std::shared_ptr<output> (new output));
		i = mOutput.size() - 1;
		if(mOutput[i] != NULL){
			mOutput[i]->mFileName = path;
			mOutput[i]->mAlias = alias;
			if(append) mOutput[i]->mFile.open(path, std::ofstream::out | std::ofstream::app);
			else mOutput[i]->mFile.open(path, std::ofstream::out);
			mOutput[i]->mLogLevel = MLOG_ERROR_SIZE;
			if(mOutput[i]->mFile.is_open()) ret = 0;
		}
	}
	mMutex.unlock();
	return ret;
}

int mLog::setLogLevel(int consoleLog, int fileLog){
	mMutex.lock();
	mLogLevelConsole = consoleLog;
	mLogLevelFile = fileLog;
	mMutex.unlock();
	return 0;
}

int mLog::setLogLevelForFile(int fileLog, const char *alias){
	mMutex.lock();
	int i = searchOutput(alias);
	if(i >= 0) mOutput[i]->mLogLevel = fileLog;
	mMutex.unlock();
	return 0;
}

int mLog::setFormat(int format){
	mFormat = format;
	return 0;
}
//############################################################### public: log

template<typename T, typename... Args>
int mLog::log(int logLevel, const char *alias, T t, Args... args){
	mMutex.lock();

	int i = searchOutput(alias);

	std::stringstream ss;
	std::string msg;
	ss << printTimeStamp(mFormat) << printLogLevel(logLevel);
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
	ss << printTimeStamp(mFormat) << printLogLevel(logLevel);
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
	if(MLOG_DISABLE_CONSOLE_LOG != 1 && logLevel <= mLogLevelConsole){
		std::cerr << t;
	}
	if(MLOG_DISABLE_FILE_LOG != 1 && logLevel <= mLogLevelFile && output >= 0){
		mOutput[output]->mFile << t;
	}
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

std::string mLog::printTimeStamp(int info){
	time_t t = time(0);
	struct tm ts;
	char buf[23];
	memset(buf, 0, 23 * sizeof(char));
	ts = *localtime(&t);

	if((info & MLOG_DATE) > 0 && (info & MLOG_TIME) > 0){
		strftime(buf, sizeof(buf), "%d.%m.%Y-%X : ", &ts);
	}
	else if((info & MLOG_DATE) > 0){
		strftime(buf, sizeof(buf), "%d.%m.%Y : ", &ts);
	}
	else if((info & MLOG_TIME) > 0){
		strftime(buf, sizeof(buf), "%X : ", &ts);
	}
	std::string ret(buf);
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
#define LOG_D(level, ...) if(MLOG_DISABLE != 1) mLog::log(level, MLOG_DEFAULT_ALIAS, __VA_ARGS__)

#define LOG_ERROR(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_ERROR, output, __VA_ARGS__)
#define LOG_WARNING(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_WARNING, output, __VA_ARGS__)
#define LOG_INFO(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_INFO, output, __VA_ARGS__)
#define LOG_TRACE(output, ...) if(MLOG_DISABLE != 1) mLog::log(MLOG_TRACE, output, __VA_ARGS__)

#define LOG_D_ERROR(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_ERROR, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_WARNING(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_WARNING, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_INFO(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_INFO, MLOG_DEFAULT_ALIAS, __VA_ARGS__)
#define LOG_D_TRACE(...) if(MLOG_DISABLE != 1) mLog::log(MLOG_TRACE, MLOG_DEFAULT_ALIAS, __VA_ARGS__)

#endif // MLOG_H






