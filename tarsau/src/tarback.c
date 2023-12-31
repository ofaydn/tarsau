#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#define MAX_FILES 32
#define MAX_TOTAL_SIZE 200 * 1024 * 1024 // 200 MB

typedef struct {
    char *name;
    int perms;
    int size;
    char *content;
} FileInfo;

void create_archive(int argc, char *argv[], char *output_name);
void extract_archive(char *archive_name, char *directory_name);
void createFile(const char *filename,const char *directory, mode_t permissions);
void create_files(FileInfo *files, int num_files, const char *directory_name);
int isTextFile(const char *filename);
int getFilePermissions(const char *filename);
int isDirectory(const char *path);
off_t getFileSize(const char *filename);

int main(int argc, char *argv[]) {
    if (argc < 3) {				//Usage check 
        printf("Usage:\n");
        printf("To merge files: tarsau -b file1 file2 ... -o output.sau\n");
        printf("To extract: tarsau -a archive.sau output_directory\n");
        exit(1);
    }

    if (strcmp(argv[1], "-b") == 0) {
        // Operation to create archive
        int i;
        if (argc > MAX_FILES + 4) {
            printf("Number of input files can't be more than 32.\n");
            exit(1);
        }
        char *output_name = "a.sau"; // Default output name if not provided
        off_t totalSize = 0;
	for(i = 2;i<argc;i++){
		if(strcmp(argv[i], "-o") == 0){
			if(argv[i+1] == argv[argc-1]){
					strcat(output_name,".sau");				
				}
			else if(argc == 4){
			printf("The name of the archive file must be given after the –o parameter.\n");
			exit(1);
				}
			}
		else if(isTextFile(argv[i]) > 0){
			totalSize += getFileSize(argv[i]);
			if(totalSize>MAX_TOTAL_SIZE){
				printf("Size of total input files can't exceed 200MBytes.\n");
				exit(1);
				}
		}else{
		printf("%s is not a text file.\n",argv[i]);
		exit(1);
		}
	}
        create_archive(argc, argv, output_name);
        
    	} else if (strcmp(argv[1], "-a") == 0) {
    	//operation to extract archive
    	
		if(argc == 4){
		char *archive_name = argv[2];
		char *directory_name = ".";
		   if(isDirectory(argv[3])){
		   	directory_name = argv[3];
		   }else{
		   printf("%s is not a valid directory, extracting to "".""\n",argv[3]);
		   }
		extract_archive(archive_name,directory_name);
		}else if(argc == 3){
			char *archive_name = argv[2];
			char *directory_name = ".";	
			extract_archive(archive_name,directory_name);
		}else{printf("Too much argument, try again.\n");
		}
		
        // Call extract_archive function
    } else {
    	printf("Usage:\n");
        printf("To merge files: tarsau -b file1 file2 ... -o output.sau\n");
        printf("To extract: tarsau -a archive.sau output_directory\n");
        exit(1);
    }
    return 0;
}

int isDirectory(const char *path) {
    DIR *dir = opendir(path);
    if (dir != NULL) {
        closedir(dir);
        return 1; // It's a directory
    }
    return 0; // It's not a directory or couldn't open
}



int getFilePermissions(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // Return file permissions // next to stmode would be specific for 3 digit permission
    }
    return -1; // Error occurred while getting permissions
}

off_t getFileSize(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size; // Return file size in bytes
    }
    printf("Error occurred while getting %s's file size.\n",filename);
    return -1; // Error occurred while getting file size
}

int isTextFile(const char *filename) { //returns 1 if file is a text, returns 0 if its not
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Failed to open %s.\n",filename);
        exit(1);
    }

    int is_text = 1; // Assume it's a text file initially

    // Check if the file contains non-printable characters (non-ASCII)
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c < 0 || c > 127) {
            is_text = 0; // Not a text file
            break;
        }
    }

    fclose(file);

    return is_text;
}

