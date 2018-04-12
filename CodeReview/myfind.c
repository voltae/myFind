// ### FB G16: Feedback Gruppe 16

/**
* @file myfind.c
* Betriebssysteme Myfind
* Einfache Implementierung des Linux find Befehls
* Beispiel 1
*
* @author Deniz Kücükkus <ic17b057@technikum-wien.at>
* @author Florian Hafner <ic17b056@technikum-wien.at>
* @author Florian Resch <ic17b035@technikum-wien.at>
* @date 2018/03/31
*
* @version 1.0
*
*
*
*/



/*
* ###############################
* ##### all needed inlcudes #####
* ###############################
*/
#include <stdio.h> // printf
#include <string.h> // strcmp
#include <dirent.h> // readdir, opendir...
#include <sys/types.h> // types
#include <sys/stat.h> // lstat, stat
#include <errno.h> // errno
#include <stdlib.h> // exit, strtol, getenv
#include <pwd.h> // getpwnam, getpwuid
#include <grp.h> // getgrgid
#include <libgen.h> //basename
#include <fnmatch.h> // fnmatch
#include <unistd.h> // readlink
#include <time.h> // localtime
#include <limits.h> // LONG_MIN, LONG_MAX
#include <string.h> // strlen


/*
* ###################
* ##### defines #####
* ###################
*/
#define PARM_PRINT "-print"
#define PARM_LS "-ls"
#define PARM_USER "-user"
#define PARM_NAME "-name"
#define PARM_TYPE "-type"
#define PARM_NOUSER "-nouser"
#define PARM_PATH "-path"

#define DATE_LENGTH 64
#define MODIFIER_LENGTH 10


static void do_file(const char *name, const char * const * parms);
static void do_dir(const char *dir_name, const char * const * parms);
static void compare_parms(const char * const * parms, const char *name);
static void print_match(const char *name, const char * const * parms, const struct stat *file);
static void print_ls(const struct stat *file, const char *name);
static int compare_user(const struct stat *file, const char *user);
static int compare_name(const char *file, const char *name);
static int compare_type(const struct stat *file, const char type);
static int compare_nouser(const struct stat *file);
static int compare_path(const char *file, const char *name);
static char get_type(const mode_t mode);
static void error_log(int line, char* text, const char* argument);


/**
*
* \brief entry point for the Programm myfind
*
* This is the main Function from that the Programs starts
*
* \param argc the number of arguments
* \param argv the arguments themselves
*
* \return EXIT_SUCCESS by success or EXIT_FAILURE by Failure
*/
int main(int argc, const char *argv[]) {

	if (argc > 1) {/*at least one Argument must be specified*/
// ### FB G16: What if no path is given? - argv[2] would be second parameter and argv[1] first action
		compare_parms(&(argv[2]), argv[0]);/*pre Argument compare to check consistents of the arguments*/
// ### FB G16: already assuming argv[1] is a path
		do_file(argv[1], &(argv[2]));/*Entry Point for the iterarion of the tree*/
	}
// ### FB G16: hier auch korrekte Fehlermeldung durch error_log?? valentin?----------------------------------------------------------------------
	else {/*no File and/or argument specified*/
		error_log(__LINE__, "usage: myfind <file or directory>\n -name <pattern>\n -user <name> | <uid>\n -type[bcdpfls]\n -path <path>\n -nouser\n -print\n -ls\n", argv[0]);
		exit(EXIT_FAILURE);	/*EXIT_FAILURE because Argument is needed*/
	}

	if (fflush(stdout) == EOF) {/*The C library function int fflush(FILE *stream) flushes the output buffer of a stream.*/
		error_log(__LINE__, "failed flush", argv[0]);
	}

	return EXIT_SUCCESS;
}

/**
*
* \brief calls print_match for Files and do_dir to iterate over a Directory
*
* \param name full path of the file
* \param parms filter and print parameters
*
* \ void Funktion no return
*/
// ### FB G16: name is actually the path of current file to check
static void do_file(const char *name, const char * const * parms) {

	struct stat stbuf;
	if (lstat(name, &stbuf) == -1) {/*reading information of the file (attributes)*/
// ### FB G16: no check for errno - which error case?
		error_log(__LINE__, "reading file", name);
		return;
	}
	print_match(name, parms, &stbuf);/*prints file if parameter matches*/
	if (S_ISDIR(stbuf.st_mode)) {
		do_dir(name, parms);/*case file is Directory start iteration of subdirectory*/
	}
}

