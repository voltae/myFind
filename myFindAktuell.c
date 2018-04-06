/**
 * @mainpage myFind
 *
 * Betriebssysteme Assigment 1 - myFind
 * <br>Beispiel 1
 * 
 * @author Valentin Platzgummer
 * @author Lara Kammerer
 */

/**
 * @file myFind.c
 *
 * @brief -----------kurze Beschreibung der Datei, was enthält sie, wozu ist sie da, ...
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
#define FUENFHUNDERT 500 // TODO: Replace the 500 with dynamic allocation
#define PROGRAM_NAME "myFind:"
#define UID_MAX 65535
/* ------------------------------------------------------- enums -------*/

/**
 * @enum output
 *
 * @brief enum holding variable if Output is needed
*/
typedef enum {
    regOut, noOut//----------------------------------------------
} output;

/**
* @enum do_loop
*
* @brief enum holding variable if Loop is still needed
*/
typedef enum {
    loopCont, loopExit
} do_loop;

/**
* @enum mybool
*
* @brief enum holding variable TRUE/FALSE
*/
typedef enum mybool {
    FALSE, TRUE
} mybool;

/**
* @enum nameFound
*
* @brief enum holding variable FOUND/NOTFOUND
*/
typedef enum nameFound {
	FOUND, NOTFOUND
} nameFound;

/* ------------------------------------------------------- functions -- */
void printDir(void);

mybool getUIDFromName(const char *name, unsigned int *uidNumber);

/* prototypes for the ls functionality */
char *getNameFromUID(uid_t uid);

char *getNameFromGID(gid_t gid);

char *combineFilePermissions (mode_t fileMode);

mybool isFilenameSameAsName (const char *filePath, const char *name);

/* check the file type */
mybool isFileType (const char argument, struct stat st);

void extendedFileOutputFromStat (const struct stat *fileStat, const char *filePath);

int do_dir(const char *dirName, const char **parms);

int do_file(const char *filename, const char **parms);

mybool isStringOnlyNumbers (const char *testString);

/* ------------------------------------------------------- const char --*/
const char *print = "-print";
const char *user = "-user";
const char *uid = "-uid";
const char *type = "-type";
const char *name = "-name";
const char *ls = "-ls";
const char *nouser = "-nouser";


