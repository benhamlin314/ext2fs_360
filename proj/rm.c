#include "globals.h"

int my_rm(){
  MINODE *mip, *pip;
  INODE *ip;
  char buf[BLKSIZE];

  char *Child;
  char *Parent;
  char temp[64];
  char temp2[64];
  strcpy(temp,pathname);
  Parent = dirname(temp);
  strcpy(temp2,pathname);
  Child = basename(temp2);

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
    if(ip->i_mode != 0x41ED){
      printf("User Can Remove File\n");

      truncate(mip);
      printf("Deallocate inode\n");
      idealloc(dev, mip->ino);
      mip->dirty = 1;


      printf("Removing %s from parent's list\n", Child);
      rm_child(pip, Child);
      pip->dirty = 1;
      pip->INODE.i_links_count--;
      pip->INODE.i_atime = pip->INODE.i_mtime = time(0L);

    }
  }
  iput(mip);
  iput(pip);
}
