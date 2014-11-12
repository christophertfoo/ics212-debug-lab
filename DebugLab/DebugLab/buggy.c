/*****************************************************************
*
*  NAME:          Christopher Foo
*
*  HOMEWORK:      N/A
*
*  CLASS:         ICS 212
*
*  INSTRUCTOR:    Ravi Narayan
*
*  DATE:          November 10, 2014
*
*  FILE:          buggy.c
*
*  DESCRIPTION:   A simple, but buggy name database program.
*
*****************************************************************/

/* Disable security warnings raised by standard C library functions */
#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Constants */
#define true                  1
#define false                 0

#define DEFAULT_FILE          "names.txt"
#define INIT_LINE             10
#define INIT_NAMES            10

#define E_NO_ERROR            0
#define E_FILE_ERROR          1
#define E_ARGUMENT_ERROR      2
#define E_MEM_ERROR           3

/* Struct Definitions */
struct namelist
{
    char **names;
    unsigned int num_names;
    unsigned int capacity;
};

/* Prototypes */
int readfile(char file[], struct namelist *pList);
int writefile(char file[], struct namelist *pList);

int addname(struct namelist *pList, char *name);
int printnames(struct namelist *pList);
int findnames(struct namelist *pList, char *substring);

char *readline(FILE *fp);
int cleanup(struct namelist *pList);

/*****************************************************************
*
*  Function name:      main
*
*  DESCRIPTION:        Runs the user interface for the name database program.
*
*  Parameters:
*          int argc
*              The number of command line arguments including
*              the name of the program.
*
*          char ** argv
*              The command line arguments.  Uses one optional
*              command line argument which is the name of the
*              file to read from / write to.
*
*  Return values:
*          E_NO_ERROR   (0) = No error
*          E_FILE_ERROR (1) = Failed to open the database file
*          E_MEM_ERROR  (3) = Failed to allocate memory, program aborted
*
*****************************************************************/
int main(int argc, char **argv)
{
    char running;
    int errorCode, returnCode;
    struct namelist list;
    char *line, *pLastChar;
    char *file = argc > 1 ? argv[1] : DEFAULT_FILE;
    char command[6] = "";

    returnCode = E_NO_ERROR;
    list.capacity = 0;
    list.num_names = 0;
    list.names = NULL;

    errorCode = readfile(file, &list);
    if (errorCode == E_NO_ERROR || errorCode == E_FILE_ERROR)
    {
        running = true;
        while (running)
        {
            printf("Options:\n");
            printf("  add:   Add a name to the list\n");
            printf("  print: Print all names\n");
            printf("  find:  Finds all names that contain the given string\n");
            printf("  quit:  Exit the program\n");
            printf(">");

            fgets(command, 6, stdin);

            /* Clear the buffer if there is extra stuff */
            pLastChar = &command[strlen(command) - 1];
            if (*pLastChar != '\n')
            {
                while (getchar() != '\n')
                    ;
            }
            else
            {
                *pLastChar = '\0';
            }

            printf("\n");

            /* Add Name*/
            if (strncmp(command, "add", 4) == 0)
            {
                printf("Name to add: ");
                line = readline(stdin);
                if (line == NULL)
                {
                    printf("Fatal Error: Failed to read input.  Exiting.\n");
                    returnCode = E_MEM_ERROR;
                    running = false;
                }
                else
                {
                    errorCode = addname(&list, line);
                    if (errorCode != E_NO_ERROR)
                    {
                        printf("Error: Could not add the name.\n");
                        free(line);
                    }
                }
            }

            /* Print All Names */
            else if (strncmp(command, "print", 6) == 0)
            {
                printnames(&list);
            }

            /* Find Names */
            else if (strncmp(command, "find", 5) == 0)
            {
                printf("String to search for: ");
                line = readline(stdin);
                if (line == NULL)
                {
                    printf("Fatal Error: Failed to read input.  Exiting.\n");
                    returnCode = E_MEM_ERROR;
                    running = false;
                }
                else
                {
                    findnames(&list, line);
                    free(line);
                }
            }

            /* Quit */
            else if (strncmp(command, "quit", 5) == 0)
            {
                if (writefile(file, &list) != E_NO_ERROR)
                {
                    printf("Error: Could not write to the file \"%s\".\n", file);
                }
                cleanup(&list);
                running = false;
            }

            /* Bad Command */
            else
            {
                printf("Error: Unrecognized command \"%s\"\n", command);
            }

            printf("\n");
        }
    }
    else
    {
        printf("Fatal Error: Could not read the file \"%s\".  Exiting.\n", file);
        returnCode = E_FILE_ERROR;
    }

    return returnCode;
}

