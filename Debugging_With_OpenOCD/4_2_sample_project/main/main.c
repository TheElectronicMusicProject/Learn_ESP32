#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef void (*function_pointer_t)(char *);

typedef struct params_struct
{
    char buffer[20];
    function_pointer_t funct_ptr;
} param_t;


void
called_with_funtion_pointer (char * data)
{
    printf("%s\n", data);

    return;
}

int32_t
foo (param_t *params)
{
    int32_t val = 1;

    for (int32_t idx = 0; idx < 10; idx++)
    {
        val += idx * val;
    }

    params->funct_ptr(params->buffer);
    memset(params->buffer, 5, 1024);
    val++;

    return val;
}

void
app_main (void)
{
    param_t params;
    sprintf(params.buffer, "hello world!");
    params.funct_ptr = called_with_funtion_pointer;
    int32_t result = foo(&params);
    printf("%d\n", result);

    return;
}