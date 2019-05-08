#include "htp.h"

#include <cstdio>

int test_var = 0x42;

int main(int argc, char **argv)
{
    HTPServer server;
    
    printf("Testvar: %p\n", &test_var);
    if(!StartServer(&server))
    {
        puts("Error starting server");
    }
    return 0;
}