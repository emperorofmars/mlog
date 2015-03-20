#include <iostream>

#include "src/mlog.h"

int main()
{
    std::cout << "Hello world!" << std::endl;

    mLog::log(1, "bla: ", 5.324, " ", 9, " dfasfasf");

    return 0;
}
