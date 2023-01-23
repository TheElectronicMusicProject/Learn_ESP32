#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define TEST_4

#ifdef TEST_1
struct Person
{
    char first_name[20];
    char last_name[20];
    int32_t age;
};

void
app_main (void)
{
    struct Person person = {0};
    strcpy(person.first_name, "Bob");
    strcpy(person.last_name, "Fisher");
    person.age = 35;

    printf("person: %s %s is %d years old\n",
           person.first_name, person.last_name, person.age);
}   /* app_main() */
#elif defined(TEST_2)
typedef struct Person_struct
{
    char first_name[20];
    char last_name[20];
    int32_t age;
} person_t;

void
app_main (void)
{
    person_t person = {0};
    strcpy(person.first_name, "Bob");
    strcpy(person.last_name, "Fisher");
    person.age = 35;

    printf("person: %s %s is %d years old\n",
           person.first_name, person.last_name, person.age);
}   /* app_main() */
#elif defined(TEST_3)
typedef struct Person_struct
{
    char first_name[20];
    char last_name[20];
    int32_t age;
} person_t;

static void
update_person (person_t * person)
{
    strcpy(person->first_name, "Bob");
    strcpy(person->last_name, "Fisher");
    person->age = 35;
}

void
app_main (void)
{
    person_t person = {0};

    update_person(&person);

    printf("person: %s %s is %d years old\n",
           person.first_name, person.last_name, person.age);
}   /* app_main() */
#elif defined(TEST_4)
typedef struct Person_struct
{
    char first_name[20];
    char last_name[20];
    int32_t age;
} person_t;

static void
exclam_it (char * phrase)
{
    strcat(phrase, "!");
}

static void
update_person (person_t * person)
{
    strcpy(person->first_name, "Bob");
    strcpy(person->last_name, "Fisher");
    person->age = 35;
}

void
app_main (void)
{
    person_t person = {0};
    char phrase[20] = {"Hello World!"};

    update_person(&person);
    exclam_it(phrase);

    printf("person: %s %s is %d years old\n",
           person.first_name, person.last_name, person.age);
    printf("funtion output: %s\n", phrase);
}   /* app_main() */
#endif