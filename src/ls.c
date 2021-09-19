#include "libs.h"
#include "ls.h"

#define BIT_A (1<<0)
#define BIT_L (1<<1)
#define INCLUDE_HIDDEN(X) (X & BIT_A)
#define LIST_FORMAT(X) (X & BIT_L)
#define IGNORE(X, FLAG) (!INCLUDE_HIDDEN(FLAG) && X[0]=='.')

// -------------------------------- Util functions --------------------------------

/**
 * @brief Returns a string containing the perms for a file displayed by ls -l
 * 
 * @param stat Pointer to stat struct populated by call to lstat
 * @return string containing the perms (1st column) of ls. Must be freed by caller.
 */
string get_perms(struct stat *sb){
	
	// Permission string is always 10 characters long
	string res = check_bad_alloc(malloc(11*sizeof(char)));
	// Mask out all non permission bits & set type appropriately
	switch(sb->st_mode & S_IFMT){
		case S_IFREG:	res[0] = '-'; break;
		case S_IFDIR:	res[0] = 'd'; break;
		case S_IFBLK:	res[0] = 'b'; break;
		case S_IFCHR:	res[0] = 'c'; break;
		case S_IFIFO:	res[0] = 'p'; break;
		case S_IFLNK:	res[0] = 'l'; break;
		case S_IFSOCK:	res[0] = 's'; break;
		default: 		res[0] = '?'; // Unknown file
	}

	string p = "rwx";
	mode_t masks[] = {
		S_IRUSR, S_IWUSR, S_IXUSR,
		S_IRGRP, S_IWGRP, S_IXGRP,
		S_IROTH, S_IWOTH, S_IXOTH
	};

	// Mark read write perms for all groups
	for(int i=0; i<9; i++)
		res[i+1] = (sb->st_mode & masks[i]) ? p[i%3] : '-';
	// Null terminate string
	res[10] = '\0';
	return res;
}

/**
 * @brief Print details of a file in ls -l format
 * 
 * @param filepath Path to file from cwd
 * @param filename Name to display on last column
 */
void __print_list_file(string filepath, string filename){

	// Read stat struct
	struct stat sb;
	if(check_perror("ls", lstat(filepath, &sb), -1)) return;

	// Obtain perms string
	string perms = get_perms(&sb);
	if(!perms) return;
	
	// Read st_mtime and format as required
	char date[81];
	time_t curtime = time(0);
	if(llabs(curtime-sb.st_mtime) >= 15811200)
		strftime(date, 80, "%b %d  %Y", localtime(&(sb.st_mtime)));
	else
		strftime(date, 80, "%b %d %H:%M", localtime(&(sb.st_mtime)));

	// Print output
	printf("%s ", perms);
	printf("%3ld ", sb.st_nlink);
	printf("%8s ", getpwuid(sb.st_uid)->pw_name);
	printf("%8s ", getgrgid(sb.st_gid)->gr_name);
	printf("%10ld ", sb.st_size);
	printf("%s ", date);
    printf("%s\n", filename);

    // Cleanup
	free(perms);
}

int64_t __get_total(string dirname, string_vector *v){
	
	int dirlen = strlen(dirname);
	int64_t total = 0;

	for(int i=0; i < v->size; i++){

		string filename = v->arr[i];
		string path = check_bad_alloc(calloc(1, dirlen + strlen(filename) + 2));
		// Generate filepath
		strcpy(path, dirname);
		if(dirname[dirlen-1] != '/') 
			strcat(path, "/");
		strcat(path, filename);

		// Sum over all file blocksizes
		struct stat sb;
		if(check_perror("ls", lstat(path, &sb), -1)) return -1;
		total += sb.st_blocks;
		free(path);
	}

	// Return total / 2
	return total>>1;
}

// -------------------------------- Util functions --------------------------------


/**
 * @brief ls -l print command
 * @details Can handle the -a flag. Handles list format
 * 
 * @param d DIR* Pointer to the directory stream of the directory to print
 * @param flags bitmask for the -a flag. Portable to more. 
 */
void __printdir_list(string dirname, string_vector *v){
	
	int dirlen = strlen(dirname);

	for(int i=0; i < v->size; i++){
		// Variable setup / allocation
		string filename = v->arr[i];
		string path = check_bad_alloc(calloc(1, dirlen + strlen(filename) + 2));

		// Generate filepath from directory path and filename
		strcpy(path, dirname);
		if(dirname[dirlen-1] != '/') 
			strcat(path, "/");
		strcat(path, filename);

		// Print file output in list format
		__print_list_file(path, filename);
		free(path);
	}

}

