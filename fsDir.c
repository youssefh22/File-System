#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fsLow.h"
#include "mfs.h"



extern vcb_t* vcb;
extern int err;

/**
 * @brief Create a Dir object
 * 
 * @param parentAddr - LBA of the directory in which we are creating this dir
 * @return dirEnt_t* - On success: pointer to the created dir (must be freed by caller)
 *                     On failure: NULL
 */
dirEnt_t* createDir(uint64_t parentAddr) {
    // Request blocks, return NULL if allocBlocks fails
    uint64_t dirLoc = allocBlocks(DIR_BLOCKS);
    if(dirLoc == 0) {
        freeBlocks(parentAddr, DIR_BLOCKS);
        return NULL;
    }
   // printf("in createDir: dirLoc = %ld\n", dirLoc);
    dirEnt_t* dir = malloc(sizeof(*dir) * vcb->dirLen);
    if(dir == NULL) {
        err = errno;
        perror("createDir");
        freeBlocks(parentAddr, DIR_BLOCKS);
        return NULL;
    }

    // Initiailize the directory entries
    //printf("initializing entries\n");
    for(int i = 0; i < vcb->dirLen; i++) {
        dir[i].dateTimeCr = 0;
        dir[i].dateTimeMd = 0;
        dir[i].location = 0;
        dir[i].size = 0;
        strncpy(dir[i].name, "", 35);
        dir[i].attr = 0;
    }

    // Initiazize "." entry
    //printf("initializing . entry\n");
    strncpy(dir[0].name, ".", 35);
    dir[0].size = DIR_BLOCKS * vcb->blockSize;
    dir[0].dateTimeCr = time(NULL);
    dir[0].dateTimeMd = dir[0].dateTimeCr;
    dir[0].attr |= DIR_MASK;
    dir[0].location = dirLoc;

    // Initialize ".." entry
    //printf("initializing .. entry\n");
    strncpy(dir[1].name, "..", 35);
    dir[1].size = DIR_BLOCKS * vcb->blockSize;
    dir[1].dateTimeCr = dir[0].dateTimeCr;
    dir[1].dateTimeMd = dir[0].dateTimeCr;
    dir[1].attr |= DIR_MASK;

    /* If parentAddr is 0, we are making the root directory
     * Set ".." location to the same as "." location
     * Set vcb->root to this directory
     */
    if(parentAddr == 0) {
        dir[1].location = dir[0].location;
        vcb->root = dir;
    }
    // Otherwise set to parentAddr
    else {
        dir[1].location = parentAddr;
    }

    // Write this directory to disk
    int ret = LBAwrite(dir, DIR_BLOCKS, dirLoc);

    if(ret != DIR_BLOCKS) {
        free(dir);
        dir = NULL;
        freeBlocks(parentAddr, DIR_BLOCKS);
    }
    
    return dir;
}

int deleteDirEnt(dirEnt_t* parent, int index) {
    uint64_t addr = parent[index].location;

    parent[index].dateTimeCr = 0;
    parent[index].dateTimeMd = 0;
    parent[index].location = 0;
    parent[index].size = 0;
    parent[index].attr = 0;
    strncpy(parent[index].name, "", strlen(parent[index].name));

    freeBlocks(addr, DIR_BLOCKS);

    LBAwrite(parent, DIR_BLOCKS, parent[0].location);

    return 1;
}


/**
 * @brief Loads a directory from the disk to the buffer passed from the caller
 * 
 * @param buf - buffer to fill with the directory
 * @param location - LBA of the directory we are loading
 * @return int - On success: 0
 *               On failure: errno
 */
int loadDir(dirEnt_t* buf, uint64_t location) {
    printf("loadDir: location: %lu\n", location);
    int ret = LBAread(buf, DIR_BLOCKS, location);
    if(ret != DIR_BLOCKS) {
        err = errno;
        perror("loadDir");
        return err;
    } 
    printf("loadDir: buf location: %lu\n", buf[0].location);
    return 0;
}


/**
 * @brief Searches a directory for a directory entry with a given name
 * 
 * @param dir - the directory being searched
 * @param token - the name of the file we are searching for
 * @return int - If found: the index of the found entry
 *               If not found: -1
 */
int searchDir(dirEnt_t* dir, char* token) {
    int index = -1;
    printf("searchDir: token= %s\n",token);
    // Loop through each entry in this directory
    for(int i = 0; i < vcb->dirLen; i++) {
        // If the token and an entry name match, load that direcotry entry
        if(strcmp(token, dir[i].name) == 0) {
            index = i;
            break;
        }
    }
    
    return index;
}

/**
 * @brief Check if a file at a given index is a directory
 * 
 * @param parent - parent directory of the entry we are checking
 * @param index - index of the entry we are checking
 * @return int - On success: 1
 *               On Failure: 0
 */
int isDir(dirEnt_t* parent, int index) {
    return (parent[index].attr & DIR_MASK);
}


