#include <iostream>

#include "src/mlog.h"

int main()
{
    std::cout << "Hello world!" << std::endl;

    mLog::log("bla: ", 5.324, " ", 9, " dfasfasf\n");

    return 0;
}