/**
 * @brief The default ls command
 * @details Can handle the -a flag. Does NOT handle list format.
 * 
 * @param d DIR* Pointer to the directory stream of the directory to print
 * @param flags bitmask for the -a flag. Portable to more. 
 */
void __printdir_dfl(string_vector *v){
	
	// Print them now :)
	for(int i=0; i < v->size; i++)
		if(printf("%s  ", v->arr[i]) < 0) 
			throw_error(PRINTF_FAIL);
}


int ls(Command *c){
	// Init command state vars
	uint8_t flags = 0;
	string_vector directories, files;
	create_vector(&directories, 2);
	create_vector(&files, 2);
	int dirs_received = 0;

	// Parse Arguments
	for(int i=1; i<=c->argc; i++){
		// If flag argument
		if(c->argv.arr[i][0] == '-'){
			// Iterate over all flags 
			for(char *ptr = c->argv.arr[i] + 1; *ptr; ptr++){
				// Set appropriate bit per flag / throw error
				switch(*ptr){
					case 'l':
						flags |= BIT_L;
					break;
					case 'a':
						flags |= BIT_A;
					break;
					default:
						throw_error(BAD_FLAGS);
						return -1;
				}
			}
		}
		// If not a flag argument, it must be a directory.
		else{
			dirs_received++;
			int W_SZ = 20; // => strlen("ls: cannot access ''")
			
			// TODO: Optimize this so I don't have to malloc for every directory
			// Can malloc once for buf and realloc for appending only when necessary
			// Constant prefix so just overwrite suffix from constant offset.
			string buf = malloc(strlen(c->argv.arr[i]) + W_SZ + 1);
			if(sprintf(buf, "ls: cannot access '%s'", c->argv.arr[i]) < 0){
				throw_error(PRINTF_FAIL); free(buf);
				return -1;
			}
			struct stat sb;
			if(check_perror(buf, lstat(c->argv.arr[i], &sb), -1)) continue;
			if(S_ISREG(sb.st_mode)){
				push_back(&files, c->argv.arr[i]);
				free(buf);
				continue;
			}

			// If given path is not a directory we can open, write error to terminal
			DIR *tmp;
			if(check_perror(buf, (long long) (tmp = opendir(c->argv.arr[i])), 0)){
				free(buf); continue;
			}
			free(buf);
			if(check_perror("ls", closedir(tmp), -1)) continue;
			// Valid directory, populate directory vector
			push_back(&directories, c->argv.arr[i]);
		}
	}

	// If no directories specified, ls on cwd
	if(dirs_received == 0) push_back(&directories, ".");
	vec_sort(&directories, CASE_INSENSITIVE_SORT);
	vec_sort(&files, CASE_INSENSITIVE_SORT);

	// Output all singleton files first
	for(int i=0; i<files.size; i++){
		if(LIST_FORMAT(flags))
			__print_list_file(files.arr[i], files.arr[i]);
		else
			printf("%s  ", files.arr[i]);
	}

	printf("\n");
	// Iterate over all directories
	for(int i=0; i<directories.size; i++){
		// If multiple directories, print directory name
		if(directories.size > 1){
			if(printf("%s:\n", directories.arr[i]) < 0){
				throw_error(PRINTF_FAIL);
				return -1;
			}
		}

		// Open directory
		DIR *d;
		if(check_perror("ls", (long long) (d = opendir(directories.arr[i])), 0)) continue;

		errno = 0; // Set errno to 0 so we can distinguish between end of stream and error
		struct dirent *dir;
		
		// Populate all files into vector
		string_vector list;
		create_vector(&list, 4);
		while((dir = readdir(d))){
			if(IGNORE(dir->d_name, flags)) continue;
			push_back(&list, dir->d_name);
		}
		if(errno) perror("ls");

		// Sort all files irrespective of case for pretty printing
		vec_sort(&list, CASE_INSENSITIVE_SORT);

		// Print total number of blocks required
		printf("total %ld\n", __get_total(directories.arr[i], &list));

		// Print directory contents
		if(!LIST_FORMAT(flags))
			__printdir_dfl(&list);
		else 
			__printdir_list(directories.arr[i], &list);

		// Cleanup & handle errors
		destroy_vector(&list);
		if(printf("\n") < 0) throw_error(PRINTF_FAIL);
		if(i!=directories.size-1) if(printf("\n") < 0) throw_error(PRINTF_FAIL);
		if(check_perror("ls", closedir(d), -1)) continue;
	}

	// Cleanup
	destroy_vector(&directories);
	return 0;
}