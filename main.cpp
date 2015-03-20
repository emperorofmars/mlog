#include <iostream>

#include "src/mlog.h"

#define MLOG_DISABLE 0

int main(){
	LOG_D(MLOG_TRACE, "before");
	//mLog::setOutputFile("log.txt", MLOG_DEFAULT_ALIAS);
	mLog::Instance()->setOutputFile("log.txt", MLOG_DEFAULT_ALIAS);

	LOG_INIT;

	LOG(MLOG_INFO, "default", "begin: ", 5.324, " ", 9, " dfasfasf", " end");
	LOG_D(MLOG_ERROR, "begin: ", 666, " afasf", " end");
	LOG_D(MLOG_TRACE, "begin: ", 666, " end");
	LOG_D(MLOG_ERROR, "begin: ", " end");
	LOG_D(MLOG_INFO, "begin: ");
	LOG_D(MLOG_WARNING, 5);


	mLog::Instance()->log(MLOG_INFO, MLOG_DEFAULT_ALIAS, "begin: ", 5.324, " ", 9, " dfasfasf", " end");

	return 0;
}
