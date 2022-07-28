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

int createDir(uint64_t parentAddr) {
    int numEnts = DIR_BLOCKS * vcb->blockSize / sizeof(dirEnt_t);
    uint64_t dirLoc = allocBlocks(DIR_BLOCKS);

    dirEnt_t* dir = malloc(numEnts * sizeof(*dir));
    if(dir == NULL) {
        err = errno;
        perror("createDir");
        return -1;
    }

    // Initiailize the directory entries
    for(int i = 0; i < numEnts; i++) {
        dir[i].dateTimeCr = 0;
        dir[i].dateTimeMd = 0;
        dir[i].location = 0;
        dir[i].size = 0;
        strncpy(dir[i].name, "", 35);
        dir[i].attr = 0;
    }

    strncpy(dir[0].name, ".", 35);
    strncpy(dir[1].name, "..", 35);
    dir[0].size = 2;
    dir[0].location = dirLoc;
    dir[0].dateTimeCr = time(NULL);
    dir[0].dateTimeMd = dir[0].dateTimeCr;

    if(parentAddr == 0) {
        dir[0].attr |= ROOT_MASK;
        dir[1].attr |= ROOT_MASK;
        dir[1].size = 2;
        dir[1].location = dir[0].location;
        dir[1].dateTimeCr = dir[0].dateTimeCr;
        dir[1].dateTimeMd = dir[0].dateTimeCr;

        vcb->root = dir;
    }
    else {
        dir[0].attr |= DIR_MASK;
        dir[0].attr |= DIR_MASK;
        dir[1].location = parentAddr;
    }
}

int loadDir(dirEnt_t* buf, uint64_t location) {
    int ret = LBAread(buf, DIR_BLOCKS, location);
    if(ret != DIR_BLOCKS) {
        err = errno;
        perror("loadDir");
        return err;
    }

    return 0;
}

fdDir* fsopendir(const char* name) {

}

int parsePath(struct pathInfo* buf, char* path) {
    int ret;
    dirEnt_t* tempDir = malloc(DIR_BLOCKS * vcb->blockSize);
    if(tempDir == NULL) {
        err = errno;
        perror("parsePath");
        return errno;
    }

    if(path[0] == '/') {
        ret = loadDir(tempDir, vcb->rootAddr);  
    }
    else {
        ret = loadDir(tempDir, vcb->cwd->location);
    }

    if(ret == 0) {
        int found;
        int prevElemDNE = 0;
        char* token;
        token = strtok(path, "/");
        while(token != NULL) {
            found = 0;
            for(int i = 0; i < vcb->numEnts; i++) {
                if(strcmp(token, tempDir[i].name) == 0) {
                    uint64_t tempLoc = tempDir[i].location;
                    ret = loadDir(tempDir, tempLoc);
                    found = 1;
                    break;
                }
            }

            token = strtok(path, "/");
            if(token != NULL && !found) {
                ret = -1;
                break;
            }
        }

        if()
    }
}


int fs_mkdir(const char* pathname, mode_t mode) {

}

int fs_isDir(char* path) {
    if(strchr(path, "/") == NULL) {
        
    }
}