/**
 * @brief implementation of a simplified find programm
 *
 the main function iterates throu al actions one my one, checks if the given actions fulfil the requested syntax.
 * names searches for matches in filepath aond/or filename.
 * user searches files owned by the given value. This value can be either a username or an userid.
 * type searches for specifed filetypes, such as directories, block files
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
        printf("%s Usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls]\n", PROGRAM_NAME);
        exit(EXIT_FAILURE);
    }
    
    /* validate the actions of myFind step by step in a loop */
    for (int i = 0; i < argc; ++i) {
        /* wheed out every -commmand that is not allowed */
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], print) && strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) &&
                strcmp(argv[i], name) && strcmp(argv[i], nouser) && (strcmp(argv[i], ls)))
            {
                printf("%s Usage: <file or directory> [-user <name>] [-name <pattern>] [-type [bcdpfls] [-print] [-ls] [-nouser]\n", PROGRAM_NAME);
                exit(EXIT_FAILURE);
            }
        }
        /* if a valid action is found -name, -type but the next argument is missing, print out an error message. */
        if (!(strcmp(argv[i], user) && strcmp(argv[i], uid) && strcmp(argv[i], type) && strcmp(argv[i], name))) {
            if (argc == i + 1) 
			{
                printf("%s Missing argument for %s\n",PROGRAM_NAME, argv[i]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed by the parameter of type -name is only a single one */
        if (!(strcmp(argv[i], name)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "%s paths must precede expression: %s\n", PROGRAM_NAME, argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed the parameter -user is only a single one */
        else if (!(strcmp(argv[i], user)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "%s user must precede expression: %s\n", PROGRAM_NAME, argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* Test if the argument followed the parameter -user is only a single one */
        else if (!(strcmp(argv[i], type)) && argv[i+2])
        {
            if (argv[i+2][0] != '-')
            {
                fprintf(stderr, "%s %s: unknown primary or operator\n", PROGRAM_NAME, argv[i+2]);
                exit(EXIT_FAILURE);
            }
        }
        /* -print, -ls, and -nouser must be single parameter, not followed by an argument */
        else if ((!(strcmp(argv[i], print)) && argv[i+1]) || (!(strcmp(argv[i], ls)) && argv[i+1]) || (!(strcmp(argv[i], nouser)) && argv[i+1]))
        {
            {
                fprintf(stderr, "%s %s: unknown primary or operator\n", PROGRAM_NAME, argv[i+1]);
                exit(EXIT_FAILURE);
            }
        }
        
        /* -type is followed only by [bcdpfls] */
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
                printf("%s -type: %s: unknown type\n",PROGRAM_NAME, argv[i+1]);
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
 * @brief checks for matching struct and builds absolute path
 * 
 * checks file for matching struct
 * <br>if no struct is found - error
 * <br>if matching struct found
 * <u>
 * <li>check for directory</li>
 * <li>checks the directory stream for NULL in this case the recursion will be stopped</li>
 * <li>build absolute path and check for arguments with <i>do_file</i></li>
 *</ul>
 *
 * @param dirName char Pointer holding relative path of the found directory.
 * @param param Pointer of charpointer holding the actions.
 * @return int 0 in case in regular case. -1 in error case.
 */
int do_dir(const char *dirName, const char **param) {
    
    /* Basepath holds the full file path till now. */
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
        fprintf(stderr, "%s %s: %s\n", PROGRAM_NAME, dirName, strerror(errno));
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
        
        if (!(filePath = malloc(lengthOfPath * sizeof(char))))
		{
            fprintf(stderr, "%s Memory error: %s\n", PROGRAM_NAME, strerror(errno));
        }
        
        /*  int snprintf(char *str, size_t size, const char *format, ...); */
        /* at the basepath get attached the given filepath to create the new filepath */
        error = (snprintf(filePath, lengthOfPath, "%s/%s", basePath, filename));
        
        if (error < 0)
		{
            fprintf(stderr, "%s An error occurred, %s\n", PROGRAM_NAME, strerror(errno));
        }
        
        /* call the do_file function with the constructed file path */
        do_file(filePath, param);
        
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

/**
*@brief checks file for arguments and prints if accurate
*
* lstat checks if file is a link
* <br>checks file for actions and their parameter
* <br>if action is -ls or -print immediately prints out data
* <br>if not - check for given argument
* <br> print if parameter are accurate
*
* @param filename charpointer holding the absolute filepath
* @param parms chardoublepointer holding the actions
*/
/* A second function for reading the directory and print out the entries in the fashion needed by entries given the application */
int do_file(const char *filename, const char **parms) {
    /* The function acts as te second part of the recursion, the first do_dir feeds in each found file, and this function examiates each file further.
     If a found file is a directory, then the fiel will be feeded in the first function to start a deeper level of recursiion in the file hierarchy. */
    /* st is a struct of type stat for each file, in the stat struct is the information about the file */
    struct stat st;
    
    /* variable for the final output */
    output out = regOut;
    do_loop looping = loopCont;
    
    /* iterator for while loop */
    int i = 0;
    
    /* read out the struct for stat , use lstat for not trapping into a link circle */
    if (lstat(filename, &st) == -1)
	{
        fprintf(stderr, "%s %s: %s\n", PROGRAM_NAME, filename, strerror(errno));
        /* in case of failure return to the do_dir function and read in the next entry */
        return -1;
    }
    
    do
    {
        /* if the first action is null, not given, leave the loop */
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
				if (isFilenameSameAsName(filename, parms[i + 1]))
				{
					i += 2;
				}
                /* if not, leave the loop, with no output */
                else
				{
					looping = loopExit;
					out = noOut;
				}
            }
            /* if no parameter was found, print the error, no argument for name found */
            else
            {
                printf("%s Missing Argument for \"-name\"\n", PROGRAM_NAME);
                exit(EXIT_FAILURE);
            }
        }
        
        /* if the action -user is found, step through following cases: */
        else if (!strcmp(parms[i], user))
        {
            if (parms[i + 1])
            {
                /* Compare the user string with the entry from system known user. Caution, the entry can differ from a known user */
                /* Better approach get user uid from passwd with the user name as char and compare with s.st_uid (int) */
                
                /* user could be a uid number as well, so test the string if contains only numbers.
                 * in this case, feed in the int to the function getUIDFromNumber. */
                /* True if the file belongs to the user uname.  If uname is numeric
                 and there is no such user name, then uname    is treated as a    user
                 ID. */
               
                /* 1. Check if parameter following -name is only numeric
                 * 2. if this is the case: threat the number as char and search for valid user
                 * if user id is ot found, take the number as valid ui id and test the file against it
                 * 3. if parameter is not numeric, test te name for valid user.
                    if not found, report error and exit program. */
                
                mybool isUserOnlyNumeric = isStringOnlyNumbers(parms[i+1]);
                unsigned int uidUser = 0;
                mybool isUidCorrect;
                /* parmater is only numeric */
                if (isUserOnlyNumeric == TRUE)
                {
                    /* treat parameter as char, the uidUser as to be a pointer of int, because it should be an unsigned int, the error detection works now with the */
                    isUidCorrect = getUIDFromName(parms[i+1], &uidUser);
                    if (isUidCorrect == TRUE)
                    {
                        /* and test the given uid with the file owner*/
                        if (st.st_uid == uidUser)
                        {
                            i += 2;
                        }
						else
						{
							out = noOut;
							looping = loopExit;
						}
                    }
                    /* no, the number as char was not found. treat the parameter as uid number, but it has to be converted into an int.  */
                   else if (isUidCorrect == FALSE)
                   {
                     unsigned int parmAsNumber = strtol(parms[i+1], NULL, 10);
                       /* if the user id is bigger than 16 Bit, report error atoi with return -1 in this case. (This is behavior from osx, Linux gives the full number */
                       if (parmAsNumber > UID_MAX)
                       {
                           printf("%s %s: Numerical result out of range\n", PROGRAM_NAME, parms[i+1]);
                           exit(EXIT_FAILURE);
                       }
                       /* found step to the next paramter, and mark found file to as output. */
                       else  if (st.st_uid == parmAsNumber)
                       {
                           i +=2;
                       }
                       else
                       {
						   looping = loopExit;
                           out = noOut;
                       }
                   }
                }
                /* parameter is not numeric only, it will be considered as user name only */
                else if (isUserOnlyNumeric == FALSE)
                {
                    isUidCorrect = getUIDFromName(parms[i+1], &uidUser);
                    /* report error if the given namenot matches with a system known user */
                    if (isUidCorrect == FALSE)
                    {
                        fprintf(stderr, "myFind: \"%s\" is not the name of a known user.\n", parms[i+1]);
                        exit(EXIT_FAILURE);
                    }
                    /* user is found, so test the found user if it is the owner of the file */
                    else
                    {
                        /* found an entry, step to the next parameter and mark file as output. */
                        if (st.st_uid == uidUser)
                        {
							i += 2;
                        }
                        else
                        {
                           looping = loopExit;
                           out = noOut;
                        }
                    }
                }
            }
            else
            {
                printf("%s Missing Argument for \"-user\"\n", PROGRAM_NAME);
                exit(EXIT_FAILURE);
            }
        }
        /* is next parameter -type?
         * if yes, check the its argument */
        else if (!strcmp(parms[i], type))
        {
            /** returns true if the argument is a correct typ */
            mybool isValidType = isFileType(*parms[i+1], st);
            if (isValidType)
            {
                out = regOut;
                i++;
            }
            else
            {
                looping = loopExit;
                out = noOut;
            }
        }
        
        /* if no valid argument is found, increment the counter.
	* Because error checking for file types are already done n the main function on further errochecking ar needed here.*/
        else
        {
            i++;
        }
        
    } while (parms[i] && (looping == loopCont));

    /* Output the filepath in -print format, only the path */
	if (out == regOut) {
		printf("%s\n", filename);
	}

    if (S_ISDIR(st.st_mode)) {
        /* call do_dir with this filempath to start a new recurion to step in this directory */
        do_dir(filename, parms);
    }
    
    return 0;
}

/**
 * @brief read the user entry from system library. Check if the entry exists anyway
 *
 * takes a charpointer with the user name and an pointer to unsigned int as buffer for the uid number datatype uid_t (which is unsigned int)
 *
 * @param name charpointer holding filename
 * @param uidNumber pointer as buffer for the found uid number of type integer.
 * @returns  a bool true if the conversion succeded, false if not.*/
mybool getUIDFromName(const char *name, unsigned int *uidNumber) {
    struct passwd *pwd = getpwnam(name);
    
    if (pwd)
    {
        *uidNumber = pwd->pw_uid;
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Check if the number does correspond to a valid entry in the system table known users and groups
 *
 * checks if the UIDnumber corresponds to a valid entry and reads out user name
 *
 * @argument unsigned integer uid_t
 * @returns a pointer to char with name if a match in database was found
 */
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

/**
 * @brief checks if the GID number does correspond to a valid entry in the system table known users and groups
 *
 * checks if the GIDnumber corresponds to a valid entry and reads out user name
 *
 * @argument unsigned integer gid_t
 * @returns a groupname with a systemcall - if the group does not exist, return groupID as string
*/
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

/**-----------------------------------------------------
 * @brief checks if parameter of -name is icluded in filepath
 *
 * checks if filepath is only filename - if not filename is picked out
 * <br>checks if parameter of -name is icluded in filename
 *
 * @param filePath charpointer holding filepath of checked file
 * @param name charpointer holding parameter to be checked
 * @returns NULL if parameter is not included
 */
mybool isFilenameSameAsName (const char *filePath, const char *name)
{
    
    const char *fileName;
    
    /* if the path does not contain any '/' the filepath should be the filename,
     * in order to find the name in the root path as well */
    if(strstr(filePath, "/"))
        fileName=strrchr(filePath, '/')+1;
    else fileName=filePath;

	if (strcmp(fileName, name) == 0)
		return TRUE;
	return FALSE;
}

/**
 * @brief helper function - tests if a charpointer is numeric only
 *
 * <br>takes a char pointer with text
 * @returns TRUE if only numbers are found, FALSE in the case a letter is in the char
 */
mybool isStringOnlyNumbers (const char *testString)
{
    mybool isNumberOnly = FALSE;
    for (unsigned int i = 0; i < strlen(testString); i++)
    {
        if (testString[i] < 48 || testString[i] > 57)
            isNumberOnly = FALSE;
        else isNumberOnly = TRUE;
    }
    return isNumberOnly;
}

/**
  * @brief prints extended output if action is -ls
  * 
  * takes statpointer and reads out filedata for extended print
  *
  * @param fileStat statpointer holds struct containing filedata
  * @param filePath charpointer holds filepath
 */
void extendedFileOutputFromStat (const struct stat *fileStat, const char *filePath)
{
    // get the inode number
    long int inodeNr = fileStat->st_ino;
    // get the size of the pointer filestat
    blksize_t blockSize = fileStat->st_blocks;
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
    
    printf("%lu\t %ld\t\t %s\t %3d\t %s\t %s\t %6lu\t %s %s\n", inodeNr, blockSize, permissionsChar, linkAmount, userName, groupName, fileLength, timeFormatted, filePath);
    free(timeFormatted);
    free(permissionsChar);
}

/**
 * @brief combines file permissions to be printed
 *
 * fileMode is checked by layering with the system masks from sys/types.h
 * <br>if permissions are set print accurate letter - if not print '-'
 *
 * @param fileMode 12bit value containing set permissions
*/
char *combineFilePermissions (mode_t fileMode)
{
    // output variable
    char *permissions = malloc(10 * sizeof(char));
    // counter
    int i = 0;
    // Test if file is directory, or Link
    if (S_ISDIR(fileMode)) permissions[i++] = 'd';
    else if (S_ISREG(fileMode)) permissions[i++] = '-';
    else if (S_ISLNK(fileMode)) permissions[i++] = 'l';
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

/**
 * @brief checks for argument of -type
 *
 * checks if argument matches type of file
 *
 * @param argument char holding argument checked
 * @param st stat holding type of file
 * @returns TRUE if argument matches filetype
*/
mybool isFileType (const char argument, struct stat st)
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
    /* search for block files */
    mybool returnValue = FALSE;
    
    switch (argument)
    {
            /* found a block file */
        case 'b':
            if (st.st_mode & S_IFBLK) returnValue = TRUE;
            break;
            
            /* search for character unbuffered special files */
        case 'c':
            /* found a character unbuffered special file */
            if (st.st_mode & S_IFCHR) returnValue = TRUE;
            break;
            
            /* search for a directory */
        case 'd':
            if (st.st_mode & S_IFDIR) returnValue = TRUE;
            break;
            
            /* search for a FIFO (Pipe) */
        case 'p':
            if (st.st_mode & S_IFIFO) returnValue = TRUE;
            break;
            
            /* search for a regular file */
        case 'f':
            if (st.st_mode & S_IFREG) returnValue = TRUE;
            break;
            
            /* search for a symbolic link  */
        case 'l':
            if (st.st_mode & S_IFLNK) returnValue = TRUE;
            break;
            
            /* search for a socket */
        case 's':
            if (st.st_mode & S_IFSOCK) returnValue = TRUE;
            break;
            
            /* found nothing return false */
        default:
            returnValue = FALSE;
            break;
    }
    /* return the given value */
    return returnValue;
}
