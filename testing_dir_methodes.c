#include <stdio.h>
#include <unistd.h>     /* for chdir */
#include <stdlib.h>     /* for free */
#include <errno.h>      /* for errno */
#include <sys/types.h>  /* for opendir */
#include <dirent.h>     /* for opendir *
#include <string.h>     /* for strerror() */

void changeDir(void);

int main() {

    changeDir();

    /* protocols the error */
    int error;
    /* chage the current directory, note directory is relative to the current one */
    error = chdir("File_Handling");

    /* if an error ocured, print it out. */
    if (error) {
        perror("Error occured");
    }
        /* int the other case print the new current directory out. */
    else {
        changeDir();
    }

    /* return to the previous diretory */

    error = chdir("..");
    /* if an error ocured, print it out. */
    if (error) {
        perror("Error occured");
    }
        /* int the other case print the new current directory out. */
    else {
        changeDir();
    }

    error = openDirectory(".");

    return 0;
}

void changeDir(void) {
    char *baseDir = NULL;
    baseDir = getcwd(NULL, 0);
    fprintf(stdout, "New base Dir: %s\n", baseDir);
    /* free the new pointer coming from the system call */
    free(baseDir);
}

/**
 * @param dirName
 * @return int 0 in case in regular case. -1 in error case.
 *
 */
int openDirectory(const char *dirName) {
    /* declare a pointer to a DIR (directory datatype)
     * https://www.systutorials.com/docs/linux/man/3-opendir/  */
    DIR *directory;
    /* The opendir function opens and returns a directory stream for reading the directory whose file name is dirname. The stream has type DIR *.
    If unsuccessful, opendir returns a null pointer. In addition to the usual file name errors (see File Name Errors), the following errno error conditions are defined for this function: */

    /*        The readdir() function returns a pointer to a dirent structure
  representing the next directory entry in the directory stream pointed
  to by dirp.  It returns NULL on reaching the end of the directory
  stream or if an error occurred.
     http://man7.org/linux/man-pages/man3/readdir.3.html */
    struct dirent *pdirent;

    directory = opendir(dirName);

    if (directory == NULL) {
        perror("An error occurred");
        return -1;
    }

    while ((pdirent = readdir(directory)) != NULL) {
        printf("[%s]\n", pdirent->d_name);

        /* d_type This field contains a value indicating the file type, making
        it possible to avoid the expense of calling lstat(2) if fur‐
        ther actions depend on the type of the file. */

        if (pdirent->d_type == DT_DIR)
        {printf("Hurray this is a directory. save that name!\n");}
    }
    closedir(directory);

    return 0;

}


