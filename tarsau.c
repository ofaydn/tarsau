#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
//
//			off_t size = getFileSize(argv[i]);
//			mode_t perms = getFilePermissions(argv[i]);
#define MAX_FILES 32
#define MAX_TOTAL_SIZE 200 * 1024 * 1024 // 200 MB

void create_archive(int argc, char *argv[], char *output_name);

void extract_archive(char *archive_name, char *directory_name) {
    // Read organization section of the archive file
    // Extract file names, permissions, sizes

    // Create directory if it doesn't exist

    // Extract files to the specified directory with their original permissions
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

off_t getFileSize(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size; // Return file size in bytes
    }
    printf("Error occurred while getting %s's file size.\n",filename);
    return -1; // Error occurred while getting file size
}

int getFilePermissions(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO); // Return file permissions // next to stmode would be specific for 3 digit permission
    }
    return -1; // Error occurred while getting permissions
}

int main(int argc, char *argv[]) {
    if (argc < 3) {				//Usage check *
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
		if(argv[i] == "-o"){
			if(argv[i+1] == argv[argc-1]){
					strcat(output_name,".sau");				
				}
			}
		if(isTextFile(argv[i]) > 0){
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
        

        // Call create_archive function
        create_archive(argc, argv, output_name);

    	} else if (strcmp(argv[1], "-a") == 0) {
        // Operation to extract archive
        if (argc < 4) {
            // Display usage instructions for extracting archive
            return 1;
        }

        char *archive_name = argv[2];
        char *directory_name = argv[3];

        // Call extract_archive function
        extract_archive(archive_name, directory_name);
    } else {
        // Display usage instructions for unknown operation
        return 1;
    }

    return 0;
}

void create_archive(int argc, char *argv[], char *outputName) {
    // Parse command-line arguments to get input file names
    char *input_files[MAX_FILES];
    int num_files = 0;
    int i;
    long secSize = 10;
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
    // Extract input file names and count from argv

    // Validate input files and their sizes
    // Check if input files are valid text files and within limits

    // Create archive file and write organization section
    // Write file names, permissions, and sizes to the organization section

    // Write content of each input file to the archive
}
