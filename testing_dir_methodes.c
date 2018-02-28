#include <stdio.h>
#include <unistd.h>     /* for chdir */
#include <stdlib.h>     /* for free */
#include <errno.h>      /* for errno */
#include <sys/types.h>  /* for opendir */
#include <dirent.h>     /* for opendir *
#include <string.h>    /* for strerror() */
#include <memory.h>     /* for strerror() */

void changeDir(void);
int openDirectory(const char *dirName);

int main() {

    /* protocols the error */
    int error;

    error = openDirectory(".");

    if (error)
    {
        fprintf(stderr, "An error occured. %s\n", strerror(errno));
    }

    changeDir();


    /* chage the current directory, note directory is relative to the current one */
    error = chdir("test_dir/test/subTest_dir");

    /* if an error ocured, print it out. */
    if (error) {
        perror("Error occured - subTest_dir");
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

    char *filename;
    char path[100];
    /* declare a pointer to a DIR (directory datatype)
     * https://www.systutorials.com/docs/linux/man/3-opendir/  */
    /* The type DIR, which is defined in the header <dirent.h>, represents a directory stream, which is an ordered sequence of all the directory entries in a particular directory.
     * Directory entries represent files; files may be removed from a directory or added to a directory asynchronously to the operation of readdir(). */
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

    /* copy the root directory name to the path string */
    strcpy(path, dirName);

    if (directory == NULL) {
       fprintf(stderr, "An error occured during open dir: %s \n%s\n", dirName, strerror(errno));
        return -1;
    }

    while ((pdirent = readdir(directory)) != NULL) {
        printf("[%s]\n", pdirent->d_name);
       /* printf("User: [%s]", pdirent->) */

        /* d_type This field contains a value indicating the file type, making
        it possible to avoid the expense of calling lstat(2) if fur‐
        ther actions depend on the type of the file. */

        if (pdirent->d_type == DT_DIR)
        {
                        printf("Hoorray this is a directory. save that name!\n");
            filename = pdirent->d_name;
            printf("And tataa the pathname as string: %s\n", filename);

            /* test the recursion , leave out the "." and ".." directory */
            if (strcmp(filename, ".") && strcmp(filename, ".."))
            {

                sprintf(path, "%s/%s", path, filename);

                printf("Stepping into [%s]\n", path);
                openDirectory(path);
            }


        }
    }
    closedir(directory);

    return 0;

}