/**
*
* \brief calls do_file for every file in the directory
*
* \param dir_name path of directory
* \param parms Parameters for Filter and compare
*
* \ void Funktion no return
*/
// ### FB G16: dir_name is actually the path of current file to check
static void do_dir(const char *dir_name, const char * const * parms) {
	DIR *dp;
	struct dirent *dirp;

	dp = opendir(dir_name);/*directory must be opened bevor reading entries of directory*/
	if (dp == NULL) {
		error_log(__LINE__, "opening dir", dir_name);
		return;
	}
	errno = 0;

	while ((dirp = readdir(dp)) != NULL) {/*as long as next entry is not null create Path and calls do_file*/
		if (dirp->d_name[0] != '.') {
// ### FB G16: in erstem If-Zweig ist new_path um 1 zu lang..anmerken? - valentin-------------------------------------------------------------------------
			char new_path[(strlen(dir_name) + strlen(dirp->d_name) + 2)];
			if (dir_name[strlen(dir_name) - 1] == '/') {
				sprintf(new_path, "%s%s", dir_name, dirp->d_name);
			}
			else {
				sprintf(new_path, "%s/%s", dir_name, dirp->d_name);
			}
			do_file(new_path, parms);
		}
// ### FB G16: was kann dieses errno hier? :/ sorry mein hirn lässt nach um die zeit---------------------------------------------------------------------
		errno = 0;
	}
	if (errno != 0) {
		error_log(__LINE__, "while do_dir", dir_name);
	}
// ### FB G16: closedir not necessary because of rekursion?----------------------------------------------------------------------------------------------
	if (closedir(dp) == -1) {
		error_log(__LINE__, "close dir", dir_name);
	}
}

/**
*
* \brief compare file to type if matching
*
* \param file stat struct of the File
* \param type type char (bfcdpls)
*
* \return 0 no match, 1 match
*
*/
static int compare_type(const struct stat *file, const char type) {
	char typeChar = get_type(file->st_mode);
	if (typeChar == type) {/*compare if type matching then return 1*/
		return 1;
	}
	return 0;
}

/**
*
* \brief helper funktion for different types
*
* \param mode mode of the file
*
* \return right type char or default '-'
*
*/
static char get_type(const mode_t mode)
{
	if (S_ISBLK(mode)) { /*if else construct to get the Format of the file*/
		return 'b';
	}
	else if (S_ISCHR(mode)) {
		return 'c';
	}
	else if (S_ISDIR(mode)) {
		return 'd';
	}
	else if (S_ISREG(mode)) {
		return '-'; /*because of original find - instead of f*/
	}
	else if (S_ISFIFO(mode)) {
		return 'p';
	}
	else if (S_ISLNK(mode)) {
		return 'l';
	}
	else if (S_ISSOCK(mode)) {
		return 's';
	}
	else {
		return '-';
	}
}

/**
*
* \brief compare file to specified name if matching
*
* \param file path of the file
* \param name name to match
*
* \return 0 no match, 1 match
*
*/
// ### FB G16: file is actually the path of current file to check----------------------------------------------------------------------(zeile 481)
static int compare_name(const char *file, const char *name)
{
	int ret = 0;
    int fnmatchRet;
	
// ### FB G16: why +1? valentin?--------------------------------------------------------------------------------------------------------
    char tempFileName[strlen(file)+1]; 
    char *baseName;

// ### FB G16: memcpy what for?--Valentin - kann man da nicht einfach sagen tempFileName = file?----------------------------------------
    memcpy(tempFileName, file, strlen(file)+1);/*else error validation const*/
    baseName = basename(tempFileName);

// ### FB G16: fnmatch gives back 0 if match - ignores *?--------------------------------------------------------------------------------
    fnmatchRet = fnmatch(name,baseName,FNM_NOESCAPE);/*check if basename matches pattern in name:*/
    if(fnmatchRet == 0) {
        ret=1;
    }
    else if (fnmatchRet != FNM_NOMATCH){
		error_log(__LINE__, "error matching name", file);
    }

    return ret;
}

