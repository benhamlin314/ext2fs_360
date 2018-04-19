#include "globals.h"

char buf[BLKSIZE];
MINODE *mip;
INODE *ip;


int stat_file(int ino,char temp[]){
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  //Set Up For Printing permissions
  char *permission = "rwxrwxrwx";
  printf("Name:        %s\n",temp);
  printf("Permissions: ");
  for(int i = 0; i < 9; i++){ //{arse Through permissions
    if(ip->i_mode & (1 << (strlen(permission)-1-i))){
      putchar(permission[i]);
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
  //Get MIP Based Off Of Passed ino (No Need For Parent, Doesn't Take Path)
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  //Load In INODEs Data Block[0] Into stat_buf
  char stat_buf[BLKSIZE], stat_temp[256];
  char *cp;
  get_block(dev, ip->i_block[0], stat_buf);

  //Set Up cp and dp
  cp = stat_buf;
  dp = (DIR *)stat_buf;

  while(cp < stat_buf+1024){//Parse Through stat_buf For pathname(name)
    strncpy(stat_temp, dp->name, dp->name_len);
    stat_temp[dp->name_len] = 0;
    if(strcmp(stat_temp,pathname)==0){ //If Hit
      stat_file(dp->inode,stat_temp); //Used For Printing (Pass ino,name)
    }
    //Increment
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}
