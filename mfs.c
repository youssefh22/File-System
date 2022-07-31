#include <sys/types.h>
#include <sys/types.h>

#include <time.h>

#include "mfs.h"

// Need to LBA write the directory //can make a function 
 fs_writedir(parent){ //can call LBAwrite to write to get it to disk
  int length = vcb_t.dirLen;//  look at the lengthof the directory
  allocBlocks(length);//  write that length
 }

fs_mkdir(char *pathname, mode_t mode){ //time 24
  // parsePath();//call parse path
  if(parsePath() != 0)
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
  timeCr = parent[].dateTimeC;
  timeMd = parent[].dateTimeMd;
  attr   = parent[].attr;  
  writedir(parent);
}

fs_setcwd(char *buf){ //time 29
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
  // call parse path
   if(!(lastElem) && fs_isfile == 0){
     deleteEntry(parent, i) //Release blob & sets dirEntry to unused
   }
}

fs_rmdir(const char *pathname){ //min 35
  // parse path
   if(lastElem && fs_isDir == 0){
      load(parent);
      scan(parent);
      if( lastElem.Dir == '.' && lastElem.Dir == '..'){ //dir is Empty
        deleteEntry(parent,i);
      }else{
       return -1;
      }
    }
}

fs_getcwd(char *buf, size_t size){//min 38
 strcpy(buf,vcb_t.cwdName);
}

fs_opendir(const char *name){ //min 44
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


// fill in from a call to fs_stat
fs_stat(const char *path, struct fs_stat *buf){

   st_size;    		
	 st_blksize; 	
	 st_blocks;  		
	 st_accesstime;   	
	 st_modtime;   	
	 st_createtime; 
}

fs_isFile(char * path){
  if(lastElem.File) //if last element is file
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
fs_isDir(char * path){
  if(lastElem.Dir) //if last element is directory
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

closeDir(){}