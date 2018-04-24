//This file will enclose open,close,lseek.

#include "globals.h"

int open_file(int mode){
  //1) Ask For A Pathname And Mode To Open
  //2) Get pathnames inumber
  //3) Get Its MINODE Pointer
  int ino = getino(dev,pathname);
  MINODE *mip = iget(dev,ino);
  int i=0;
  int j=0;
  //4) Check Inodes i_mode to verify Regular File And Permissions OK
  if(mip->INODE.i_mode == 0x81A4){
    printf("Is File\n");
    for(i = 0; i < 10; i++){
      printf("Iteration: %d\n\n",i);
      
      if(running->fd[i] == 0){ //There is NO File
	printf("NO FILE\n");
	for(j = 0; j < 10; j++){
	  printf("Sub Iteration: %d\n",j);
	  if(running->fd[j] == 0){
	    OFT * oftp = (OFT *)malloc(sizeof(OFT));
	    oftp->mode = mode;
	    oftp->refCount = 1;
	    mip->INODE.i_atime = time(0L);
	    oftp->mptr = mip;
	    switch(mode){
	    case 0:
	      printf("Set Mode 0\n");
	      oftp->offset = 0;
	      break;
	    case 1:
	      printf("Set Mode 1\n");
	      truncate(mip);
	      oftp->offset = 0;
	      break;
	    case 2:
	      printf("Set Mode 2\n");
	      oftp->offset = 0;
	      break;
	    case 3:
	      printf("Set Mode 3\n");
	      oftp->offset = mip->INODE.i_size;
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
      else{ //There Is File Check
	printf("Ran into File, Check\n");
	if(running->fd[i]->mptr->ino == ino){
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
		mip->INODE.i_atime = time(0L);
		oftp->mptr = mip;
		switch(mode){
		case 0:
		  printf("Case 0\n");
		  oftp->offset = 0;
		  break;
		case 1:
		  printf("Case 1\n");
		  truncate(mip);
		  oftp->offset = 0;
		  break;
		case 2:
		  printf("Case 2\n");
		  oftp->offset = 0;
		  break;
		case 3:
		  printf("Case 3\n");
		  oftp->offset = mip->INODE.i_size;
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
    }
  }
  iput(mip);
}
    
int my_truncate(MINODE *mip){
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
  //Double Indirect NEED TO FIX THIS PART
  get_block(dev,mip->INODE.i_block[13],buf);
  char newbuf[BLKSIZE];
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
    OFT * oftp = running->fd[fd];
    oftp = running->fd[fd];
    running->fd[fd] = 0;
    oftp->refCount--;
    if (oftp->refCount > 0){
      return 0;
    }
    
    MINODE *mip = oftp->mptr;
    mip->dirty=1;
    iput(mip);
    free(oftp);
    
    return 0; 
  }
}

int my_lseek(int fd, int position){
  //From fd find the OFT entry
  if(running->fd[fd] == 0){
    printf("Didnt Work\n");
    return -1;
  }
  else{//File Isnt NULL
    int original_position = 0;
    //Must check to make sure it doesnt overstep the position.

    if(position < 0){ //Under Start Of File
      printf("Cannot be less that offset 0\n");
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = 0; //reset new position to zero
      printf("Original Position: %d\n",original_position);
      return original_position; //return original position
    }
    else if(running->fd[fd]->mptr->INODE.i_size < position){ //Passed End Of File
      printf("Cannot Be More than size\n");
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = (running->fd[fd]->mptr->INODE.i_size); //reset new position to end of file
      printf("Original Position: %d\n",original_position);
      return original_position; //return original position
    }
    else{ //The good position
      printf("This soup is just right\n");
      original_position=running->fd[fd]->offset; //Grab original position to return
      running->fd[fd]->offset = position; //reset new position
      printf("Original Position: %d\n",original_position);
      return original_position; //return original position
    }
  }
 }

int my_pfd(){
  printf(" fd   mode  offset   INODE\n");
  printf("---- ----- -------- -------\n");
  for(int i = 0; i < 10; i++){
    if(running->fd[i] == 0){
      printf("none  none   none    none \n");
    }
    else{
      printf("  %d      %d     %d     [dev,%d]\n",i,running->fd[i]->mode,running->fd[i]->offset,running->fd[i]->mptr->ino);
    }
  }

}
