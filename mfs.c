
/* Need to LBA write the directory //can make a function 
*  
* fs_writedir(parent){ can call LBAwrite to write to get it to disk
*  look at the length of the directory
*  write that length
* }
*/

fs_mkdir(){
  /* 
  * call parse path
  * if(error){
  * get out}
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
}
fs_setcwd(path){
/*
*parse path
* if(lastElement exist && is a Dir){
*  // from parse path
* DirEntry * cwdpointer = LoadDir(parent[i]);
* char * cwdName = Malloc() 
*           strcpy(cwd,path)//make an absulote path       
* }else{
* error 
* } 
*/
}
fs_delete(path){
  /*
  * parse path
  * if(lastElement exist && isfile){
  *   deleteEntry(parent, i) //Release blob & sets dirEntry to unused
  * }
  */
}

fs_rmdir(){
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
  * }
  */
}

fs_getcwd(){
  /*
  *strcpy(buffer,cwdName);
  */
}

fs_opendir(path){
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
}

fs_readdir(fdDir){
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
}




closeDir(){

}

fs_stat(){

}

fs_isFile(){
/*
* //parse !open == file
* if(){
* 
*   return 1//true
* }else{
*   return 0//false
* }
*/
}
fs_isDir(){
/*
* //parse open == dir
* if(){
* 
*   return 1//true
* }else{
*   return 0//false
* }
*/
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

