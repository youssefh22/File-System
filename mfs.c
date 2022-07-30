
// Need to LBA write the directory //can make a function 
 fs_writedir(parent){ //can call LBAwrite to write to get it to disk
//  look at the length of the directory
//  write that length
 }

fs_mkdir(char *pathname, mode_t mode){ //time 24
  /* 
  * call parse path
  * if(error){
  * get out
  * }
  * if(lastElement index =< 0){
  * get out //bc this means last element exist
  * }
  * location = createDir(parent[0].loaction);//from milestone 1
  * index = get a free dir entry (parent) //find a unused directory entry
  * parent[index].loaction = location
  *               .size = size
  *                .name = name
  *                   ...
  * writeDir(parent);
  */ 

  parsePath();//call parse path
  if(error)
  {
    return -2;
  }
  if(lastElem.DNE)//bc this means last element exist
  { 
    return -1;
  }
  
  int location = createDir(parent[0].location);//from milestone 1
  int index = freeDirEntry(parent); //find a unused directory entry
  //from 
  location  = parent[index].location;
  size  = parent[].size;
  name = parent[].name; 
  ....
  writedir(parent);
}

fs_setcwd(char *buf){ //time 29
/*
* parse path
* if(lastElement exist && is a Dir){
*  // from parse path
* DirEntry * cwdpointer = LoadDir(parent[i]);
* char * cwdName = Malloc() 
*           strcpy(cwd,path)//make an absulote path       
* }else{
* error 
* } 
*/
  parsePath();//call parse path
  if(!(lastElem.DNE) && fs_isDir == 0)
  {
    dirEnt * cwdpointer = loadDir(parent[i]); //from parse path
    char * cwdName = malloc(); 
    strcpy(cwd,path);
    //make an absulote path       
  }else
  {
    return -1; //error 
  } 
}
fs_delete(char* filename){ //min 33
  /*
  * parse path
  * if(lastElement exist && isfile){
  *   deleteEntry(parent, i) //Release blob & sets dirEntry to unused
  * }
  */
  // call parse path
   if(!(lastElem) && fs_isfile == 0){
     deleteEntry(parent, i) //Release blob & sets dirEntry to unused
   }
}

fs_rmdir(const char *pathname){ //min 35
  /*
  * parse path
  * if(lastElement exist && is a Dir){
  *   load(parent)
  *   scan parent
  *   if( . && ..){ //dir is Empty
  *     deleteEntry(parent,i)
  *   }else{
  *     get out
  *   }
  */
  // parse path
   if(lastElem && fs_isDir == 0){
      load(parent);
      scan(parent);
      if( . && ..){ //dir is Empty
        deleteEntry(parent,i);
      }else{
       return -1;
      }
    }
}

fs_getcwd(char *buf, size_t size){//min 38
  /*
  *strcpy(buffer,cwdName);
  */
  strcpy(buffer,cwdName);
}

fs_opendir(const char *name){ //min 44
  /*
  * parse path
  * if(lastElement exist && isDir){
  *   fdDir returnValue = malloc(sizeof(fdDir))
  *   returnValue -> dir = LoadDir(parent[i])
  *   returValue -> current =0;
  *   returValue -> max = parent[i].size/sizeof(DirEntry)
  *   return(retuenValue)
  * }else{
  *   error
  * }
  */
  // call parse path
   if(lastElem && fs_isDir == 0){
     fdDir returnValue = malloc(sizeof(fdDir));
     returnValue->dir = LoadDir(parent[i]);
     returValue->current = 0;
     returValue->max = parent[i].size/sizeof(DirEntry);
     return(retuenValue);
   }else{
     return -1;
   }
}

fs_readdir(fdDir *dirp){//min 47
  /*
  * for(i = fdDir->current; i< fdDir->max; i++){
  *   if(fdDir->Directory[i] is used){
  *     copy name of fdDir->Directory[i].name to fdDir->item.name
  *                   fdDir->item.type = fileDirectory
  *                   fdDir->current = i+1; //next element 
  *     return (&fdDir->item) //address
  *   }
  * }
  * return null //bc none of the remainding entries are used
  */
  for(int i = fdDir->current; i < fdDir->max; i++){
     if(fdDir->directory[i]){ //is used
        fdDir->item.name = fdDir->Directory[i].name;
        // fdDir->item.type = file or Directory
        fdDir->current = i+1; //next element 
        return (&fdDir->item); //address
     }
   }
   return null;
}




