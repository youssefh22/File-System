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
        return NULL;
    }
    printf("in createDir: dirLoc = %ld\n", dirLoc);
    dirEnt_t* dir = malloc(sizeof(*dir) * vcb->dirLen);
    if(dir == NULL) {
        err = errno;
        perror("createDir");
        return NULL;
    }

    // Initiailize the directory entries
    printf("initializing entries\n");
    for(int i = 0; i < vcb->dirLen; i++) {
        dir[i].dateTimeCr = 0;
        dir[i].dateTimeMd = 0;
        dir[i].location = 0;
        dir[i].size = 0;
        strncpy(dir[i].name, "", 35);
        dir[i].attr = 0;
    }

    // Initiazize "." entry
    printf("initializing . entry\n");
    strncpy(dir[0].name, ".", 35);
    dir[0].size = DIR_BLOCKS * vcb->blockSize;
    dir[0].dateTimeCr = time(NULL);
    dir[0].dateTimeMd = dir[0].dateTimeCr;
    dir[0].attr |= DIR_MASK;
    dir[0].location = dirLoc;

    // Initialize ".." entry
    printf("initializing .. entry\n");
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
        dir[1].location = dirLoc;
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
    }
    
    return dir;
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
    int ret = LBAread(buf, DIR_BLOCKS, location);
    if(ret != DIR_BLOCKS) {
        err = errno;
        perror("loadDir");
        return err;
    }

    return 1;
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


int parsePath(struct pathInfo* buf, const char* path) {
    dirEnt_t* curDir; // Current directory we are looking in
    char* pathCpy = malloc(strlen(path) + 1);
    strcpy(pathCpy, path);
    printf("pathCpy: %s\nlen: %ld\n", pathCpy, strlen(pathCpy));

    // If path begins with "/" it is an absolute path, begin at root directory
    if(path[0] == '/') {
        printf("parse path starting at root\n");
        curDir = vcb->root;
    }
    // Otherwise, it is a relative path, begin at current working directory
    else {
        printf("parse path starting at cwd\n");
        curDir = vcb->cwd;
    }

    enum fType status; // has values of DNE, File, or Dir
    char* token; // Token returned by strtok
    int index; // Index in directory
    char absPath[MAX_PATH] = "/\0";

    // Tokenize with strtok
    token = strtok(pathCpy, "/");
    dirEnt_t* prevDir = NULL;
    dirEnt_t* nextDir = malloc(vcb->dirLen * sizeof(*nextDir));

    // Loop through all tokens
    while(token != NULL) {
        // Initialize found for each token
        // found = 0;
        //
        //
        //  Loop through each entry in this directory
        //  for(int i = 0; i < vcb->dirLen; i++) {
        //    
        //       If the token and an entry name match, load that direcotry entry
        //       if(strcmp(token, tempDir[i].name) == 0) {
        //           // uint64_t tempLoc = tempDir[i].location;
        //           // ret = loadDir(tempDir, tempLoc);
        //           tempDir = malloc(DIR_BLOCKS * vcb->blockSize);
        //           if(tempDir == NULL) {
        //               err = errno;
        //               perror("parsePath");
        //               return err;
        //           }
        //           ret = loadDir(tempDir, tempDir[i].location);
        //           if(ret != 0) {
        //               printf("ERROR: parsePath: loadDir\n");
        //               free(tempDir);
        //               return -1;
        //           }
        //           found = 1;
        //           break;
        //       }
        //  }
        //  if(found == 1) {
        //    
        //  }
        status = DNE;
        // Search the loaded directory for a filename matching the token
        index = searchDir(curDir, token);
        if(index != -1) {
            // If the found entry is a directory, load it
            if(isDir(curDir, index)) {
                status = Dir;
                strcat(absPath, token);
                strcat(absPath, "/");
                int ret = loadDir(nextDir, curDir[index].location);
                if(ret != 0) {
                    return -1;
                }
            }
            else {
                status = File;
            }
        }

        // Get next token
        token = strtok(NULL, "/");
        printf("token: %s\n", token);
        // Free prevDir if it has been malloc'd and is not one of the vcb dirs
        if(prevDir && prevDir != vcb->root && prevDir != vcb->cwd) {
            free(prevDir);
            prevDir = NULL;
        }

        // Handle cases when we are not at the end of the specified path
        if(token != NULL) {
            // If we found a directory, update prevDir and curDir
            if(status == Dir) {
                prevDir = curDir;
                curDir = nextDir;
            }
            // Otherwise, the path is invalid. Clean-up and return negative value
            else {
                printf("invalid path\n");
                // Make sure not to free directories stored in the vcb
                if(curDir && curDir != vcb->root && curDir != vcb->cwd) {
                    free(curDir);
                    curDir = NULL;
                }
                if(nextDir && nextDir != vcb->root && nextDir != vcb->cwd) {
                    free(nextDir);
                    nextDir = NULL;
                }
                free(pathCpy);
                
                return -1;
            } 
        }
    }

    buf->lastElem = status;
    buf->index = index;
    buf->parent = curDir;
    buf->parentLoc = curDir[0].location;
    strcpy(buf->absPath, absPath);
    
    //if(nextDir && nextDir != vcb->root && nextDir != vcb->cwd) {
        free(nextDir);
        nextDir = NULL;
    //}
    free(pathCpy);
    pathCpy = NULL;

    return 0;
}


fdDir* fs_opendir(const char *name) {
    dirEnt_t* curDir;
    if(name[0] == '/') {
        curDir = vcb->root;
    }

    fdDir* dirp = malloc(sizeof(*dirp));

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
                // Set the parent entry to the new dir
                else {
                    info.parent[index] = *dir;

                    // Get the name of the dir, the last token in pathname
                    char* name = strrchr(pathname, '/');
                    if(name != NULL) {
                        name = name + 1;
                        printf("name: %s\n", name);
                    }
                    else {
                        name = pathname;
                    }
                    strcpy(info.parent[index].name, name);

                    LBAwrite(info.parent, DIR_BLOCKS, info.parentLoc);

                    ret = 1;
                }
            }
        
        }
        if(info.parent && info.parent != vcb->root && info.parent != vcb->cwd) {
            free(info.parent);
            info.parent = NULL;
        }
    }

    return ret;
}


char* fs_getcwd(char* buf, size_t size) {
    return strncpy(buf, vcb->cwdName, size);
}

// int fs_setcwd(char* buf) {
//     if(fs_isDir(buf)) {

//     }
// }