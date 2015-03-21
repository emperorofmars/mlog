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
** File:	mlog.cpp
** Project:	Mars Logging
*/

#include "mlog.h"

#ifndef MLOG_VARIABLES
#define MLOG_VARIABLES
	bool mLog::mInited = false;
	mLog mLog::mInstance;
#endif // MLOG_VARIABLES

//############################################################### set functions

mLog *mLog::Instance(){
	return &mInstance;
}

mLog::mLog(){
	output *tmp = new output;
	tmp->mAlias.push_back(MLOG_DEFAULT_ALIAS);
	tmp->mLogLevel = MLOG_ERROR_SIZE;
	std::stringstream ss;
	ss << "============================================================"
			<< std::endl << printTimeStamp(MLOG_DATE | MLOG_TIME)
			<< "Logging Started" << std::endl;
	tmp->mBuffer.push_back({std::vector<std::string>({ss.str()})});
	tmp->mBufferLogLevel.push_back(0);
	mOutput.push_back(std::shared_ptr<output>(tmp));

	mLogLevelConsole = MLOG_ERROR_SIZE;
	mLogLevelFile = MLOG_ERROR_SIZE;
	mFormat = MLOG_DATE | MLOG_TIME | MLOG_FILE | MLOG_LINE;
}

mLog::~mLog(){
}

int mLog::init(const char *defaultFile, bool append){
	mMutex.lock();
	mInited = true;

	setOutputFile(defaultFile, MLOG_DEFAULT_ALIAS, append);

	for(unsigned int i = 0; i < mOutput.size(); i++){
		for(unsigned int j = 0; j < mOutput[i]->mBuffer.size(); j++){
			std::stringstream ss;
			std::string msg;
			for(unsigned int k = 0; k < mOutput[i]->mBuffer[j].size(); k++){
				msg = mOutput[i]->mBuffer[j][k];
				realLog<std::string>(mOutput[i]->mBufferLogLevel[j], i, msg);
			}
			mOutput[i]->mBuffer.erase(mOutput[i]->mBuffer.begin() + j);
			mOutput[i]->mBufferLogLevel.erase(mOutput[i]->mBufferLogLevel.begin() + j);
			j--;
		}
	}
	mMutex.unlock();
	return 0;
}

int mLog::setOutputFile(const char *path, const char *alias, bool append){
	mMutex.lock();
	int ret = -1;
	std::string al(alias);
	int ia = searchOutput(alias);
	int ip = searchFile(path);
	if(ip >= 0){	//if file already open add alias
		if(ip != ia){
			while((ia = searchOutput(alias)) >= 0){	//delete alias already exists somwhere else
				for(unsigned int i = 0; i < mOutput[ia]->mAlias.size(); i++){
					if(mOutput[ip]->mAlias[i] == al){
						mOutput[ip]->mAlias.erase(mOutput[ip]->mAlias.begin() + i);
						break;
					}
				}
			}
			mOutput[ip]->mAlias.push_back(al);
		}
	}
	else if(ia >= 0){	//if alias already exists, open different file
		mOutput[ia]->mFileName = path;
		if(append) mOutput[ia]->mFile.open(path, std::ofstream::out | std::ofstream::app);
		else mOutput[ia]->mFile.open(path, std::ofstream::out);
		if(mOutput[ia]->mFile.is_open()) ret = 0;
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
			if(mOutput[ia]->mFile.is_open()) ret = 0;
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

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}

	realLog(logLevel, i, printTimeStamp(mFormat), printLogLevel(logLevel), t, args..., printEndl());

	mMutex.unlock();
	return 0;
}

template<typename T>
int mLog::log(int logLevel, const char *alias, T t){
	mMutex.lock();

	int i = searchOutput(alias);

	if(!mInited){
		mOutput[i]->mBuffer.push_back(std::vector<std::string>());
		mOutput[i]->mBufferLogLevel.push_back(logLevel);
	}

	realLog(logLevel, i, printTimeStamp(mFormat), printLogLevel(logLevel), t, printEndl());

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
	if(mInited){
		if(MLOG_DISABLE_CONSOLE_LOG != 1 && logLevel <= mLogLevelConsole){
			std::cerr << t;
		}
		if(MLOG_DISABLE_FILE_LOG != 1 && logLevel <= mLogLevelFile && output >= 0){
			mOutput[output]->mFile << t;
		}
	}
	else{
		std::stringstream ss;
		ss << t;
		unsigned int i;
		i = mOutput[output]->mBuffer.size();
		if(i > 0){
			mOutput[output]->mBuffer[i - 1].push_back(ss.str());
		}
	}
	return 0;
}

//############################################################### utility

int mLog::searchOutput(const char *alias){
	int res = -1;
	if(alias == NULL) return res;
	std::string al(alias);

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