char* buildPath(char* path, char* token) {
    printf("buildPath: path: %s  token: %s\n", path, token);
    // If token is "..", remove the last element of the path
    if(strcmp(token, "..") == 0) {
        if(strcmp(path, "/") != 0) {
            char* toRemove = strrchr(path, '/');
            *toRemove = '\0';
            toRemove = strrchr(path, '/');
            *(toRemove + 1) = '\0';
            printf("buildPath path: %s\n", path);
        }
    }
    // Else, if the token is not ".", add it to the path
    else if(strcmp(token, ".") != 0) {
        strcat(path, token);
        strcat(path, "/");
    }
    printf("new path: %s\n", path);
    return path;
}


/**
 * @brief Parses a path given to it, filling the struct pathInfo buffer passed
 * to it.
 * 
 * @param buf - the pathInfo struct we are filling for the caller
 * @param path - the pathname we are parsing
 * @return int - On success: 0
 *               On Failure: -1
 * NOTE: If parsePath fails, the data in buf cannot be guaranteed to be valid
 */
int parsePath(struct pathInfo* buf, const char* path) {
    dirEnt_t* curDir; // Current directory we are looking in
    char* pathCpy = malloc(strlen(path) + 1);
    strcpy(pathCpy, path);

    char absPath[MAX_PATH] = "\0";
    //printf("pathCpy: %s\nlen: %ld\n", pathCpy, strlen(pathCpy));

    // If path begins with "/" it is an absolute path, begin at root directory
    if(path[0] == '/') {
        //printf("parse path starting at root\n");
        strcpy(absPath, "/\0");
        curDir = vcb->root;
    }
    // Otherwise, it is a relative path, begin at current working directory
    else {
        //printf("parse path starting at cwd\n");
        strcpy(absPath, vcb->cwdName);
        curDir = vcb->cwd;
    }

    enum fType status = DNE; // has values of DNE, File, or Dir
    char* token; // Token returned by strtok
    int index; // Index in directory
    

    // Tokenize with strtok
    token = strtok(pathCpy, "/");
    dirEnt_t* prevDir = NULL;
    dirEnt_t* nextDir = malloc(DIR_BLOCKS * vcb->blockSize);

    // Loop through all tokens
    while(token != NULL) {
        printf("token: %s\n", token);
        // Re-set staus each loop
        status = DNE;
        // Search the loaded directory for a filename matching the token
        index = searchDir(curDir, token);
//      printf("seaching in %lu\n", curDir[0].location);
//      printf("searchDir: %d\n", index);
        
        if(index != -1) {
            // If the found entry is a directory, load it
            if(isDir(curDir, index)) {
                status = Dir;
//              printf("loading dir: %s\n", token);
                int ret = loadDir(nextDir, curDir[index].location);
                if(ret != 0) {
                    return -1;
                }
                buildPath(absPath, token);
            }
            else {
                status = File;
            }
        }


        // Get next token
        token = strtok(NULL, "/");
        
    }
    buildPath(absPath, "..");
    buf->lastElem = status;
    buf->index = index;
    buf->parent = curDir;
    buf->parentLoc = curDir[1].location;
    strcpy(buf->absPath, absPath);
    
    //printf("exiting: freeing next dir\n");
    if(nextDir) {
        if(nextDir != curDir && nextDir != vcb->root && nextDir != vcb->cwd) {
            free(nextDir);
            nextDir = NULL;
        }
    }
    //printf("exiting: freeing pathCpy\n");
    if(pathCpy) {
        free(pathCpy);
        pathCpy = NULL;
    }
    
    //printf("absPath: %s\n", absPath);
    return 0;
}


// Returns index of next free directory entry or -1 if none are free
int getFreeDirEnt(dirEnt_t* dir) {
    int ret = -1;
    for(int i = 0; i < vcb->dirLen; i++) {
        if(dir[i].name[0] == '\0') {
            ret = i;
            break;
        }
    }

    return ret;
}


int fs_mkdir(const char* pathname, mode_t mode) {
    struct pathInfo info;
    int ret;

    // call parsePath and check return
    if(parsePath(&info, pathname) == 0) {
        // If the last element in the path already exists, set return val to 0
        if(info.lastElem != DNE) {
            printf("File or Directory already exists\n");
            ret = 0;
        }
        // Otherwise, get the next free directory entry
        else {
            int index = getFreeDirEnt(info.parent);
            printf("getFreeDirent: %d\n", index);

            if(index == -1) {
                ret = 0;
            }
            // Call createDir to make the new dir
            else {
                dirEnt_t* dir = createDir(info.parentLoc);
                if(dir == NULL) {
                    ret = 0;
                }
                // Set the parent's entry to the new dir
                else {
                    info.parent[index] = *dir;

                    // Get the name of the dir, the last token in pathname
                    char* name = strrchr(pathname, '/');
                    if(name != NULL) {
                        name = name + 1;
                        printf("name: %s\n", name);
                    }
                    else {
                        char temp[strlen(pathname) + 1];
                        strcpy(temp, pathname);
                        name = temp;
                    }
                    strcpy(info.parent[index].name, name);

                    LBAwrite(info.parent, DIR_BLOCKS, info.parentLoc);

                    ret = 1;
                }
            }
        
        }
        if(info.parent) {
            if(info.parent != vcb->root && info.parent != vcb->cwd) {
              printf("mkdir: freeing parent\n");
                free(info.parent);
                info.parent = NULL;
            }
        }
    }

    return ret;
}

