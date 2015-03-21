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
	MLOG_FUNC = 0b00001000,
	MLOG_LINE = 0b00010000,
};

enum logTarget{
	MLOG_TARGET_FILE = 0b0001,
	MLOG_TARGET_CONSOLE = 0b0010,
	MLOG_TARGET_ALL = 0b0011,
};

//############################################################### singleton logging class

class mLog{
public:
	inline static mLog *Instance();

	inline int init(const char *defaultFile, bool append = true);

	inline int setOutputFile(const char *path, const char *alias, bool append = true);
	inline int setLogLevel(int consoleLog, int fileLog);
	inline int setLogLevelForFile(int fileLog, const char *alias);
	inline int setFormat(int format);

	template<typename T, typename... Args>
		inline int log(int logLevel, const char *alias, std::vector<std::string> posInfo, T t, Args... args);
	template<typename T>
		inline int log(int logLevel, const char *alias, std::vector<std::string> posInfo, T t);

	template<typename T, typename... Args>
		inline int log_f(int logLevel, const char *path, std::vector<std::string> posInfo, T t, Args... args);
	template<typename T>
		inline int log_f(int logLevel, const char *path, std::vector<std::string> posInfo, T t);

private:
	inline mLog();
	inline ~mLog();
	inline mLog(const mLog &) = delete;
	inline void operator =(const mLog &) = delete;

	template<typename T, typename... Args>
		inline int realLog(int target, int logLevel, int output, T t, Args... args);
	template<typename T>
		inline int realLog(int target, int logLevel, int output, T &t);

	inline int realSetOutputFile(const char *path, const char *alias, bool append = true);

	inline int searchOutput(const char *alias);
	inline int searchFile(const char *path);

	inline std::string printInfo(int info, int logLevel, std::vector<std::string> &posInfo);
	inline std::string printTimeStamp(int info);
	inline std::string printLogLevel(int logLevel);
	inline std::string printEndl();
	inline std::string printStartedMsg();

	struct output{
		std::string mFileName;
		std::vector<std::string> mAlias;
		std::ofstream mFile;
		int mLogLevel;

		struct buffer{
			std::vector<std::string> mMsg;
			int mTarget;
			int mLogLevel;
		};
		std::vector<std::vector<std::string>> mBuffer;	//use buffer
		std::vector<int> mBufferLogLevel;
	};
	std::vector<std::shared_ptr<output>> mOutput;

	std::mutex mMutex;
	std::mutex mInnerMutex;
	bool mInnerLock;
	int mLogLevelConsole;
	int mLogLevelFile;
	int mFormat;
	int mFormatConsole;
	int mFormatFile;

	bool mInited;
};

//############################################################### set functions

mLog *mLog::Instance(){
	static mLog mInstance;
	return &mInstance;
}

mLog::mLog(){
	output *tmp = new output;
	tmp->mAlias.push_back(MLOG_DEFAULT_ALIAS);
	tmp->mLogLevel = MLOG_ERROR_SIZE;
	tmp->mBufferLogLevel.push_back(0);
	mOutput.push_back(std::shared_ptr<output>(tmp));	//all other outputs

	mLogLevelConsole = MLOG_ERROR_SIZE;
	mLogLevelFile = MLOG_ERROR_SIZE;
	mFormat = MLOG_DATE | MLOG_TIME | MLOG_FILE | MLOG_LINE;
}

mLog::~mLog(){
}

int mLog::init(const char *defaultFile, bool append){
	mMutex.lock();
	mInnerMutex.lock();
	mInited = true;
	setOutputFile(defaultFile, MLOG_DEFAULT_ALIAS, append);

	for(unsigned int i = 0; i < mOutput.size(); i++){
		bool first = true;
		for(unsigned int j = 0; j < mOutput[i]->mBuffer.size(); j++){
			int tmplevel = mLogLevelConsole;
			if(first) mLogLevelConsole = -1;	//do not print logging started message
			std::stringstream ss;
			std::string msg;
			for(unsigned int k = 0; k < mOutput[i]->mBuffer[j].size(); k++){
				msg = mOutput[i]->mBuffer[j][k];
				realLog<std::string>(MLOG_TARGET_ALL, mOutput[i]->mBufferLogLevel[j], i, msg);
			}
			mOutput[i]->mBuffer.erase(mOutput[i]->mBuffer.begin() + j);
			mOutput[i]->mBufferLogLevel.erase(mOutput[i]->mBufferLogLevel.begin() + j);
			j--;
			if(first) mLogLevelConsole = tmplevel;
			first = false;
		}
	}
	mInnerMutex.unlock();
	mMutex.unlock();
	return 0;
}

