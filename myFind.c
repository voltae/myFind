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
#include <grp.h>        // getgrgid() get group name
#include <time.h>       // for time conversion

/* ------------------------------------------------------- defines -----*/
#define PATHDIVIDER "/"
#define FUENFHUNDERT 500 // TODO: Replace the 500 with dynamic allocation

/* ------------------------------------------------------- functions -- */
void printDir(void);

int readUID(uid_t uid, const char *name);

/* prototypes for the ls functionality */
char *getNameFromUID(uid_t uid);

char *getNameFromGID(gid_t gid);

char *combineFilePermissions (mode_t fileMode);

char *isNameInFilename (const char *filePath, const char *name);

void extendedFileOutputFromStat (const struct stat *fileStat, const char *filePath);

int do_dir(const char *dirName, const char **parms);

int do_file(const char *filename, const char **parms);

/* ------------------------------------------------------- enums -------*/
typedef enum {
    regOut, noOut, lsOut
} output;
typedef enum {
    loopCont, loopExit
} do_loop;

typedef enum nameFound {FOUND, NOTFOUND} nameFound;

/* ------------------------------------------------------- const char --*/
const char *print = "-print";
const char *user = "-user";
const char *uid = "-uid";
const char *type = "-type";
const char *name = "-name";
const char *ls = "-ls";
const char *nouser = "-nouser";


/** @brief implementation of a simplified find programm
 *
 * @param argc Number of arguments
 * @param argv actions used
 * @return 0 if all is correct, in failure chase report the error.
 */
