#include <stdio.h>
#include <stdint.h>
#include "calc.h"

void
app_main (void)
{
    int32_t result = add(3, 5);

    printf("result = %d\n", result);
}   /* app_main() */