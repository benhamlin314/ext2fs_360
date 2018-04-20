#include "globals.h"

int my_touch(){

  int ino = getino(dev,pathname);
  if(ino == 0){ //No File Exists
    creat_file(); //Call FileName
  }
  else{ //File Exists Touch It
    MINODE *mip = iget(dev,ino);
    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L); //Touch It
    mip->dirty=1; //Its Dirty From Touching
    iput(mip); //Put it in its place
  }
}