void create_archive(int argc, char *argv[], char *outputName) {
    int i;
    int seeknum;
    
    FILE *file = fopen(outputName, "w");
    //int file = open(outputName,O_CREAT | O_TRUNC,0644 );
    if (file == NULL){
    	printf("Failed to create a file.\n");
    	exit(1);
    }
    char *content = "0000000000";
    seeknum = strlen(content);
    fseek(file,0,SEEK_SET);
    fwrite(content,sizeof(char),seeknum,file);
    fseek(file,seeknum,SEEK_SET);
    for(i = 2;i<argc;i++){
	if(strcmp(argv[i], "-o") == 0){
	    break;
	}
    	fprintf(file, ".|%s,%o,%ld|",argv[i],getFilePermissions(argv[i]),getFileSize(argv[i]));
    	fseek(file, 0, SEEK_END);
    }
    long fileSize = getFileSize(outputName);
    fseek(file,0,SEEK_SET);
    char sizeString[11];
    snprintf(sizeString, sizeof(sizeString),"%010ld",fileSize);
    fwrite(sizeString,sizeof(char),10,file);
    fseek(file,0,SEEK_END);
    for(i=2;i<argc;i++){
    	if(strcmp(argv[i], "-o") == 0){
	    break;
	}
	FILE *inputFile = fopen(argv[i],"r");
	fprintf(file,"\n");
	int c;
	while((c = fgetc(inputFile)) != EOF){
		if(c != '\n'){
		fprintf(file,"%c",c);
		}else{
		fprintf(file, " ");
		}
	}
	fclose(inputFile);
	fseek(file,0,SEEK_END);
	
    }
    fprintf(file,"\n");
    fclose(file);
}
void printFileInfo(const FileInfo *files, int num_files) {
    printf("Files Information:\n");
    for (int i = 0; i < num_files; ++i) {
        printf("File %d:\n", i + 1);
        printf("Name: %s\n", files[i].name);
        printf("Permissions: %d\n", files[i].perms);
        printf("Size: %d\n", files[i].size);
        printf("\n");
    }
}
void create_files(FileInfo *files, int num_files, const char *directory_name) {
    char fullpath[256]; // Adjust the size according to your requirements

    for (int i = 0; i < num_files; ++i) {
        // Construct the file path with the directory name
        snprintf(fullpath, sizeof(fullpath), "%s/%s", directory_name, files[i].name);
	int fd = open(fullpath, O_CREAT | O_WRONLY,files[i].perms);
	    if(fd == -1){
	    	printf("Failed to create a file for %s.\n",files[i].name);
	    	exit(1);
	    }
	    ssize_t bytes_written = write(fd, files[i].content,files[i].size);
	    if(bytes_written < 0){
	    printf("Unable to write %s file.\n",files[i].name);
	    }else if ((int)bytes_written<files[i].size){
	    printf("succyccc");}
	    
	    close(fd);

/*
        FILE *file = fopen(filepath, "w");
        if (file == NULL) {
            fprintf(stderr, "Failed to create file: %s\n", filepath);
            continue; // Move to the next file if unable to create the current one
        }
        // Write content to the file
        fwrite(files[i].content, sizeof(char), strlen(files[i].content), file);

        fclose(file);*/
    }
}
/*
void createFile(const char *filename,const char *directory, mode_t permissions){
    char fullPath[512];
    
    snprintf(filepath, sizeof(fullPath),"%s%s",directory_name,files[i].name);
    int fd = open(fullPath, O_CREAT | O_WRONLY,files[i].perms);
    if(fd == -1){
    	printf("Failed to create a file for %s.\n",filename);
    	exit(1);
    }
    ssize_t bytes_written = write(fd, files[i].content,files,files[i].size+1);
    if(bytes_written < 0){
    printf("Unable to write %s file.\n",files[i].name);
    }
    
    close(fd);
}*/
/*
void createFile(const char *filename,const char *directory, mode_t permissions){
    char fullPath[512];
    snprintf(fullPath, sizeof(fullPath),"%s%s",directory,filename);
    int fd = open(fullPath, O_CREAT | O_WRONLY,permissions);
    if(fd == -1){
    	printf("Failed to create a file for %s.\n",filename);
    	exit(1);
    }
    close(fd);
}*/

void extract_archive(char *archive_name, char *directory_name) {
    FILE *file = fopen(archive_name, "r");
    if (file == NULL) {
        printf("Failed to open archive %s.\n",archive_name);
        exit(1);
    }
    char buffer[11]; // Buffer to store the first 10 characters (+1 for null terminator)
    if (fscanf(file, "%10s", buffer) != 1) {
        printf("Failed to read archive  section size.\n");
        fclose(file);
        exit(1);
    }
    int archiveSize = atoi(buffer);
    int num_files = 0;
    FileInfo files[MAX_FILES];
    if (fseek(file, 10, SEEK_SET) != 0) {
        printf("Failed to seek to the 11th character.\n");
        fclose(file);
        exit(1);
    }
    //reading section
    fseek(file,10,SEEK_SET);
    char buff[archiveSize + 1 -10];
    size_t sectionLength = archiveSize -10;
    size_t bytes_read = fread(buff,sizeof(char),sectionLength,file);
    buff[bytes_read] = '\0'; //null terminate
    printf("1\n");
    //printf("%s\n",buff);
    if (buff != NULL && num_files < MAX_FILES) {
        char *token = strtok(buff, "!|");
        while(token !=NULL){
        	files[num_files].name = malloc(strlen(token) + 1);
		if (sscanf(token, "%[^,],%d,%d", files[num_files].name, &files[num_files].perms, &files[num_files].size) == 3) {
		        printf("2\n");
		        num_files++;        
		    }
        token = strtok(NULL, "!|");
        }  
    }
    fseek(file,archiveSize,SEEK_SET);
    int i;
    for (i = 0; i < num_files; ++i) {
        fscanf(file, "%d\n", &files[i].size);
        files[i].content = malloc(files[i].size + 1); // Allocate memory
        if (files[i].content != NULL) {
            if (fgets(files[i].content, files[i].size + 1, file) != NULL) {
                // Remove newline character if present
                printf("3\n");
                size_t len = strlen(files[i].content);
                if (len > 0 && files[i].content[len - 1] == '\n') {
                    files[i].content[len - 1] = '\0';
                }
            } else {
                fprintf(stderr, "Failed to read content for file %d.\n", i + 1);
                printf("4\n");
                free(files[i].content); // Free memory if content reading fails
                break;
            }
        } else {
            fprintf(stderr, "Memory allocation failed for file %d.\n", i + 1);
            break;
        }
    }
    printf("5\n");
    create_files(files,num_files,directory_name); 
    printf("6\n");  
    fclose(file);

}