int mLog::setOutputFile(const char *path, const char *alias, bool append){
	if(mInnerMutex.try_lock()) mMutex.lock();
	int ret = -1;
	if(alias != NULL && path != NULL)
		ret = realSetOutputFile(path, alias, append);
	if(mInnerMutex.try_lock()) mMutex.unlock();
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
int mLog::log(int logLevel, const char *alias, std::vector<std::string> posInfo, T t, Args... args){
	mMutex.lock();

	int i = searchOutput(alias);

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}
	realLog(MLOG_TARGET_ALL, logLevel, i, printInfo(mFormat, logLevel, posInfo), t, args..., printEndl());

	mMutex.unlock();
	return 0;
}

template<typename T>
int mLog::log(int logLevel, const char *alias, std::vector<std::string> posInfo, T t){
	mMutex.lock();

	int i = searchOutput(alias);

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}
	realLog(MLOG_TARGET_ALL, logLevel, i, printInfo(mFormat, logLevel, posInfo), t, printEndl());

	mMutex.unlock();
	return 0;
}

template<typename T, typename... Args>
int mLog::log_f(int logLevel, const char *path, std::vector<std::string> posInfo, T t, Args... args){
	mMutex.lock();

	int i = searchFile(path);
	if(i < 0){
		i = realSetOutputFile(path, NULL, false);
		if(!mOutput[i]->mFile.is_open()) i = -1;
	}

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}
	realLog(MLOG_TARGET_ALL, logLevel, i, printInfo(mFormat, logLevel, posInfo), t, args..., printEndl());

	mMutex.unlock();
	return 0;
}

template<typename T>
int mLog::log_f(int logLevel, const char *path, std::vector<std::string> posInfo, T t){
	mMutex.lock();

	int i = searchFile(path);
	if(i < 0){
		i = realSetOutputFile(path, NULL, false);
		if(!mOutput[i]->mFile.is_open()) i = -1;
	}

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}
	realLog(MLOG_TARGET_ALL, logLevel, i, printInfo(mFormat, logLevel, posInfo), t, printEndl());

	mMutex.unlock();
	return 0;
}

//############################################################### realLog

template<typename T, typename... Args>
int mLog::realLog(int target, int logLevel, int output, T t, Args... args){
	realLog(target, logLevel, output, t);
	realLog(target, logLevel, output, args...);
	return 0;
}

template<typename T>
int mLog::realLog(int target, int logLevel, int output, T &t){
	if(output < 0 || output >= mOutput.size()) return -1;
	if(mInited){
		if(MLOG_DISABLE_FILE_LOG != 1 && (target & MLOG_TARGET_FILE) > 0 && logLevel <= mLogLevelFile && output >= 0){
			mOutput[output]->mFile << t;
		}
		if(MLOG_DISABLE_CONSOLE_LOG != 1 && (target & MLOG_TARGET_CONSOLE) > 0 && logLevel <= mLogLevelConsole){
			std::cerr << t;
		}
	}
	else{
		std::stringstream ss;
		ss << t;
		unsigned int i = mOutput[output]->mBuffer.size();
		if(i > 0){
			mOutput[output]->mBuffer[i - 1].push_back(ss.str());
			mOutput[output]->mBufferLogLevel.push_back(logLevel);
		}
	}
	return 0;
}

//############################################################### utility

int mLog::realSetOutputFile(const char *path, const char *alias, bool append){
	int ret = -1;
	std::string al;
	if(alias != NULL) al = alias;
	int ia = searchOutput(alias);
	int ip = searchFile(path);
	if(ip >= 0 && ip != ia){	//if file already open add alias
		while((ia = searchOutput(alias)) >= 0){	//delete alias if already exists somwhere else
			for(unsigned int i = 0; i < mOutput[ia]->mAlias.size(); i++){
				if(mOutput[ia]->mAlias[i] == al){
					mOutput[ia]->mAlias.erase(mOutput[ia]->mAlias.begin() + i);
					break;
				}
			}
		}
		if(al.size() != 0) mOutput[ip]->mAlias.push_back(al);
		ret = ip;
	}
	else if(ia >= 0){	//if alias already exists, open different file
		mOutput[ia]->mFileName = path;
		if(append) mOutput[ia]->mFile.open(path, std::ofstream::out | std::ofstream::app);
		else mOutput[ia]->mFile.open(path, std::ofstream::out);
		if(mOutput[ia]->mFile.is_open()){
			if(!mInited){
				mOutput[ia]->mBuffer.push_back(std::vector<std::string>());
				mOutput[ia]->mBufferLogLevel.push_back(0);
			}
			std::string msg = printStartedMsg();
			realLog<std::string>(MLOG_TARGET_FILE, 0, ia, msg);
			ret = ia;
		}
	}
	else{
		mOutput.push_back(std::shared_ptr<output> (new output));
		ia = mOutput.size() - 1;
		if(mOutput[ia] != NULL){
			mOutput[ia]->mFileName = path;
			mOutput[ia]->mAlias.push_back(al);
			if(append) mOutput[ia]->mFile.open(path, std::ofstream::out | std::ofstream::app);
			else mOutput[ia]->mFile.open(path, std::ofstream::out);
			mOutput[ia]->mLogLevel = MLOG_ERROR_SIZE;
			if(mOutput[ia]->mFile.is_open()){
				if(!mInited){
					mOutput[ia]->mBuffer.push_back(std::vector<std::string>());
					mOutput[ia]->mBufferLogLevel.push_back(0);
				}
				std::string msg = printStartedMsg();
				realLog<std::string>(MLOG_TARGET_FILE, 0, ia, msg);
				ret = ia;
			}
		}
	}
	return ret;
}

