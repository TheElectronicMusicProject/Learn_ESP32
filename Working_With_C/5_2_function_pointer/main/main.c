#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

typedef void (* p_funct_t)(char *);

typedef struct Person_struct
{
    char first_name[20];
    char last_name[20];
    int32_t age;
    p_funct_t function;
} person_t;

static void
hello_amigo (char * p_char)
{
    printf("Ciao %s!\n", p_char);
}

static void
update_person (person_t * p_person, char * p_data)
{
    strcpy(p_person->first_name, "data from data");
    strcpy(p_person->last_name, "Fisher");
    p_person->age = 35;
    p_person->function = hello_amigo;

    p_person->function("some params");
}

static void
connect_and_get_info (char * p_url, p_funct_t function)
{
    char * p_data_retrieved = (char *) malloc(1024 * sizeof(char));
    function(p_data_retrieved);
    free((void *) p_data_retrieved);
}

static void
do_work_for_person (char * data)
{
    person_t person = {0};

    update_person(&person, data);

    printf("person: %s %s is %d years old\n",
           person.first_name, person.last_name, person.age);
}

void
app_main (void)
{
    connect_and_get_info("http://getperson.com", do_work_for_person);

}   /* app_main() */