/*****************************************************************
*
*  Function name:      readfile
*
*  DESCRIPTION:        Reads the names from the provided database file
*                      into the given list.
*
*  Parameters:
*          char file[]
*              The name of the file to read from.
*
*          struct namelist *pList
*              A pointer to the namelist struct that the names should
*              be read into.
*
*  Return values:
*          E_NO_ERROR      (0) = No error
*          E_FILE_ERROR    (1) = Failed to open the database file
*          E_ARGUMENT_ERROR(2) = NULL parameter
*          E_MEM_ERROR     (3) = Failed to allocate memory, execution aborted
*
*****************************************************************/
int readfile(char file[], struct namelist *pList)
{
    unsigned int i;
    FILE *fp = NULL;
    int returnCode = E_NO_ERROR;
    if (file == NULL || pList == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        fp = fopen(file, "r");
        if (fp == NULL)
        {
            returnCode = E_FILE_ERROR;
        }
        else
        {
            /* Grab the number of names in the file */
            fscanf(fp, "%d\n", &pList->capacity);
            pList->names = (char **) malloc(sizeof(char *) * pList->capacity);

            if (pList->names == NULL)
            {
                returnCode = E_MEM_ERROR;
            }
            else
            {

                /* Read the names from the file */
                pList->num_names = 0;
                for (i = 0; i < pList->capacity && returnCode == E_NO_ERROR; i++)
                {
                    pList->names[i] = readline(fp);

                    /* Failed to read the name */
                    if (pList->names[i] == NULL)
                    {
                        /* Clean up the names before the one that failed */
                        cleanup(pList);
                        returnCode = E_MEM_ERROR;
                    }
                    else
                    {
                        pList->num_names++;
                    }
                }
            }
            fclose(fp);
        }
    }
    return returnCode;
}

/*****************************************************************
*
*  Function name:      writefile
*
*  DESCRIPTION:        Writes the names from the given list into
*                      a file with the given name.
*
*  Parameters:
*          char file[]
*              The name of the file to write to.
*
*          struct namelist *pList
*              A pointer to the namelist struct that contains the
*              names to be written.
*
*  Return values:
*          E_NO_ERROR      (0) = No error
*          E_FILE_ERROR    (1) = Failed to open the database file
*          E_ARGUMENT_ERROR(2) = NULL parameter
*          E_MEM_ERROR     (3) = Failed to allocate memory, execution aborted
*
*****************************************************************/
int writefile(char file[], struct namelist *pList)
{
    unsigned int i;
    FILE *fp;
    int returnCode = E_NO_ERROR;

    if (file == NULL || pList == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        fp = fopen(file, "w");
        if (fp == NULL)
        {
            returnCode = E_FILE_ERROR;
        }
        else
        {
            fprintf(fp, "%d\n", pList->num_names);
            for (i = 0; i < pList->num_names; i++)
            {
                fprintf(fp, "%s\n", pList->names[i]);
            }
            fclose(fp);
        }
    }
    return returnCode;
}

/*****************************************************************
*
*  Function name:      addname
*
*  DESCRIPTION:        Adds the given name to the given list.
*
*  Parameters:
*          struct namelist *pList
*              A pointer to the namelist struct that the name should
*              be added to.
*
*          char *name
*              The name to be added.
*
*  Return values:
*          E_NO_ERROR      (0) = No error
*          E_ARGUMENT_ERROR(2) = NULL parameter
*          E_MEM_ERROR     (3) = Failed to allocate memory, execution aborted
*
*****************************************************************/
int addname(struct namelist *pList, char *name)
{
    char **temp;
    int returnCode = E_NO_ERROR;

    if (pList == NULL || name == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        /* Initialize the empty list */
        if (pList->capacity == 0)
        {
            pList->names = (char **) malloc(sizeof(char *) * INIT_NAMES);
            if (pList->names == NULL)
            {
                returnCode = E_MEM_ERROR;
            }
            else
            {
                pList->capacity = INIT_NAMES;
            }
        }

        /* Resize if not empty and ran out of space */
        else if (pList->num_names >= pList->capacity)
        {
            temp = (char **) malloc(sizeof(char *) * pList->capacity * 2);

            /* Could not allocate the new array, error */
            if (temp == NULL)
            {
                returnCode = E_MEM_ERROR;
            }

            /* Allocation successful, use the new array */
            else
            {
                memcpy(temp, pList->names, sizeof(char *) * pList->capacity);
                free(pList->names);
                pList->names = temp;
                pList->capacity *= 2;
            }
        }

        /* Add the name if everything is fine */
        if (returnCode == E_NO_ERROR)
        {
            pList->names[pList->num_names] = name;
            pList->num_names++;
        }
    }

    return returnCode;
}

