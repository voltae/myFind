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
#include <stdlib.h>     /* malloc and free */
#include <unistd.h>     /* for chdir */
#include <stdlib.h>     /* for free */
#include <errno.h>      /* for errno */
#include <sys/types.h>  /* for opendir */
#include <dirent.h>     /* for opendir */
#include <string.h>    /* for strerror() */
#include <sys/stat.h>
#include <unistd.h>

/* ------------------------------------------------------- functions -- */

/* ------------------------------------------------------- const char --*/
const char *print = "-print";
const char *user = "-user";
const char *uid = "-uid";
const char *type = "-type";
const char *name = "-name";
const char *ls = "-ls";

void printDir(void);

int do_dir(const char *dirName, const char **parms);

int do_file(const char *filename, const char **parms);

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
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], print) && strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) &&
                strcmp(argv[i], name)
                && strcmp(argv[i], ls)) {
                printf("usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]");
                exit(EXIT_FAILURE);
            }
        }

        if (!(strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) && strcmp(argv[i], name))) {
            if (argc == i + 1) {
                printf("Missing argument for %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }

    }


    char *basePath = argv[1];
    /* call the openDirectory function for stepping though the directory */

    error = do_dir(basePath, argv[2]);
    /* log out itf an error occured */
    if (error) {

        return 0;
    }
/* Function prints out the current working directory. Same as "pwd" */
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
int do_dir(const char *dirName, const char **param) {

    char *filename;
    /* define the path char array, start with a fixed length size
     * was 100 , has to be much longer trying 500 */
    /* FIXME: Don't forget the free */
    char *basePath = malloc(sizeof(dirName));
    char *filePath = NULL;

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
    if ((pdirent = readdir(directory)) != NULL) {
        int error = 0;
        char *filename = pdirent->d_name;
        /* create the full filepath for found file */
        /* FIXME: Don't forget the free the filepath */
        filePath = malloc((sizeof(basePath) + sizeof(filename) + sizeof(char)));
        error = (snprintf(filePath, sizeof(filePath), "%s/%s", basePath, filename));
        if (error < 0) {
            fprintf(stderr, "An error occurred, %s", strerror(errno));
        }

        /* call the do_file function with the file path */
        do_file(filePath, param);

        /*
            printf("Filename: [%s/%s]\n", basePath, pdirent->d_name);
            /* printf("User: [%s]", pdirent->)

            if (pdirent->d_type == DT_DIR) {
                filename = pdirent->d_name;

                / test the recursion , leave out the "." and ".." directory
                if ((strcmp(filename, ".") != 0) && (strcmp(filename, "..") != 0)) {

                    /* TODO: replace sprintf with snprintf (Buffer overflow)

                    sprintf(pathCurrent, "%s/%s", basePath, filename);

                    /* start the recursion with each found directory
                    openDirectory(pathCurrent, NULL;

                    */


        /* free the filePath created in here */
        free(filePath);
        filePath = NULL;
    }

    free(basePath);

    basePath = NULL;
    closedir(directory);


    return 0;
}

/* TODO: Add second function for reading the directory and print out the entries in the fashion needed by entries given the application */
/* a stub for the function directory is the pointer to the directory struct */
int do_file(const char *filename, const char **parms) {
    /* here comes the implementation of the new function */
    /* st is a struct of type stat for each file, in the stat struct is the information about the file */
    struct stat *st;

    /* iterator for while loop */
    int i = 0;

    /* FIXME: Das ist momentan eine Endlosschleife */
    while (parms[i] != NULL) {
        if (stat(filename, st) == -1) {
            fprintf(stderr, "Error in stat: %s", strerror(errno));

            /* in case of failure return to the do_dir function and read in the next entry */
            return;
        }

        printf("File path: %s", filename);

    }


    /* assign the filestats to the current file */
    if (stat(filename, st) == -1) {
        fprintf(stderr, "Error in stat: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Filename: %s\n", filename);

    if (S_ISDIR(st->st_mode)) {
        printf("this is a directory!\n");
    }
}


//
// Created by marcaurel on 03.03.18.
//

