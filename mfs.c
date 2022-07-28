
#include "mfs.h"
fs_Dir* inodes;

char requestedFilePath[FILEPATH_SIZE];
char requestedFilePathArray[MAX_DEPTH][FILENAME_SIZE];
int requestedFilePathArraySize = 0;

char currentDirectoryPath[FILEPATH_SIZE];
char currentDirectoryPathArray[MAX_DEPTH][FILENAME_SIZE];
int currentDirectoryPathArraySize = 0;

void parseFilePath(const char *pathname) { 

  // Clear previous 
  requestedFilePath[0] = '\0';
  requestedFilePathArraySize = 0;

  //copy of pathname
  char copy_pathname[FILEPATH_SIZE];
  strcpy(copy_pathname, pathname);

  char* savePointer;
  char* token = strtok_r(copy_pathname, "/", &savePointer);

  int isAbsolute = pathname[0] == '/';
  int isChild = !strcmp(token, ".");
  int isParent = !strcmp(token, "..");

  if(token && !isAbsolute) {
    int maxLevel = isParent? currentDirectoryPathArraySize - 1 : currentDirectoryPathArraySize;
    for(int i=0; i<maxLevel; i++) {
      strcpy(requestedFilePathArray[i], currentDirectoryPathArray[i]);
      sprintf(requestedFilePath, "%s/%s", requestedFilePath, currentDirectoryPathArray[i]);
      requestedFilePathArraySize++;
    }
  }

  //Take away . && ..
  if(isChild || isParent) {
    token = strtok_r(0, "/", &savePointer);
  }

  while(token && requestedFilePathArraySize < MAX_DEPTH) {

    strcpy(requestedFilePathArray[requestedFilePathArraySize], token);
    sprintf(requestedFilePath, "%s/%s", requestedFilePath, token);

    requestedFilePathArraySize++;
    token = strtok_r(0, "/", &savePointer);

  }
}//End of ParseFilePath



int fs_mkdir(const char *pathname, mode_t mode) {
    char parentPath[256] = "";
  parseFilePath(pathname);

  for (size_t i = 0; i < requestedFilePathArraySize - 1; i++) {
     strcat(parentPath, "/");
     strcat(parentPath, requestedFilePathArray[i]);
  }
  
  parent = //getInode with parentPath
  if (parent) {
    for (size_t i = 0; i < /*parent to number children*/; i++){
      if(strcmp(/*parent to array of children[i]*/, requestedFilePathArray[requestedFilePathArraySize - 1])){
          printf("Already exists\n");
          return -1;
      }
    }
  } else {
    printf("Non existing Parent : '%s'\n", parentPath);return -1;
  }
  
  //Create a Inode and return 0

  printf("Failed to make directory : '%s'.\n", pathname);
  return -1;
}//End of fs_mkdir

char * fs_getcwd(char *buf, size_t size) {
  if(strlen(currentDirectoryPath) > size) {
    errno = ERANGE;
    return NULL;
  }
  strcpy(buf, currentDirectoryPath);
  return buf;
}//End of fs_getcwd

int fs_isFile(char * path) {
  inode = //get Inode with path
  return inode ? inode->type == I_FILE : 0;
}//end of fs_isFile

int fs_isDir(char * path) {
  inode = //get Inode with path
  return inode ? inode->type == I_DIR : 0;
}

 fs_opendir(const char *fileName) {
  int x = b_open(fileName, 0);
  if(x < 0) {
    return NULL;
  }
  return//get Inode with fileName
}//END of fs_opendir

//fs_setcwd
//fs_readdir
//fs_closedir
//fs_stat
//fs_delete
//fs_rmdir