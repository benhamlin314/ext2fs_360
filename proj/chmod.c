#include "globals.h"

char buf[BLKSIZE];
MINODE *mip;
INODE *ip;

int chmod_file(int ino, char new_i_mode[]){
  mip = iget(dev, ino);
  ip = &(mip->INODE);
  char *Permission = "rwxrwxrwx";

  long int new_imode = strtol(new_i_mode, NULL, 8);
  ip->i_mode = (ip->i_mode & 0xF000) | new_imode;
  printf("New Permissions: ");
  for(int i = 0; i < 9; i++){
    if(ip->i_mode & (1 << (strlen(Permission)-1-i))){
      putchar(Permission[i]);
    }
    else{
      putchar('-');
    }
  }
}

int my_chmod(int ino, char new_i_mode[]){
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  char chbuf[BLKSIZE], temp[256];
  char *cp;
  get_block(dev, ip->i_block[0], chbuf);

  cp = chbuf;
  dp = (DIR *)chbuf;

  while(cp < chbuf+1024){
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    if(strcmp(temp,pathname)==0){
      chmod_file(dp->inode, new_i_mode);
    }

    cp += dp->rec_len;
    dp = (DIR *)cp;
    }
  mip->dirty = 1;
  iput(mip);
}
