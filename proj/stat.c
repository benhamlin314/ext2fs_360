#include "globals.h"

char buf[BLKSIZE];
MINODE *mip;
INODE *ip;


int stat_file(int ino,char temp[]){
  mip = iget(dev, ino);
  ip = &(mip->INODE);
  char *Permission = "rwxrwxrwx";
  printf("Name:        %s\n",temp);
  printf("Permissions: ");
  for(int i = 0; i < 9; i++){
    if(ip->i_mode & (1 << (strlen(Permission)-1-i))){
      putchar(Permission[i]);
    }
    else{
      putchar('-');
    }
  }
  printf("\ni_mode:      %07o\n",ip->i_mode);
  printf("i_uid:       %d\n",ip->i_uid);
  printf("i_gid:       %d\n",ip->i_gid);
  printf("i_atime:     %ld\n",ip->i_atime);
  printf("i_mtime:     %lu\n",ip->i_mtime);
  printf("i_ctime:     %d\n",ip->i_ctime);
  printf("Link Count:  %d\n",ip->i_links_count);
  printf("device:      %d\n",dev);
  printf("size:        %d\n",ip->i_size);  
}

int my_stat(int ino){
  printf("INO: %d\n",ino);
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  char stbuf[BLKSIZE], sttemp[256];
  char *cp;
  get_block(dev, ip->i_block[0], stbuf);

  cp = stbuf;
  dp = (DIR *)stbuf;

  while(cp < stbuf+1024){
    strncpy(sttemp, dp->name, dp->name_len);
    sttemp[dp->name_len] = 0;
    if(strcmp(sttemp,pathname)==0){
      stat_file(dp->inode,sttemp);
    }
    printf("PathName: %s, Temp: %s\n",pathname,sttemp);
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}