/**
*
* \brief compare files user to a specified user if matching
*
* \param file stat struct of the file
* \param user username or uid
*
* \return 0 no match, 1 matching
*
*/
static int compare_user(const struct stat *file, const char* user)
{
	unsigned long uid;
	char *pEnd;
	struct passwd *pwd = NULL;

	pwd = getpwnam(user);
	if (errno != 0) { /*Error handling pwnam */
		error_log(__LINE__, "getting getpwnam", user);
		return 0;
	}

	if (pwd != NULL) { /*user name found*/
		if (file->st_uid == pwd->pw_uid) { return 1; }
	}
	else { /*user name not found user id*/
		uid = strtol(user, &pEnd, 10);
		if (*pEnd == '\0') {
			if (file->st_uid == uid) { return 1; }
		}
	}
	return 0;
}


// ### FB G16: Output should be (and IS) more than ls -l
/**
*
* \brief print type of Output ls -l
*
* \param file stat struct of the file
* \param file_name path of the file
*
* \ void Funktion no return
*/
//
static void	print_ls(const struct stat *file, const char *file_name)
{
	int userUid = 0;
	int grpUid = 0;
	struct passwd *pwdEntry = NULL;
	struct group *grpEntry = NULL;
	const time_t *modTime = &(file->st_mtime);

	long blockCount;
	char permissionBits[MODIFIER_LENGTH + 1] = "----------";
	char date[DATE_LENGTH + 1];

	errno = 0;
	pwdEntry = getpwuid(file->st_uid);
	if (pwdEntry == NULL) {
		error_log(__LINE__, strerror(errno), "");
		userUid = 1;
	}

	errno = 0;
	grpEntry = getgrgid(file->st_gid);
	if (grpEntry == NULL) {
		error_log(__LINE__, strerror(errno), "");
		grpUid = 1;
	}

	if (strftime(date, DATE_LENGTH, "%b %e %H:%M", localtime(modTime)) == 0) {
		error_log(__LINE__, "strftime for date", "");
		return;
	}

	permissionBits[0] = get_type(file->st_mode); /*get File type*/
	blockCount = 0L;
	if (permissionBits[0] != 'l') /* calculate blocksize */
	{
		blockCount = file->st_blocks;
		if (getenv("POSIXLY_CORRECT") == NULL)
		{
			blockCount = file->st_blocks / 2 + file->st_blocks % 2;
		}
	}

	/* set Permission Bits*/
	static const char *rwx[] = { "---", "--x", "-w-", "-wx",
		"r--", "r-x", "rw-", "rwx" };
// ### FB G16: the calculation of index is kind of hard to read
	strcpy(&permissionBits[1], rwx[(file->st_mode >> 6) & 7]);
	strcpy(&permissionBits[4], rwx[(file->st_mode >> 3) & 7]);
	strcpy(&permissionBits[7], rwx[(file->st_mode & 7)]);
	if (file->st_mode & S_ISUID)
		permissionBits[3] = (file->st_mode & S_IXUSR) ? 's' : 'S';
	if (file->st_mode & S_ISGID)
		permissionBits[6] = (file->st_mode & S_IXGRP) ? 's' : 'l';
	if (file->st_mode & S_ISVTX)
		permissionBits[9] = (file->st_mode & S_IXOTH) ? 't' : 'T';


	if (S_ISLNK(file->st_mode)) {
		char linkbuf[file->st_size + 1]; /* handle symbolic link */
		errno = 0;
		const int charsread = readlink(file_name, linkbuf, file->st_size);
		if (charsread == -1) {
			error_log(__LINE__, strerror(errno), "");
		}
		else { /* print file information of link*/
			linkbuf[file->st_size] = '\0';
// ### FB G16: why test userUID and grpUID?------------------------------------------------------------------------------------------------------
			if(userUid == 0 && grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%s\t%ld\t%s\t%s -> %s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, grpEntry->gr_name, file->st_size, date, file_name, linkbuf) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else if (userUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%u\t%ld\t%s\t%s -> %s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, file->st_gid, file->st_size, date, file_name, linkbuf) < 0) {
					fprintf(stderr, "error printing output");
				}		
			}
			else if (grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%s\t%ld\t%s\t%s -> %s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, grpEntry->gr_name, file->st_size, date, file_name, linkbuf) < 0) {
					fprintf(stderr, "error printing output");
				}				
			}
			else {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%u\t%ld\t%s\t%s -> %s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, file->st_gid, file->st_size, date, file_name, linkbuf) < 0) {
					fprintf(stderr, "error printing output");
				}							
			}	
		}
	}
