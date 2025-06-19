#include "src/mockup/mockup.h"

[[noreturn]] int main()
{
    setup();
    while(true)
    {
        loop();
    }
}
