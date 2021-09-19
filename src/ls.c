#include "libs.h"
#include "ls.h"

#define BIT_A (1<<0)
#define BIT_L (1<<1)
#define INCLUDE_HIDDEN(X) (X & BIT_A)
#define LIST_FORMAT(X) (X & BIT_L)
#define IGNORE(X, FLAG) (!INCLUDE_HIDDEN(FLAG) && X[0]=='.')

// -------------------------------- Util functions --------------------------------

string get_perms(struct stat *sb){
	
	string res = check_bad_alloc(malloc(11*sizeof(char)));

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

	for(int i=0; i<9; i++)
		res[i+1] = (sb->st_mode & masks[i]) ? p[i%3] : '-';
	
	res[10] = '\0';
	return res;
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

		string filename = v->arr[i];
		string path = check_bad_alloc(calloc(1, dirlen + strlen(filename) + 2));
		strcpy(path, dirname);
		if(dirname[dirlen-1] != '/') 
			strcat(path, "/");
		strcat(path, filename);

		struct stat sb;
		if(check_perror("ls", lstat(path, &sb), -1)) {
			free(path);	
			return;
		}
		free(path);

		string perms = get_perms(&sb);
		if(!perms) return;

		printf("%s %3ld %8s %8s %10ld ", perms, sb.st_nlink, getpwuid(sb.st_uid)->pw_name, \
				getgrgid(sb.st_gid)->gr_name, sb.st_size);

		char date[81];
		time_t curtime = time(0);
		if(llabs(curtime-sb.st_mtime) >= 15811200)
			strftime(date, 80, "%b %d  %Y", localtime(&(sb.st_mtime)));
		else
			strftime(date, 80, "%b %d %H:%M", localtime(&(sb.st_mtime)));

		printf("%s ", date);

        printf(" %s\n", filename);

		free(perms);
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
	string_vector directories;
	create_vector(&directories, 2);
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