// ### FB G16: Blockfile and Charfile do not have a blocksize?-----------------------------------------------------------------------------------
	else if (S_ISCHR(file->st_mode) || S_ISBLK(file->st_mode)) {
			if(userUid == 0 && grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%s\t%s\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, grpEntry->gr_name, " ", date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else if (userUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%u\t%s\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, file->st_gid, " ", date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else if (grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%s\t%s\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, grpEntry->gr_name, " ", date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%u\t%s\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, file->st_gid, " ", date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
	}
	else {  /* print file information of all other types*/
	
			if(userUid == 0 && grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%s\t%ld\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, grpEntry->gr_name, file->st_size, date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else if (userUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%s\t%u\t%ld\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, pwdEntry->pw_name, file->st_gid, file->st_size, date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else if (grpUid == 0) {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%s\t%ld\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, grpEntry->gr_name, file->st_size, date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}
			else {
				if (printf("%ld\t%ld\t%s\t%u\t%u\t%u\t%ld\t%s\t%s\n", file->st_ino, blockCount, permissionBits, file->st_nlink, file->st_uid, file->st_gid, file->st_size, date, file_name) < 0) {
					fprintf(stderr, "error printing output");
				}	
			}

	}
}

/**
*
* \brief same function as in compare_name redundant but to hold the structure of myfind
*
* \param file path of the file
* \param name Parameters for Filter and compare

* \return 0 no match 1 matching
*
*/
// ### FB G16: *name is now path searched for because of parm, in print_match *name is path, as here *file -> confusing as same var name is used for different arguments
static int compare_path(const char *file, const char *name) {

	int ret = 0; /*return value*/
    int fnmatchRet;

    fnmatchRet = fnmatch(name,file,FNM_NOESCAPE);
    if(fnmatchRet == 0) {
        ret=1;
    }
    /*if fnmatchRet not 0 and not FNM_NOMATCH -> error*/
    else if (fnmatchRet != FNM_NOMATCH){
		error_log(__LINE__, "error matching name", file);
    }

    return ret;
}

/**
*
* \brief prints file if parameter matches, break up after one does not match
*
* \param name path of the file
* \param parms Parameters for Filter and compare
* \param file lstat information of the file
* \ void Funktion no return Value but return in case of no match
*
*/
// ### FB G16: name is actually the path of current file to check----------------------------------------------------------------------------(zeile 118)
// ### FB G16: doublepointer unnecessary because never changed
static void print_match(const char *name, const char * const * parms, const struct stat *file)
{
	int count = 0;
	int printed = 0;
	while (parms[count] != NULL) {
		if (strcmp(parms[count], PARM_PRINT) == 0) {
// ### FB G16: printf instead of fprintf
			if (printf("%s\n", name) < 0) { error_log(__LINE__, "print", name); } /*-print*/
			printed = 1;
		}
		else if (strcmp(parms[count], PARM_LS) == 0) {
			print_ls(file, name); /*-ls*/
			printed = 1;
		}
		else if (strcmp(parms[count], PARM_USER) == 0) {
			count++;
			if (compare_user(file, parms[count]) == 0) { return; } /*-user*/
		}
		else if (strcmp(parms[count], PARM_NAME) == 0) {
			count++;
			if (compare_name(name, parms[count]) == 0) { return; } /*-name*/
		}
		else if (strcmp(parms[count], PARM_TYPE) == 0) {
			count++;
// ### FB G16: *parms[0] should be *parms[count]
			if (compare_type(file, *parms[0]) == 0) { return; } /*-type*/
		}
		else if (strcmp(parms[count], PARM_NOUSER) == 0) {
			if (compare_nouser(file) == 0) { return; } /*-nouser*/
		}
		else if (strcmp(parms[count], PARM_PATH) == 0) {
			count++;
			if (compare_path(name, parms[count]) == 0) { return; } /*-path*/
		}
// ### FB G16: 'if' not necessary - while loop looks if == NULL, if error because of missing argument for action - already error in compare_parms - no call for print_match
		if (parms[count] != NULL) {
			count++;
		}
	}

	if (!printed) {
		if (printf("%s\n", name) < 0) {
			error_log(__LINE__, "print", name);
		}
	}
}

/**
*
* \brief compare_nouser checks if files has no current user.
*
* \param file stat struct of the file
* \return 0 no match 1 matching
*
*/
static int compare_nouser(const struct stat *file)
{
	struct passwd *pwd_entry = NULL;

	pwd_entry = getpwuid(file->st_uid); /*Chech if no user*/
	if ((pwd_entry == NULL) && (errno == 0)) {
		return 1;
	}
	else if (errno != 0) {
		error_log(__LINE__, strerror(errno), "");
	}

	return 0;
}

/**
*
* \brief pre check of parameters, exit if something wrong
*
* \param parms Parameters for Filter and compare
* \param name File name for User error
* \ void Funktion no return
*
*/
static void compare_parms(const char * const * parms, const char *name)
{
// ### FB G16: no check if path for searching is given, if not then first action is argv[1] --------------------------------------(line 90)

// ### FB G16: sufficient to forward pointer (doublepointer not necessary if worked with copy of pointer?)
	char **argP;
	argP = (char**)parms;

	while (*argP != NULL) {
		if (strcmp(*argP, PARM_PRINT) == 0) {
			/*no extra value needed*/
		}
		else if (strcmp(*argP, PARM_LS) == 0) {
			/*no extra value needed*/
		}
		else if (strcmp(*argP, PARM_NOUSER) == 0) {
			/*no extra value needed*/
		}
		else if (strcmp(*argP, PARM_USER) == 0) { /*Check if user or uid exists*/
			struct passwd *pwd = NULL;
			char *pEnd;
			long strtolRet;

			argP++;
			if (*argP == NULL) {
				error_log(__LINE__, "arguments inconsitent", "");
				exit(EXIT_FAILURE);
			}
			pwd = getpwnam(*argP);
			if (errno != 0) {
				error_log(__LINE__, "getting getpwnam", *argP);
			}
			if (pwd == NULL) {
				strtolRet = strtol(*argP, &pEnd, 10);
// ### FB G16: only if same as LONG_MAX or LONG_MIN?
				if (strtolRet == LONG_MAX || strtolRet == LONG_MIN) {
					error_log(__LINE__, "parse userid", *argP);
					exit(EXIT_FAILURE);
				}
				if (*pEnd != '\0') {
// ### FB G16: fprintf not tested
					fprintf(stderr, "%s: `%s' is not the name of a known user\n", name, *argP); /*Try to recreate error of find because of test.sh*/
					exit(EXIT_FAILURE);
				}
			}
		}
		else if (strcmp(*argP, PARM_NAME) == 0) { /*Check if there is a name given*/
			argP++;
			if (*argP == NULL) {
				error_log(__LINE__, "-name arguments inconsitent", "");
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(*argP, PARM_TYPE) == 0) { /*check if type is -bfcdpls*/
			argP++;
			if (*argP == NULL) {
				error_log(__LINE__, "-type arguments inconsitent", "");
				exit(EXIT_FAILURE);
			}

			if (strlen(*argP) > 1) {
				error_log(__LINE__, "-type arguemts inconsitent only one Letter allowed", "");
				exit(EXIT_FAILURE);
			}

			if ((*argP[0] != 'b') && (*argP[0] != 'f') && (*argP[0] != 'c') && (*argP[0] != 'd') &&
				(*argP[0] != 'p') && (*argP[0] != 'l') && (*argP[0] != 's')) {
				error_log(__LINE__, "-type arguemts inconsitent only -bfcdpls allowed", "");
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(*argP, PARM_PATH) == 0) { /*checks if there is a path given*/
			argP++;
			if (*argP == NULL) {
				error_log(__LINE__, "-path arguments inconsitent", "");
				exit(EXIT_FAILURE);
			}
		}
		else {
			error_log(__LINE__, "arguments inconsitent", "");
			exit(EXIT_FAILURE);
		}
// ### FB G16: 'if' unnecessary because of while loop, else if incremented in else if branch always checked and if *argP == NULL -> Error
		if (*argP != NULL) {
			argP++;
		}
	}
}


/**
*
* \brief global Error function
*
* \param line Line of code where the error happend for bug tracking in future
* \param text error Text of the specific error
* \param argument to specific a error a argument can be added
* \ void Funktion no return
*
*/
static void error_log(int line, char* text, const char* argument) {
	// ### FB G16: right error message? valentin du weißt wie die ausschauen müssten?-------------------------------------------------------
	// ### FB G16: fprintf not tested, line in output should only be used for testing
	fprintf(stderr, "%s: error: %s at Line: %d argument: %s\n", __FILE__, text, line, argument);
}