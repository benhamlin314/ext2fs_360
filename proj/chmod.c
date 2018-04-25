#include "globals.h"

char buf[BLKSIZE];
MINODE *mip;
INODE *ip;

int chmod_file(int ino, char new_mode[]){
  //Child Inode
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  //Parse Through Permission In Order To Write Correctly
  char *permission = "rwxrwxrwx";
  long int new_imode = strtol(new_mode, NULL, 8);
  ip->i_mode = (ip->i_mode & 0xF000) | new_imode;
  printf("New Permissions: ");
  for(int i = 0; i < 9; i++){
    if(ip->i_mode & (1 << (strlen(permission)-1-i))){
      putchar(permission[i]);
    }
    else{
      putchar('-');
    }
  }
  printf("\n");
  //Set As Dirty And Write Back
  mip->dirty = 1;
  iput(mip);
}

int my_chmod(char new_mode[]){
  //Create Temp To Tokenize Pathname
  char temp_pathname[64];
  strcpy(temp_pathname,pathname);
  char * chmod_parent = dirname(temp_pathname);
  char * chmod_child = basename(pathname);

  printf("PARENT: %s\tCHILD: %s\n\n\n\n",chmod_parent,chmod_child);
  //Get The Parent INODE
  int inom = getino(dev,chmod_parent);
  mip = iget(dev, inom);
  ip = &(mip->INODE);

  //Load The Data Block[0] Into chmod_buf
  char chmod_buf[BLKSIZE], temp[256];
  char *cp;
  get_block(dev, ip->i_block[0], chmod_buf);

  //Set Up cp and dp
  cp = chmod_buf;
  dp = (DIR *)chmod_buf;

  //Increment Through chmod_buf Looking for child(name)
  while(cp < chmod_buf+1024){
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    if(strcmp(temp,chmod_child)==0){ //If Hit
      chmod_file(dp->inode, new_mode); 
    }
    //Increment
    cp += dp->rec_len;
    dp = (DIR *)cp;
    }
  //Set As Dirty And Write Back
  mip->dirty = 1;
  iput(mip);
}