/*****************************************************************
*
*  Function name:      printnames
*
*  DESCRIPTION:        Prints all of the names in the given list.
*
*  Parameters:
*          struct namelist *pList
*              A pointer to the namelist struct that should be printed.
*
*  Return values:
*          E_NO_ERROR      (0) = No error
*          E_ARGUMENT_ERROR(2) = NULL parameter
*
*****************************************************************/
int printnames(struct namelist *pList)
{
    unsigned int i;
    int returnCode = E_NO_ERROR;
    if (pList == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        printf("Printing all %d names:\n", pList->num_names);
        for (i = 0; i < pList->num_names; i++)
        {
            printf("  [%d] %s\n", i + 1, pList->names[i]);
        }
    }
    return returnCode;
}

/*****************************************************************
*
*  Function name:      findnames
*
*  DESCRIPTION:        Prints all names in the given list that contain the
*                      given substring.
*
*  Parameters:
*          struct namelist *pList
*              A pointer to the namelist struct that should be searched.
*
*          char *substring
*              The substring to search for.
*
*  Return values:
*          E_NO_ERROR      (0) = No error
*          E_ARGUMENT_ERROR(2) = NULL parameter
*
*****************************************************************/
int findnames(struct namelist *pList, char *substring)
{
    unsigned int i;
    char found = false;
    int returnCode = E_NO_ERROR;

    if (pList == NULL || substring == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        printf("Printing all names containing \"%s\":\n", substring);
        for (i = 0; i < pList->num_names; i++)
        {
            if (strstr(pList->names[i], substring) != NULL)
            {
                printf("  [%d] %s\n", i + 1, pList->names[i]);
                found = true;
            }
        }

        if (!found)
        {
            printf("  No matches found.\n");
        }
    }
    return returnCode;
}

/*****************************************************************
*
*  Function name:      readline
*
*  DESCRIPTION:        Reads a full line from the given FILE * and
*                      returns a pointer to the string containing
*                      the line.  This line is allocated using
*                      heap memory and MUST be freed.
*
*  Parameters:
*          FILE *fp
*              The file to read from.
*
*  Return values:
*          A pointer to the string containing the line of input
*          or NULL if an error occurred.
*
*****************************************************************/
char *readline(FILE *fp)
{
    char *line, *temp, input;
    int current = 0;
    int size = INIT_LINE;

    if (fp == NULL)
    {
        line = NULL;
    }
    else
    {
        line = (char *) malloc(sizeof(char) * size);
        if (line != NULL)
        {
            for (input = fgetc(fp); input != '\n' && line != NULL; input = fgetc(fp))
            {

                /* Resize if we ran out of space */
                if (current >= size)
                {
                    temp = (char *) malloc(sizeof(char) * size * 2);

                    /* Could not allocate the new array, error */
                    if (temp == NULL)
                    {
                        free(line);
                        line = NULL;
                    }

                    /* Allocation successful, use the new array */
                    else
                    {
                        memcpy(temp, line, sizeof(char) * size);
                        free(line);
                        line = temp;
                        size *= 2;
                    }
                }

                if (line != NULL)
                {
                    line[current] = input;
                    current++;
                }
            }

            /* NUL-terminate the string if we didn't have an allocation problem */
            if (line != NULL)
            {
                line[current] = '\0';
            }
        }
    }
    return line;
}

/*****************************************************************
*
*  Function name:      cleanup
*
*  DESCRIPTION:        Empties the given list.
*
*  Parameters:
*          struct namelist *pList
*              The list to be emptied.
*
*  Return values:
*          E_NO_ERROR       (0) = No error
*          E_ARGUMENT_ERROR (1) = NULL parameter
*
*****************************************************************/
int cleanup(struct namelist *pList)
{
    unsigned int i;
    int returnCode = E_NO_ERROR;
    if (pList == NULL)
    {
        returnCode = E_ARGUMENT_ERROR;
    }
    else
    {
        for (i = 0; i < pList->num_names; i++)
        {
            free(pList->names[i]);
        }
        free(pList->names);

        pList->names = NULL;
        pList->num_names = 0;
        pList->capacity = 0;
    }
    return returnCode;
}
