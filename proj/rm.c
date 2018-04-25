#include "globals.h"

int my_rm(){
  MINODE *mip, *pip;
  INODE *ip;
  char buf[BLKSIZE];

  char *Child;
  char *Parent;
  char temp[64];
  strcpy(temp,pathname);
  Parent = dirname(temp);
  strcpy(temp,pathname);
  Child = basename(temp);
  
  int child_ino = getino(dev,pathname);
  if(child_ino == 0){
    printf("Passed Path Cannot Be Found, Cannot Remove\n");
    return -1;
  }
  mip = iget(dev,child_ino);

  int parent_ino = getino(dev,Parent);
  pip = iget(dev,parent_ino);

  ip=&(mip->INODE);
  printf("ip->i_uid: %d\trunning->uid: %d\n",ip->i_uid, running->uid);

  if(ip->i_uid == running->uid || running->uid == 0){
    printf("User Can Remove Object\n");
    if(ip->i_mode == 0x81A4){
      printf("User Can Remove File\n");

      int i = 0;
      for(i = 0; i < 12; i++){//deallocate block loop
	if(ip->i_block[i] != 0){
	  printf("Deallocating block %d\n", i);
	  bdealloc(mip->dev, ip->i_block[i]);
	}
      }
      printf("Deallocate inode\n");
      idealloc(dev, mip->ino);
      mip->dirty = 1;
      iput(mip);
      
      printf("Removing %s from parent's list\n", Child);
      rm_child(pip, Child);
      pip->dirty = 1;
      pip->INODE.i_links_count--;
      pip->INODE.i_atime = pip->INODE.i_mtime = time(0L);
      iput(pip);
    }
  }
}