closeDir(){

}

fs_stat(){

}

fs_isFile(){
  if(lastElem.File)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
fs_isDir(){
  if(lastElem.Dir)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


//Return value of Parse path  = 0 , path valid or
//                          non 0 , path invalid
/*
struct 
diEntry * dir //memory pointer to ram
int index // last Element
          //  -1  , not found
          //  > 0 , is index
dir[0].location // location of current dir 
*/















//#include "mfs.h"
//fs_Dir* inodes;
//
//char requestedFilePath[FILEPATH_SIZE];
//char requestedFilePathArray[MAX_DEPTH][FILENAME_SIZE];
//int requestedFilePathArraySize = 0;
//
//char currentDirectoryPath[FILEPATH_SIZE];
//char currentDirectoryPathArray[MAX_DEPTH][FILENAME_SIZE];
//int currentDirectoryPathArraySize = 0;
//
//void parseFilePath(const char *pathname) { 
//
//  // Clear previous 
//  requestedFilePath[0] = '\0';
//  requestedFilePathArraySize = 0;
//
//  //copy of pathname
//  char copy_pathname[FILEPATH_SIZE];
//  strcpy(copy_pathname, pathname);
//
//  char* savePointer;
//  char* token = strtok_r(copy_pathname, "/", &savePointer);
//
//  int isAbsolute = pathname[0] == '/';
//  int isChild = !strcmp(token, ".");
//  int isParent = !strcmp(token, "..");
//
//  if(token && !isAbsolute) {
//    int maxLevel = isParent? currentDirectoryPathArraySize - 1 : currentDirectoryPathArraySize;
//    for(int i=0; i<maxLevel; i++) {
//      strcpy(requestedFilePathArray[i], currentDirectoryPathArray[i]);
//      sprintf(requestedFilePath, "%s/%s", requestedFilePath, currentDirectoryPathArray[i]);
//      requestedFilePathArraySize++;
//    }
//  }
//
//  //Take away . && ..
//  if(isChild || isParent) {
//    token = strtok_r(0, "/", &savePointer);
//  }
//
//  while(token && requestedFilePathArraySize < MAX_DEPTH) {
//
//    strcpy(requestedFilePathArray[requestedFilePathArraySize], token);
//    sprintf(requestedFilePath, "%s/%s", requestedFilePath, token);
//
//    requestedFilePathArraySize++;
//    token = strtok_r(0, "/", &savePointer);
//
//  }
//}//End of ParseFilePath
//
//
//
//int fs_mkdir(const char *pathname, mode_t mode) {
//    char parentPath[256] = "";
//  parseFilePath(pathname);
//
//  for (size_t i = 0; i < requestedFilePathArraySize - 1; i++) {
//     strcat(parentPath, "/");
//     strcat(parentPath, requestedFilePathArray[i]);
//  }
//  
//  parent = //getInode with parentPath
//  if (parent) {
//    for (size_t i = 0; i < /*parent to number children*/; i++){
//      if(strcmp(/*parent to array of children[i]*/, requestedFilePathArray[requestedFilePathArraySize - 1])){
//          printf("Already exists\n");
//          return -1;
//      }
//    }
//  } else {
//    printf("Non existing Parent : '%s'\n", parentPath);return -1;
//  }
//  
//  //Create a Inode and return 0
//
//  printf("Failed to make directory : '%s'.\n", pathname);
//  return -1;
//}//End of fs_mkdir
//
//char * fs_getcwd(char *buf, size_t size) {
//  if(strlen(currentDirectoryPath) > size) {
//    errno = ERANGE;
//    return NULL;
//  }
//  strcpy(buf, currentDirectoryPath);
//  return buf;
//}//End of fs_getcwd
//
//int fs_isFile(char * path) {
//  inode = //get Inode with path
//  return inode ? inode->type == I_FILE : 0;
//}//end of fs_isFile
//
//int fs_isDir(char * path) {
//  inode = //get Inode with path
//  return inode ? inode->type == I_DIR : 0;
//}
//
// fs_opendir(const char *fileName) {
//  int x = b_open(fileName, 0);
//  if(x < 0) {
//    return NULL;
//  }
//  return//get Inode with fileName
//}//END of fs_opendir
//
////fs_setcwd
////fs_readdir
////fs_closedir
////fs_stat
////fs_delete
////fs_rmdir