int main(int argc, const char **argv) {
    
    /* protocols the error */
    int error;
    
    /* if no arguments are provided, print out a use message, and exit with failure */
    if (argc < 2) {
        printf("myFind: Usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]\n");
        exit(EXIT_FAILURE);
    }
    
    /* validate the actions of myFind step by step in a loop */
    for (int i = 0; i < argc; ++i) {
        /* wheed out every -commmand that is not allowd */
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], print) && strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) &&
                strcmp(argv[i], name) && strcmp(argv[i], nouser) && (strcmp(argv[i], ls))) {
                printf("myFind: Usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls] [-nouser]\n");
                exit(EXIT_FAILURE);
            }
        }
        /* if a valid action is found -name, -type but the next argument is missing, print out an error message. */
        if (!(strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) && strcmp(argv[i], name))) {
            if (argc == i + 1) {
                printf("myFind: Missing argument for %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed by the parameter of type -name is only a single one */
        if (!(strcmp(argv[i], name)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "myFind: paths must precede expression: %s\n", argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed the parameter -user is only a single one */
        else if (!(strcmp(argv[i], user)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "myFind: user must precede expression: %s\n", argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed the parameter -user is only a single one */
        else if (!(strcmp(argv[i], type)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "myFind: %s: unknown primary or operator\n", argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* -print, -ls, and -nouser must be single parameter, not followed by an argument */
        else if ((!(strcmp(argv[i], print)) && argv[i+1]) || (!(strcmp(argv[i], ls)) && argv[i+1]) || (!(strcmp(argv[i], nouser)) && argv[i+1]))
        {
            {
                fprintf(stderr, "myFind: %s: unknown primary or operator\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
        }
        
        /* -type is followed ony by [bcdpfls] */
         else if (!(strcmp(argv[i], type)))
         {
             nameFound allowedCharFound = NOTFOUND;
             int j = 0;
             char *allowedChar = NULL;
             char allowedChars[7] = "bcdpfls";
             do
             {
                 allowedChar = strchr(argv[i+1], allowedChars[j++]);
                 if (!allowedChar) allowedCharFound = NOTFOUND;
                 else allowedCharFound = FOUND;
            } while (allowedCharFound == NOTFOUND && j < 7);
             
             if (allowedCharFound == NOTFOUND)
             {
                 printf("myFind: -type: %s: unknown type\n", argv[i+1]);
                 exit(EXIT_FAILURE);
             }
        }
        
                  
    }
    
    /* call the openDirectory function for stepping though the directory */
    error = do_file(argv[1], &argv[2]);
    /* log out if an error occured */
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
        fprintf(stderr, "myFind: An error occurred during open dir: %s \n%s\n", dirName, strerror(errno));
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
    
    /* variable for the final output */
    output out = regOut;
    do_loop looping = loopCont;
    
    /* iterator for while loop */
    int i = 0;
    
    /* read out the struct for stat , use lstat for not trapping into a link circle */
    if (lstat(filename, &st) == -1) {
        fprintf(stderr, "Error in stat: %s\n", strerror(errno));
        
        /* in case of failure return to the do_dir function and read in the next entry */
        return -1;
    }
    
    do
    {
        /* Valentin did this */
        /* if the first action is null, not given, leave the loop
         * and print the entry out as same as -print. This is not necessary, the loop will
         *stop anyway because parms[0] == NULL, but more readable */
        /* We don't need this case at all, just delete it and make the next as if */
        if (!parms[0]) looping = loopExit;
        
        /* if the action -print is given, step one element in the action further,
         * it might followed by -ls */
        else if (!strcmp(parms[i], print))
        {
            printf("%s\n", filename);
            i++;
            out = noOut;
        }
        
        else if (!strcmp(parms[i], ls))
        {
            extendedFileOutputFromStat(&st, filename);
            i++;
            out = noOut;
        }
        
        /* if -name is found, step through following cases: */
        else if (!strcmp(parms[i], name))
        {
            /* if is followed by a parameter, take this as valid name */
            if (parms[i + 1])
            {
                /* if found a name - match in path */
                if (isNameInFilename(filename, parms[i+1])) i += 2;
                /* if not, leave the loop, with no output */
                else { looping = loopExit; out = noOut; }
            }
            /* if no parameter was found, print the error, no argument for name found */
            else
            {
                printf("myFind: Missing Argument for \"-name\"\n");
                exit(EXIT_FAILURE);
            }
        }
        
        /* if the action -user is found, step through following cases: */
        else if (!strcmp(parms[i], user))
        {
            char *userUIDString;
            /* calculate the length of the given uid in characters + 2 for the terminating 0, note log is natural, log10 is log based 10 */
            /* this works only if the entry in st.uid is > 0 */
            if (st.st_uid > 0)
            {
                int userLength = (floor(log10(st.st_uid)) + 2);
                /* allocate the char string for the uid */
                userUIDString = malloc(userLength * sizeof(char));
                /* write the number as string, to be comparable with parameter char */
                snprintf(userUIDString, userLength, "%u", st.st_uid);
            }
            /* initialize userUIDString with '0' */
            else
            {
                userUIDString = malloc(2 * sizeof(char));
                userUIDString[0] = '0';
                userUIDString[1] = 0;
            }
            if (parms[i + 1])
            {
                /* Compare the user string with the entry from system known user. Caution, the entry can differ from a known user */
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
            else
            {
                printf("myFind: Missing Argument for \"-name\"\n");
                exit(EXIT_FAILURE);
            }
        }
        
        else if (!strcmp(parms[i], type))
        {
            /*Check on type
             *  b   ... block
             *  c   ... character unbuffered special
             *  d   ... directory
             *  p   ... named pipe  (FIFO)
             *  f   ... regular file
             *  l   ... symolic link
             *  s   ... socket
             */
            
            /* increment the counter */
            i++;
        }
        
        else if (!strcmp(parms[i], ls))
        {
            out = lsOut;
            i++;
        }
        
        /* if no valid argument is found, increment the counter */
        else
        {
            i++;
        }
        
    } while (parms[i] && (looping == loopCont));
    
    /* Output the filepath in -print format, only the path */
    if (out == regOut) {
        printf("%s\n", filename);
    }
    /* Output the path in extended format with all parameters -ls */
    else if (out == lsOut) {
        printf("%s\n", filename);
    }
    
    if (S_ISDIR(st.st_mode)) {
        /* call do_dir with this filempath to start a new recusrion to step in this directory */
        do_dir(filename, parms);
    }
    
    return 0;
    
}
/* read the user entry from system library. Check if the entry exists anyway */
int readUID(uid_t uid, const char *name) {
    struct passwd *pwd = getpwuid(uid);
    
    /* User is not in the Struct list. Leave the program, no further search is needed*/
    if (pwd == NULL)
    {
        fprintf(stderr, "myFind: \"%s\" is not the name of a known user.\n", name);
        exit(EXIT_FAILURE);
    }
    /* User is in the struct list */
    /* test if the name and the user matches */
    else if (!(strcmp(name, pwd->pw_name)))
        return 1;
    else
        return 0;
}

char *getNameFromUID(uid_t uid)
{
    struct passwd *pwd = getpwuid(uid);
    /* Check if the number doe correspont to a valid entry  in the system table known users and groups */
    if (pwd == NULL)
    {
        /* calculate the length of the given uid in characters + 2 for the terminating 0, note log is natural, log10 is log based 10 */
        int userLength = (floor(log10(uid)) + 2);
        char *uidAsNumber = malloc(userLength * sizeof(char));
        snprintf(uidAsNumber, 9, "%u", uid);
        return uidAsNumber;
    }
    /* group is found, check if the entry matches */
    else
    {
        /* set the enum user known */
        return pwd->pw_name;
    }
}
/** function returns a groupname with a systemcall
 * if the group does not exist, return groupID as string */
char *getNameFromGID(gid_t gid)
{
    struct group *pwd = getgrgid(gid);
    /* Check if the number does correspont with a valid group number in the system */
    if (pwd == NULL)
    {
        /* calculate the length of the given gid in characters + 2 for the terminating 0, note log is natural, log10 is log based 10 */
        int groupLength = (floor(log10(gid)) + 2);
        char *gidAsNumber = malloc(groupLength * sizeof(char));
        snprintf(gidAsNumber, groupLength, "%u", gid);
        return gidAsNumber;
    }
    /* group exists, return the name */
    else return pwd->gr_name;
}
char *isNameInFilename (const char *filePath, const char *name)
{
    
    const char *fileName;
    
    /* if the path does not contain any '/' the filepath should be the filename,
     * in order to find the name in the root path as well */
    if(strstr(filePath, "/"))
        fileName=strrchr(filePath, '/')+1;
    else fileName=filePath;
    
    return strstr(fileName, name);
}

void extendedFileOutputFromStat (const struct stat *fileStat, const char *filePath)
{
    // get the inode number
    long int inodeNr = fileStat->st_ino;
    // get the size of the pointer filestat
    int sizePointer = sizeof(fileStat);
    // number of hardlinks in inode
    int linkAmount = fileStat->st_nlink;
    /* user Id  owner. Caution, the entry could be an number with is not a system known user */
    int userID = fileStat->st_uid;
    char *userName = getNameFromUID(userID);
    
    int groupID = fileStat->st_gid;
    /* group ID owner. Caution, the entry could be an number with is not a system known user */
    char *groupName = getNameFromGID(groupID);
    // file length
    long int fileLength = fileStat->st_size;
    // time of last modification
    time_t timeLastMod = fileStat->st_mtime;
    // TODO: format time
    char *timeFormatted = malloc(FUENFHUNDERT * sizeof(char));
    
    struct tm *localeTime = localtime(&timeLastMod);
    strftime(timeFormatted, FUENFHUNDERT, "%B %d %H:%M", localeTime);
    
    // Permissions
    mode_t permissions = fileStat->st_mode;
    char *permissionsChar = combineFilePermissions(permissions);
    
    printf("%lu\t %d \t %s\t %3d\t %s\t %s\t %6lu\t %s %s\n", inodeNr, sizePointer, permissionsChar, linkAmount, userName, groupName, fileLength, timeFormatted, filePath);
    free(timeFormatted);
    free(permissionsChar);
    
}
char *combineFilePermissions (mode_t fileMode)
{
    // output variable
    char *permissions = malloc(10 * sizeof(char));
    // counter
    int i = 0;
    // Test if file is directory, or Link
    if (fileMode & S_IFDIR) permissions[i++] = 'd';
    else if (fileMode & S_IFLNK) permissions[i++] = 'l';
    else permissions[i++] = '-';
    permissions[i++] = (fileMode & S_IRUSR) ? 'r' : '-';
    permissions[i++] = (fileMode & S_IWUSR) ? 'w' : '-';
    if ((fileMode & S_IXUSR) && (fileMode & S_ISUID)) permissions[i++] = 's';
    else if (fileMode & S_ISUID) permissions[i++] = 'S';
    else if (fileMode & S_IXUSR) permissions[i++] = 'x';
    else permissions[i++] = '-';
    
    permissions[i++] = (fileMode & S_IRGRP) ? 'r' : '-';
    permissions[i++] = (fileMode & S_IWGRP) ? 'w' : '-';
    if ((fileMode & S_IXGRP) && (fileMode & S_ISGID)) permissions[i++] = 's';
    else if (fileMode & S_ISGID) permissions[i++] = 'S';
    else if (fileMode & S_IXGRP) permissions[i++] = 'x';
    else permissions[i++] = '-';
    
    permissions[i++] = (fileMode & S_IROTH) ? 'r' : '-';
    permissions[i++] = (fileMode & S_IWOTH) ? 'w' : '-';
    if ((fileMode & S_IXOTH) && (fileMode & S_ISVTX)) permissions[i++] = 't';
    else if (fileMode & S_ISVTX) permissions[i++] = 'T';
    else if (fileMode & S_IXOTH) permissions[i++] = 'x';
    
    permissions[i] = 0;
    return permissions;
}
