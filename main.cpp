#include <iostream>

#include "src/mlog.h"

#define MLOG_DISABLE 0

int main(){
	LOG_D(MLOG_TRACE, "before");

	LOG_ALL.setLogLevel(1, 1);

	LOG_INIT("log.txt", false);

	LOG(MLOG_INFO, "default", "begin: ", 5.324, " ", 9, " dfasfasf", " end");
	LOG_D(MLOG_ERROR, "begin: ", 666, " afasf", " end");
	LOG_D(MLOG_TRACE, "begin: ", 666, " end");
	LOG_D(MLOG_ERROR, "begin: ", " end");
	LOG_D(MLOG_INFO, "begin: ");
	LOG_D(MLOG_WARNING, 5);

	mLog::Instance()->log(MLOG_INFO, MLOG_DEFAULT_ALIAS, "begin: ", 5.324, " ", 9, " dfasfasf", " end");

	LOG_D_ERROR("ERROR");

	return 0;
}
