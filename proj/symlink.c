#include "globals.h"

int my_symlink(char *oldname, char *newname){
  int inumber = getino(dev, oldname);
  if(inumber){//if inumber not 0 then oldname exists
    strcpy(pathname,newname);
    creat_file();
    int ino2 = getino(dev, newname);
    MINODE *mip = iget(dev, ino2);
    mip->INODE.i_mode = 0120000;
    char buf[BLKSIZE];
    int blkno = balloc(dev);
    mip->INODE.i_block[0] = blkno;
    get_block(dev,blkno,buf);
    strcpy(buf,oldname);
    put_block(dev,blkno,buf);
    mip->INODE.i_size = strlen(oldname)+1;
    mip->dirty = 1;
    iput(mip);
  }
}

char * my_readlink(){
  int ino = getino(dev, pathname);
  MINODE * mip = iget(dev, ino);
  char buf[BLKSIZE];
  int blk = mip->INODE.i_block[0];
  get_block(dev,blk,buf);
  return buf;
}
