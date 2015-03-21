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
** File:	main.cpp
** Project:	Mars Logging
*/

#include <iostream>

#include "src/mlog.h"

#define MLOG_DISABLE 0

int main(){
	LOG_F(MLOG_TRACE, "bar.txt", "before ", 6666, " bla");
	LOG_F(MLOG_TRACE, "bar.txt", "before ", 9999, " bla");
	LOG_F(MLOG_TRACE, "log.txt", "before ", 9999, " bla");
	LOG_D(MLOG_TRACE, "before");

	//LOG_ALL.setLogLevel(2, 2);

	LOG_INIT("log.txt", false);

	LOG_ALL.setOutputFile("bla.txt", "bla", false);
	LOG_ALL.setOutputFile("bla.txt", "xxx");

	LOG(MLOG_INFO, "default", "begin: ", 5.324, " ", 9, " dfasfasf", " end");
	LOG_D(MLOG_ERROR, "begin: ", 666, " afasf", " end");
	LOG_D(MLOG_TRACE, "begin: ", 666, " end");
	LOG(MLOG_ERROR, "bla", "begin: ", 666, " TO BLA end");
	LOG(MLOG_INFO, "xxx", "begin: TO XXX");
	LOG_D(MLOG_WARNING, 5);

	//mLog::Instance()->log(MLOG_INFO, MLOG_DEFAULT_ALIAS, "begin: ", 5.324, " ", 9, " dfasfasf", " end");

	LOG_D_ERROR("ERROR");

	LOG_F(MLOG_ERROR, "foo.txt", "THIS GOES TO FOO: ", 6876876876, " ihfalfh");
	LOG_F(MLOG_ERROR, "foo.txt", "KAKAKAKA");
	LOG_F(MLOG_ERROR, "foo.txt", "KAKAKAKA");

	return 0;
}
