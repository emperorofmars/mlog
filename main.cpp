#include <iostream>

#include "src/mlog.h"

int main(){

	mLog::log(MLOG_INFO, "bla: ", 5.324, " ", 9, " dfasfasf");
	mLog::log(MLOG_ERROR, "huehuehue ", 666, " afasf");
	mLog::log(MLOG_ERROR, "huehuehue ", 666);
	mLog::log(MLOG_ERROR, "huehuehue ");

	return 0;
}
