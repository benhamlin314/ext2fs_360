//This file will enclose open,close,lseek.

#include "globals.h"

int open_file(int mode){
  //1) Ask For A Pathname And Mode To Open
  //2) Get pathnames inumber
  //3) Get Its MINODE Pointer
  int ino = getino(dev,pathname);
  MINODE *mip = iget(dev,ino);
  int i, j;
  //4) Check Inodes i_mode to verify Regular File And Permissions OK
  if(mip->INODE.i_mode == 0x81A4){
    for(i = 0; i < 10; i++){
      if(running->fd[i] != 0){ //There Is File
	if(running->fd[i]->minodePtr->ino == ino){
	  if(running->fd[i]->mode == 1){ //Write Mode
	    printf("Open For Incorrect Mode\n");
	    return -1;
	  }
	  else if(running->fd[i]->mode == 2){ //Read Write Mode
	    printf("Open For Incorrect Mode\n");
	    return -1;
	  }
	  else if(running->fd[i]->mode == 3){ //Append Mode
	    printf("Open For Incorrect Mode\n");
	    return -1;
	  }
	  else if(running->fd[i]->mode == 0 && mode != 0){
	    printf("File Is Open For Read\n");
	    return -1;
	  }
	  else{ //Must Be Read Mode If This Is Reached
	    for(j = 0; j < 10; j++){
	      if(running->fd[j] == 0){
		OFT * oftp = (OFT *)malloc(sizeof(OFT));
		oftp->mode = mode;
		oftp->refCount = 1;
		oftp->minodePtr = mip;
		switch(mode){
		case 0:
		  oftp->offset = 0;
		  running->fd[j]->minodePtr->INODE.i_atime = time(0L);
		  break;
		case 1:
		  truncate(mip);
		  oftp->offset = 0;
		  running->fd[j]->minodePtr->INODE.i_atime = time(0L);
		  running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
		  break;
		case 2:
		  oftp->offset = 0;
		  running->fd[j]->minodePtr->INODE.i_atime = time(0L);
		  running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
		  break;
		case 3:
		  oftp->offset = mip->INODE.i_size;
		  running->fd[j]->minodePtr->INODE.i_atime = time(0L);
		  running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
		  break;
		default:
		  printf("Invalid Mode\n");
		  return -1;
		}
		//Set Running Equal To OFTP and Return The i Value
		running->fd[j] = oftp;
		return j;
	      }
	    }
	  }
	}
      }
      else{ //There is NO File
	for(j = 0; j < 10; j++){
	  if(running->fd[j] == 0){
	    OFT * oftp = (OFT *)malloc(sizeof(OFT));
	    oftp->mode = mode;
	    oftp->refCount = 1;
	    oftp->minodePtr = mip;
	    switch(mode){
	    case 0:
	      oftp->offset = 0;
	      running->fd[j]->minodePtr->INODE.i_atime = time(0L);
	      break;
	    case 1:
	      truncate(mip);
	      oftp->offset = 0;
	      running->fd[j]->minodePtr->INODE.i_atime = time(0L);
	      running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
	      break;
	    case 2:
	      oftp->offset = 0;
	      running->fd[j]->minodePtr->INODE.i_atime = time(0L);
	      running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
	      break;
	    case 3:
	      oftp->offset = mip->INODE.i_size;
	      running->fd[j]->minodePtr->INODE.i_atime = time(0L);
	      running->fd[j]->minodePtr->INODE.i_mtime = time(0L);
	      break;
	    default:
	      printf("Invalid Mode\n");
	      return -1;
	    }
	    //Set Running Equal To OFTP and Return The i Value
	    running->fd[j] = oftp;
	    return j;
	  }
	}
      }
    }
  }
  iput(mip);
}

int truncate(MINODE *mip){
  //Direct
  int i = 0;
  for(i = 0; i < 12; i++){
    if(mip->INODE.i_block[i] != 0){
      bdealloc(dev,mip->INODE.i_block[i]);
    }
  }
  //Indirect
  int j = 0;
  int ino;
  MINODE *temp_mip;
  char buf[BLKSIZE];
  char *cp;
  DIR *dp;
  get_block(dev,mip->INODE.i_block[12],buf);
  cp = buf;
  dp = (DIR *)buf;
  for(j = 0; j < 256; j++){
    ino=getino(dev,dp->inode);
    temp_mip=iget(dev,ino);
      for(int k = 0; k < 12; k++){
	if(temp_mip->INODE.i_block[k] != 0){
	  bdealloc(dev,temp_mip->INODE.i_block[k]);
	}
      }
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
  //Double Indirect
  get_block(dev,mip->INODE.i_block[13],buf);
  for(int l = 0; l < 256; l++){
    get_block(dev,buf[l],newbuf);
    cp = newbuf;
    dp = (DIR *)newbuf;
    for(int m = 0; m < 256; m++){
      ino = getino(dev,dp->inode);
      temp_mip=iget(dev,ino);
      for(int n = 0; n < 12; n++){
	if(temp_mip->INODE.i_block[n] != 0){
	  bdealloc(dev,temp_mip->INODE.i_block[n]);
	}
      }
    cp += dp->rec_len;
    dp = (DIR *)cp;
    }
  }


  mip->INODE.i_size = 0;
  mip->dirty = 1;
  iput(mip);
}

int close_file(int fd){
  if(fd > 9 || fd < 0){
    return -1;
  }
  if(running->fd[fd] != 0){ //Not Null, Pointing At Something
    OFT * oftp = (OFT *)malloc(sizeof(OFT));
    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0){
      return 0;
    }
    
    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE *mip = oftp->minodeptr;
    truncate(mip);
    idealloc(dev,mip->INODE);
    //Do you need to call rm_child?
    mip->dirty=1;
    iput(mip);
    
    return 0; 
  }
}

int lseek(int fd, int position){
  //From fd find the OFT entry
  if(running->fd[fd] != 0){ //File Isnt NULL
    int original_position = 0;
    //Must check to make sure it doesnt overstep the position.

    if(position < 0){ //Under Start Of File
      printf("Cannot be less that offset 0\n");
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = 0; //reset new position to zero
      return original_position; //return original position
    }
    else if(running->fd[fd]->minodeptr->INODE.i_size < position){ //Passed End Of File
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = (running->fd[fd]->minodeptr->INODE.i_size)-1; //reset new position to end of file
      return original_position; //return original position
    }
    else{ //The good position
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = position; //reset new position
      return original_position; //return original position
    }
  }
  return -1; //Error with file
  //Change the OFT offset, do not overstep or understep
  //return Original position
}