int mLog::searchOutput(const char *alias){
	int res = -1;
	if(alias == NULL) return res;
	std::string al(alias);
	if(al.size() == 0) return res;

	for(unsigned int i = 0; i < mOutput.size(); i++){
		if(mOutput[i] == NULL){
			mOutput.erase(mOutput.begin() + i);
			i--;
		}
		for(unsigned int j = 0; j < mOutput[i]->mAlias.size(); j++){
			if(mOutput[i]->mAlias[j] == al){
				res = i;
				break;
			}
		}
		if(res >= 0) break;
	}
	return res;
}

int mLog::searchFile(const char *path){
	int res = -1;
	if(path == NULL) return res;
	std::string p(path);

	for(unsigned int i = 0; i < mOutput.size(); i++){
		if(mOutput[i] == NULL){
			mOutput.erase(mOutput.begin() + i);
			i--;
		}
		if(mOutput[i]->mFileName == p){
			res = i;
			break;
		}
	}
	return res;
}

std::string mLog::printInfo(int info, int logLevel, std::vector<std::string> &posInfo){
	std::stringstream ss;
	ss << printTimeStamp(info) << printLogLevel(logLevel);
	if(posInfo.size() == 3){
		ss << posInfo[0] << ": " << posInfo[1] << ": " << posInfo[2] << ": ";
	}
	return ss.str();
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

std::string mLog::printEndl(){
	std::stringstream ss;
	ss << std::endl;
	return ss.str();
}

std::string mLog::printStartedMsg(){
	std::stringstream ss;
	ss << "=========================================="
			<< std::endl << printTimeStamp(MLOG_DATE | MLOG_TIME)
			<< "Logging Started" << std::endl;
	return ss.str();
}

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

#undef MLOG_PRINT_POSITION

//#define MLOG_PRINT_POSITION __FILE__, ": ", __func__, ": ", __LINE__, " : "
#define MLOG_PRINT_POSITION std::vector<std::string> ({__FILE__, __func__, std::to_string(__LINE__)})

#define LOG_ALL (*mLog::Instance())
#define LOG_INIT mLog::Instance()->init

#define LOG(level, output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(level, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_F(level, output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log_f(level, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_D(level, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(level, MLOG_DEFAULT_ALIAS, MLOG_PRINT_POSITION, __VA_ARGS__)

#define LOG_ERROR(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_ERROR, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_WARNING(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_WARNING, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_INFO(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_INFO, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_TRACE(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_TRACE, output, MLOG_PRINT_POSITION, __VA_ARGS__)

#define LOG_F_ERROR(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log_f(MLOG_ERROR, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_F_WARNING(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log_f(MLOG_WARNING, output, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_F_INFO(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log_f(MLOG_INFO, output, MLOG_PRINT_POSITION,  __VA_ARGS__)
#define LOG_F_TRACE(output, ...) if(MLOG_DISABLE != 1) mLog::Instance()->log_f(MLOG_TRACE, output, MLOG_PRINT_POSITION,  __VA_ARGS__)

#define LOG_D_ERROR(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_ERROR, MLOG_DEFAULT_ALIAS, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_D_WARNING(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_WARNING, MLOG_DEFAULT_ALIAS, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_D_INFO(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_INFO, MLOG_DEFAULT_ALIAS, MLOG_PRINT_POSITION, __VA_ARGS__)
#define LOG_D_TRACE(...) if(MLOG_DISABLE != 1) mLog::Instance()->log(MLOG_TRACE, MLOG_DEFAULT_ALIAS, MLOG_PRINT_POSITION, __VA_ARGS__)

#endif // MLOG_H