int fs_isDir(char* path) {
    struct pathInfo info;
    int ret = 0;
    if(parsePath(&info, path) == 0) {
        if(info.lastElem == Dir) {
            ret = 1;
        }
    }
    if(info.parent) {
        if(info.parent != vcb->cwd && info.parent != vcb->root) {
            free(info.parent);
        }
    }

    return ret;
}

int fs_isFile(char* path) {
    struct pathInfo info;
    int ret = 0;
    if(parsePath(&info, path) == 0) {
        if(info.lastElem == File) {
            ret = 1;
        }
    }
    if(info.parent) {
        if(info.parent != vcb->cwd && info.parent != vcb->root) {
            free(info.parent);
        }
    }

    return ret;
}

char* fs_getcwd(char* buf, size_t size) {
    return strncpy(buf, vcb->cwdName, size);
}

int fs_setcwd(char* buf) {
    struct pathInfo info;
    int ret = -1;
    if(parsePath(&info, buf) == 0) {
        if(info.lastElem == Dir) {
            dirEnt_t* dir = malloc(DIR_BLOCKS * vcb->blockSize);
            printf("setcwd: loading %u\n", info.index);
            printf("cwd: info.parent[index] location: %lu\n", info.parent[info.index].location);
            int chk = loadDir(dir, info.parent[info.index].location);
            if(chk == 0) {
                printf("setcwd: index: %d\n", info.index);
                char* path = malloc(MAX_PATH);
                strcpy(path, info.absPath);
                printf("absPath: %s\n", info.absPath);
                buildPath(path, info.parent[info.index].name);
                //strcat(path, info.parent[info.index].name);
                //strcat(path, "/");
                strcpy(vcb->cwdName, path);
                free(path);
                printf("cwd: parentLoc: %lu\n", vcb->cwd[1].location);

                printf("cwd: dirloc: %lu\n", dir[0].location);
                if(vcb->cwd != vcb->root && vcb->cwd != info.parent) {
                    free(vcb->cwd);
                }
                printf("cwd: parentLoc: %lu\n", vcb->cwd[1].location);
                vcb->cwd = dir;
                printf("cwd=== %lu\n\n", vcb->cwd[0].location);
                ret = 0;
            }
            else {
                printf("setcwd: freeing dir\n");
                free(dir);
            }
        }
    }
    if(info.parent) {
        printf("setcwd: freeing parent\n");
        if(info.parent != vcb->cwd && info.parent != vcb->root) {
            free(info.parent);
        }
    }
    printf("cwd loc: %lu\n", vcb->cwd[0].location);

    return ret;
}

int fs_delete(char* filename) {
    struct pathInfo info;
    int ret = 0;
    if(parsePath(&info, filename) == 0) {
        if(info.lastElem == File) {
            deleteDirEnt(info.parent, info.index);
            ret = 1;
        }
    }
    if(info.parent) {
        if(info.parent != vcb->cwd && info.parent != vcb->root) {
            free(info.parent);
        }
    }

    return ret;
}

fdDir* fs_opendir(const char* name) {
    struct pathInfo info;
    fdDir* dirp;
    if(parsePath(&info, name) == 0) {
        fdDir* dirp = malloc(sizeof(*dirp));
        dirp->dirPtr = malloc(DIR_BLOCKS * vcb->blockSize);
        loadDir(dirp->dirPtr, info.parent[info.index].location);
        dirp->directoryStartLocation = dirp->dirPtr[0].location;
        dirp->dirEntryPosition = 0;
        dirp->d_reclen = DIR_BLOCKS * vcb->blockSize;
        dirp->diInfo = malloc(sizeof(*dirp->diInfo));
    }
    else {
        dirp = NULL;
    }

    return dirp;
}

struct fs_diriteminfo* fs_readdir(fdDir* dirp) {
    strcpy(dirp->diInfo->d_name, dirp->dirPtr->name);
    dirp->diInfo->d_reclen = dirp->dirPtr->size;
    if(dirp->dirPtr->attr & DIR_MASK) {
        dirp->diInfo->fileType = 'D';
    }
    else {
        dirp->diInfo->fileType = 'F';
    }

    return dirp->diInfo;
}

int fs_closedir(fdDir* dirp) {
    
}

int fs_rmdir(const char* pathname) {
    struct pathInfo info;
    int ret = 0;

    if(parsePath(&info, pathname) == 0) {
        dirEnt_t* dir = malloc(DIR_BLOCKS * vcb->blockSize);
        loadDir(dir, info.parent[info.index].location);

        for(int i = 0; i < vcb->dirLen; i++) {

        }
    }

}

