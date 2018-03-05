/** @file myFind.c
 * Betriebssysteme Assigment 1 - myFind
 * Beispiel 1
 * @author Valentin Platzgummer
 * @author Lara Kammerer
 *
 * @todo
 */

//* ------------------------------------------------------ includes -- */
#include <stdio.h>
#include <unistd.h>     /* for chdir */
#include <stdlib.h>     /* for free */
#include <errno.h>      /* for errno */
#include <sys/types.h>  /* for opendir */
#include <dirent.h>     /* for opendir */
#include <string.h>    /* for strerror() */
#include <memory.h>     /* for strerror() */

/* ------------------------------------------------------- functions -- */

/* ------------------------------------------------------- const char --*/
const char *print = "-print";
const char *user = "-user";
const char *uid ="-uid";
const char *type = "-type";
const char *name = "-name";
const char *ls = "-ls";

void printDir(void);
int openDirectory(const char *dirName);
int readDirectory (DIR *directory);

/** @brief implementation of a simplified find programm
 *
 * @param argc Number of arguments
 * @param argv actions used
 * @return
 */
int main(int argc, char **argv) {

    /* protocols the error */
    int error;

    /* if no arguments are provided, print out a use message, and exit with failure */
    if (argc < 2) {
        printf("usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]");
        exit(EXIT_FAILURE);
    }

    /* validate argv actions (begins always with a -) against menu action */
    for (int i = 0; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], print)  && strcmp(argv[i], user) &&  strcmp(argv[i], uid) && strcmp(argv[i], type) && strcmp(argv[i], name)
                    &&  strcmp(argv[i], ls))
            {
                printf("usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]");
                exit(EXIT_FAILURE);
            }
        }

        if (!(strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) && strcmp(argv[i], name)))
        {
            if (argc == i + 1)
            {
                printf("Missing argument for %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }

    }



    char *basePath = argv[1];
    /* call the openDirectory function for stepping though the directory */

    error = openDirectory(basePath);
    /* log out itf an error occured */
    if (error) {

        return 0;
    }
/* Function prints out the currend working directory. Same as "pwd" */
    void printDir(void) {
        char *baseDir = NULL;
        baseDir = getcwd(NULL, 0);
        fprintf(stdout, "New base Dir: %s\n", baseDir);
        /* free the new pointer coming from the system call */
        free(baseDir);
    }
}

/**
 * @param dirName
 * @return int 0 in case in regular case. -1 in error case.
 *
 */
int openDirectory(const char *dirName) {

    char *filename;
    /* define the path char array, start with a fixed length size
     * was 100 , has to be much longer trying 500 */
    /* FIXME: the char pointer path should be initialized with a dynamic length */
    char basePath[500];
    char pathCurrent[500];

    /* declare a pointer to a DIR (directory datatype) */
    DIR *directory;
    /* The opendir function opens and returns a directory stream for reading the directory whose file name is dirname. The stream has type DIR */
    struct dirent *pdirent;

    /* definition of opendir, https://github.com/lattera/glibc/blob/master/sysdeps/unix/opendir.c */
    directory = opendir(dirName);

    /* copy the root directory name to the path string, this is the root of the directory path  */
    strcpy(basePath, dirName);

    if (directory == NULL) {
        fprintf(stderr, "An error occurred during open dir: %s \n%s\n", dirName, strerror(errno));
        return -1;
    }

    /* definition of readdir */
    while ((pdirent = readdir(directory)) != NULL) {


        printf("Filename: [%s/%s]\n", basePath, pdirent->d_name);
        /* printf("User: [%s]", pdirent->) */

        if (pdirent->d_type == DT_DIR) {
            filename = pdirent->d_name;

            /* test the recursion , leave out the "." and ".." directory */
            if ((strcmp(filename, ".") != 0) && (strcmp(filename, "..") != 0)) {

                /* TODO: replace sprintf with snprintf (Buffer overflow) */

                sprintf(pathCurrent, "%s/%s", basePath, filename);

                /* start the recursion with each found directory */
                openDirectory(pathCurrent);
            }
        }
    }
    closedir(directory);


    return 0;
}

/* TODO: Add second function for reading the directory and print out the entries in the fashion needed by entries given the application */
/* a stub for the function directory is the pointer to the directory struct */
int readDirectory (DIR *directory)
{
    /* here comes the implementation of the new function */

    return 0;
}


//
// Created by marcaurel on 03.03.18.
//

