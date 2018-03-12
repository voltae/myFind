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
#include <math.h>       /* log() */
#include <unistd.h>     /* for chdir */
#include <stdlib.h>     /* for free */
#include <errno.h>      /* for errno */
#include <sys/types.h>  /* for opendir */
#include <dirent.h>     /* for opendir */
#include <string.h>    /* for strerror() */
#include <sys/stat.h>
#include <pwd.h>        /* getpwuid() */
#include <unistd.h>



/* ------------------------------------------------------- defines -----*/
#define PATHDIVIDER "/"

/* ------------------------------------------------------- functions -- */
void printDir(void);

int readUID(uid_t uid, const char *name);

char *isNameInFilename (const char *filePath, const char *name);

int do_dir(const char *dirName, const char **parms);

int do_file(const char *filename, const char **parms);

/* ------------------------------------------------------- enums -------*/
typedef enum {
    regOut, noOut, lsOut
} output;
typedef enum {
    loopCont, loopExit
} do_loop;

/* ------------------------------------------------------- const char --*/
const char *print = "-print";
const char *user = "-user";
const char *uid = "-uid";
const char *type = "-type";
const char *name = "-name";
const char *ls = "-ls";


/** @brief implementation of a simplified find programm
 *
 * @param argc Number of arguments
 * @param argv actions used
 * @return
 */
int main(int argc, const char **argv) {
    
    /* protocols the error */
    int error;
    
    /* if no arguments are provided, print out a use message, and exit with failure */
    if (argc < 2) {
        printf("usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]\n");
        exit(EXIT_FAILURE);
    }
    
    /* validate argv actions (begins always with a -) against menu action */
    for (int i = 0; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], print) && strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) &&
                strcmp(argv[i], name)
                && strcmp(argv[i], ls)) {
                printf("usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]\n");
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
    
    
    /* call the openDirectory function for stepping though the directory */
    
    error = do_file(argv[1], &argv[2]);
    /* log out itf an error occured */
    if (error) {
        
        return 0;
    }
}

/**
 * @param dirName char Pointer holding path of the found directory.
 * @param param Pointer of charpointer holding the actions.
 * @return int 0 in case in regular case. -1 in error case.
 *
 */
int do_dir(const char *dirName, const char **param) {
    
    /* Error was produced by this line: char *basePath = malloc(sizeof(dirName)); */
    /* basePath was f course way to short for holding the entire string, now corrected */
    char *basePath = malloc((strlen(dirName) + 1) * sizeof(char));
    char *filePath = NULL;
    
    /* declare a pointer to a DIR (directory datatype) */
    DIR *directory;
    /* The opendir function opens and returns a directory stream for reading the directory whose file name is dirname. The stream has type DIR */
    struct dirent *pdirent;
    
    /* definition of opendir, https://github.com/lattera/glibc/blob/master/sysdeps/unix/opendir.c */
    directory = opendir(dirName);
    
    /* copy the root directory name to the path string, this is the root of the directory path,
     * copies the most length of dirName char, buffer overflow, don't forget the place for the last terminating '\0' */
    strncpy(basePath, dirName, strlen(dirName) + 1);
    
    
    if (directory == NULL) {
        fprintf(stderr, "An error occurred during open dir: %s \n%s\n", dirName, strerror(errno));
        return -1;
    }
    
    /* definition of readdir */
    while ((pdirent = readdir(directory)) != NULL) {
        int error = 0;
        char *filename = pdirent->d_name;
        
        /* leave out the "." and ".." */
        
        if (!strcmp(filename, ".") || !strcmp(filename, ".."))
            continue;
        
        /* create the full filepath for found file, add 2 extra spaces for  '/' and '\0' */
        int lengthOfPath = (int) (strlen(filename) + (int) strlen(basePath) + 2);
        
        if (!(filePath = malloc(lengthOfPath * sizeof(char)))) {
            fprintf(stderr, "Memory error: %s\n", strerror(errno));
        }
        
        /*  int snprintf(char *str, size_t size, const char *format, ...);  creates the path in the buffer filePath at most lengthOfPath
         * characters long, buffer overflow */
        error = (snprintf(filePath, lengthOfPath, "%s/%s", basePath, filename));
        if (error < 0) {
            fprintf(stderr, "An error occurred, %s\n", strerror(errno));
        }
        /*
         strncat(filePath, basePath, strlen(basePath));
         strncat(filePath, PATHDIVIDER, strlen(PATHDIVIDER));
         strncat(filePath, filename, strlen(filename));
         */
        
        /* call the do_file function with the file path */
        do_file(filePath, param);
        
        // printf("Filename: [%s/%s]\n", basePath, pdirent->d_name);
        // printf("File is Directory: %s\n", (pdirent->d_type == DT_DIR)? "yes" : "no");
        
        /* free the filePath created in here */
        free(filePath);
        filePath = NULL;
        filename = NULL;
        
    }
    
    /* free the outer basePath */
    free(basePath);
    basePath = NULL;
    closedir(directory);
    
    return 0;
}

