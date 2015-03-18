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
#include <vector>
#include <cstdarg>

enum logLevel{
	MLOG_ERROR = 0,
	MLOG_WARNING = 1,
	MLOG_INFO = 2,
	MLOG_TRACE = 3,
};

class mLog{
public:
	mLog();
	~mLog();

	template<typename T>
	static int log(T &t){
		std::cerr << t;
		return 0;
	};

	template<typename T, typename... Args>
	static int log(T t, Args... args){
		log(t);
		log(args...);
		return 0;
	};
};

#endif // MLOG_H






