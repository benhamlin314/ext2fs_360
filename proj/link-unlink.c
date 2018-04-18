#include "globals.h"

//This File Will House Link and Unlink Functions

my_link_creat(MINODE *pip, char *name, int ino){

  //Do Not Allocate A New ino Number
  printf("CHILD NAME: %s\n",name);
  //Begin Process For Writing INODE into a MINODE[]
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  ip->i_mode = 0x81A4;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = 0;
  ip->i_links_count = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  enter_name(pip,ino,name);
  mip->dirty = 1;
  iput(mip);
  
}

int my_link(char oldfile[], char newfile[]){
  //link oldFile newFile
  printf("Old: %s\nNew: %s\n",oldfile,newfile);


  //Check OldFile Path NEED TO DO THIS

  
  //Get INODE For OldFile
  int old_pino = getino(dev,oldfile);
  MINODE *old_pip = iget(dev,old_pino);

  //Validate Type
  if(old_pip->INODE.i_mode == 0x81A4){
  
    //Check NewFile Path (Minus Last Entry)
    char * Parent;
    char * Child;
    char tokenstring[64];
    strcpy(tokenstring,newfile);
    Parent = dirname(newfile);
    Child = basename(tokenstring);
    printf("Parent: %s Child: %s\n\n\n",Parent,Child);

    int pino = getino(dev,Parent);
    MINODE *pip = iget(dev,pino);
  
    //Add Entry To New Dir
    if(pip->INODE.i_mode == 0x41ED){ //Parent Is A Directory
      //Make NewFile->ino The Same As OldFile->ino (Done In Function)
      my_link_creat(pip,Child,old_pino);
      pip->dirty = 1;
      pip->INODE.i_atime = time(0L);
      iput(pip);
    }
    else{ //Parent Isn't A Dir
      printf("Parent Isn't A Directory\n");
      return -1;
    }
    old_pip->dirty = 1;
    old_pip->INODE.i_links_count++;
  }
  else{
    printf("Old File Not Directory\n");
  }

  //Write It Back To Disk
  iput(old_pip);
}