/* A second function for reading the directory and print out the entries in the fashion needed by entries given the application */
int do_file(const char *filename, const char **parms) {
    /* here comes the implementation of the new function */
    /* st is a struct of type stat for each file, in the stat struct is the information about the file */
    struct stat st;
    
    char *userName;
    /* variable for the final output */
    output out = regOut;
    do_loop looping = loopCont;
    
    /* iterator for while loop */
    int i = 0;
    
    /* read out the struct for stat */
    if (stat(filename, &st) == -1) {
        fprintf(stderr, "Error in stat: %s\n", strerror(errno));
        
        /* in case of failure return to the do_dir function and read in the next entry */
        return -1;
    }
    
    do
    {
        /* Lara did this */
        if (!parms[i]);
        else if (!strcmp(parms[i], print)) i++;
        
        else if (!strcmp(parms[i], name))
        {
            if (parms[i + 1]) {
                if (isNameInFilename(filename, parms[i+1])) i += 2;
                else { looping = loopExit; out = noOut; }
                
            }
            else
            {
                printf("Missing Argument for -name\n");
                exit(EXIT_FAILURE);
            }
        }
        
        else if (strcmp(parms[i], user) == 0) {
            /* calculate the length of the given uid in characters + 2 for the terminating 0, note log is natural, log10 is log based 10 */
            int userLength = (floor(log10(st.st_uid)) + 2);
            /* allocate the char string for the uid */
            char *userUIDString = malloc(userLength * sizeof(char));
            /* write the number as string, to be comparable with parameter char */
            snprintf(userUIDString, userLength, "%u", st.st_uid);
            
            if (parms[i + 1]) {
                
                if (((strcmp(userUIDString, parms[i + 1])) == 0) || readUID(st.st_uid, parms[i + 1]))
                {
                    i += 2;
                    free(userUIDString);
                }
                else
                {
                    looping = loopExit;
                    out = noOut;
                    free(userUIDString);
                }
            }
            else {
                printf("Missing Argument for -name\n");
                exit(EXIT_FAILURE);
            }
        }
        
        else if (strcmp(parms[i], type)) {
            /*Check on type */
        }
        
        else if (strcmp(parms[i], ls)) {
            out = lsOut;
            i++;
            
        }
    } while (parms[i] && (looping == loopCont));
    
    if (out == regOut) {
        printf("File path: %s\n", filename);
    }
    else if (out == lsOut) {
        printf("File formatted ls out: %s\n", filename);
    }
    
    
    
    if (S_ISDIR(st.st_mode)) {
        //  printf("this is a directory!\n");
        /* call do_dir with this filempath to start a new recusrion to step in this directory */
        do_dir(filename, parms
               );
        
    }
    else
    {
        //  printf("this is a regular file!\n");
    }
    // printf("Uid: %d\n", st.st_uid);
    
    return 0;
    
}

int readUID(uid_t uid, const char *name) {
    struct passwd *pwd = getpwuid(uid);
    
    return !(strcmp(name, pwd->pw_name));
}

char *isNameInFilename (const char *filePath, const char *name)
{
    
    const char *fileName;
    //if (fileName = strrchr(filePath, '/')
    if(strstr(filePath, "/"))
        fileName=strrchr(filePath, '/')+1;
    else fileName=filePath;
    
    return strstr(fileName, name);
}

