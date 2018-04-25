#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <ext2fs/ext2_fs.h>

#include "type.h"
#include "globals.h"
#include "getino.c"

MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;
MNTABLE mntable, *mntPtr;

SUPER *sp;
GD    *gp;
INODE *ip;
DIR *dp, *mdp;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[64];

char tempPathName[64];
char gpath[128];   // hold tokenized strings
char *name[64];    // token string pointers
int  n;            // number of token strings

MINODE * iget(int dev, int ino)
{
  printf("iget(%d %d): ", dev, ino);
  return (MINODE *)my_iget(dev, ino);
}

int iput(MINODE *mip)
{
  printf("iput(%d %d)\n", mip->dev, mip->ino);
  return my_iput(mip);
}

int getino(int dev, char *pathname)
{
  return my_getino(dev, pathname);
}

#include "util.c"
#include "ls-cd-pwd.c"
#include "mkdir-creat.c"
#include "alloc-dealloc.c"
#include "rmdir.c"
#include "stat.c"
#include "chmod.c"
#include "link-unlink.c"
#include "symlink.c"
#include "touch.c"
#include "rm.c"
#include "open_close_lseek.c"
#include "read-cat.c"
#include "write-cp-mv.c"


int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    p->pid = i;
    p->uid = 0;
    p->cwd = 0;
    for (j=0; j<NFD; j++)
      p->fd[j] = 0;
  }
}

// load root INODE and set root pointer to it
int mount_root()
{
  printf("mount_root()\n");
  root = iget(dev, 2);
  root->mounted = 1;
  root->mptr = &mntable;

  mntPtr = &mntable;
  mntPtr->dev = dev;
  mntPtr->ninodes = ninodes;
  mntPtr->nblocks = nblocks;
  mntPtr->bmap = bmap;
  mntPtr->imap = imap;
  mntPtr->iblk = iblk;
  mntPtr->mntDirPtr = root;
  strcpy(mntPtr->devName, "mydisk");
  strcpy(mntPtr->mntName, "/");
}

char *disk = "mydisk";
main(int argc, char *argv[ ])
{
  int ino;
  char buf[BLKSIZE];

  if (argc > 1)
    disk = argv[1];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);  exit(1);
  }
  dev = fd;

  /********** read super block at 1024 ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system *****************/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }
  printf("OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf);
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d iblk = %d\n", bmap, imap, iblk);

  init();
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  //printf("hit a key to continue : "); getchar();
  while(1){
    printf("Commands: [ls|cd|pwd|mkdir|creat|rmdir|rm|stat|chmod|link|unlink|touch|open|close|lseek|pfd|quit]\n");
    printf("Input: ");
    fgets(line, 128, stdin);

    line[strlen(line)-1] = 0;

    if (line[0]==0)
      continue;
    pathname[0] = 0;
    char permissions[64];

    sscanf(line, "%s %s %s", cmd, pathname,tempPathName);
    printf("cmd=%s pathname=%s\n", cmd, pathname);


    if(strcmp(cmd,"chmod")==0){
      strcpy(permissions,pathname);
      strcpy(pathname,tempPathName);
      for(int k = 0; k < 64; k++){
	tempPathName[k] = 0;
	}
    }
    char oldfile[64];
    char newfile[64];
    if(strcmp(cmd,"link")==0){
      //Do This In Order To Reorder Arguments
      strcpy(oldfile,pathname);
      strcpy(newfile,tempPathName);
      my_link(oldfile, newfile);
    }
    if(strcmp(cmd,"unlink")==0){
      strcpy(oldfile,pathname); //Do This To Avoid Destorying The Global
      my_unlink(oldfile);
    }
    if (strcmp(cmd, "ls")==0){
      list_file(pathname);
    }
    if (strcmp(cmd, "cd")==0)
      change_dir(pathname);
    if (strcmp(cmd, "pwd")==0){
      pwd(running->cwd);
      printf("\n");
    }
    if (strcmp(cmd, "quit")==0)
       quit();
    if(strcmp(cmd, "mkdir")==0){
      make_dir();
    }
    if(strcmp(cmd, "creat")==0){
      creat_file();
    }
    if(strcmp(cmd, "rmdir")==0){
      rmdir();
    }
    if(strcmp(cmd, "stat")==0){
      get_block(dev,running->cwd->INODE.i_block[0], buf);
      char *cp = buf;
      dp = (DIR *)buf;
      my_stat(dp->inode);
    }
    if(strcmp(cmd, "chmod")==0){
      //Note: Need to do parent and child
      my_chmod(permissions);
    }
    if(strcmp(cmd, "touch")==0){
      my_touch();
    }
    if(strcmp(cmd, "symlink")==0){
      char temp[64];
      strcpy(temp,pathname);
      my_symlink(temp,tempPathName);
    }
    if(strcmp(cmd, "readlink")==0){
      my_readlink();
    }
    if(strcmp(cmd, "rm")==0){
      my_rm();
    }
    if(strcmp(cmd,"open")==0){
      int mode = atoi(tempPathName);
      open_file(mode);
    }
    if(strcmp(cmd,"close")==0){
      int mode = atoi(pathname);
      close_file(mode);
    }
    if(strcmp(cmd, "lseek")==0){
      int fd = atoi(pathname);
      int position = atoi(tempPathName);
      printf("fd: %d\tposition: %d\n",fd,position);
      my_lseek(fd,position);
    }
    if(strcmp(cmd, "pfd")==0){
      my_pfd();
    }
    if(strcmp(cmd, "read")==0){
      read_file();
    }
    if(strcmp(cmd, "cat")==0){
      cat_file();
    }
    if(strcmp(cmd, "write")==0){
      write_file();
    }
    if(strcmp(cmd, "cp")==0){
      my_cp(tempPathName);
    }
    if(strcmp(cmd, "mv")==0){
      my_mv();
    }
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  exit(0);
}
