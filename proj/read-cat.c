#include "globals.h"

int read_file(){
  //pathname contains the fd number and nbytes to read
  int fd, nbytes;
  fd = atoi(pathname);
  nbytes = atoi(tempPathName);
  char buf[(256+12+256*256)*BLKSIZE];
  if(running->fd[fd] != 0){
    if(running->fd[fd]->mode == 0 || running->fd[fd]->mode == 2){
      return my_read(fd,buf,nbytes);
    }
    else{
      printf("ERROR: File desciptor not open to READ\n");
    }
  }
  else{
    printf("ERROR: File desciptor not open\n");
  }
  return -1;
}

int my_read(int fd, char * buf, int nbytes){
  MINODE *mip, *mip2;
  mip = running->cwd;
  int count = 0, blk;
  int available = running->fd[fd]->mptr->INODE.i_size - running->fd[fd]->offset;
  char  *cp = buf, readbuf[BLKSIZE];
  mip2 = running->fd[fd]->mptr;

  while(nbytes && available){
    int lbk = running->fd[fd]->offset / BLKSIZE;
    int startbyte = running->fd[fd]->offset % BLKSIZE;

    if(lbk < 12){
      blk = mip2->INODE.i_block[lbk];
    }
    else if(lbk >= 12 && lbk < 256 + 12){
      int indirect[256];
      get_block(mip2->dev,mip2->INODE.i_block[12],indirect);
      blk = indirect[lbk-12];
    }
    else{
      int dbl_indirect[256];
      get_block(mip2->dev,mip2->INODE.i_block[13],dbl_indirect);
      for(int j = 1; j<257; j++){
        if(lbk >= 256*j+12 && lbk < 256*(j+1) + 12)
        {
          int block_storage[256];
          get_block(mip2->dev,dbl_indirect[j-1],block_storage);
          blk = block_storage[lbk-(256*j+12)];
          break;
        }
      }
    }
    if(blk == 0){
      return count;
    }
    get_block(mip->dev, blk, readbuf);

    //int optimizer = 64;//change this to alter optimization make it a

    char *store = readbuf + startbyte;
    int remain = BLKSIZE - startbyte;
    /*while(remain > 0){
      if(nbytes-optimizer <= 0 || available-optimizer <= 0){
        break;
      }
      strncpy(buf,store,optimizer);
      running->fd[fd]->offset+=optimizer;
      count += optimizer;
      available -= optimizer;
      nbytes -= optimizer;
      remain -= optimizer;
    }
    //transfers left over when less than 64 nbytes
    while(remain > 0){
      if(nbytes <= 0 || available <= 0){
        break;
      }
      *store++ = *cp++;
      running->fd[fd]->offset++;
      count++;
      available--;
      nbytes--;
      remain--;
    }*/
    while(remain > 0){
      if(nbytes-BLKSIZE > 0){
        printf("BLKSIZE to read\n");
        strncpy(buf, store, BLKSIZE);
        count += BLKSIZE;
        nbytes -= BLKSIZE;
        available -= BLKSIZE;
        remain -= BLKSIZE;
        running->fd[fd]->offset += BLKSIZE;

        if(nbytes <= 0){
          break;
        }
      }
      else{
        printf("less than BLKSIZE to read\n");
        strncpy(buf, store, nbytes);
        count += nbytes;
        available -= nbytes;
        remain -= nbytes;
        running->fd[fd]->offset += nbytes;
        nbytes -= nbytes;
      }
    }
  }
  return count;
}

int cat_file(){
  char buf[BLKSIZE];
  int n, i = 0;

  int fd = open_file(0);
  if(fd > -1){
    while( n = my_read(fd, buf, BLKSIZE)){
      for(i=0;i<n;i++){
        if(buf[i] == '\\'){
          if(buf[i+1] == 'n'){
            putchar('\n');
            i++;
          }
        }else{
          putchar(buf[i]);
        }
      }
    }
    printf("\n");//print new line
    close_file(fd);
  }
  else{
    printf("ERROR: couldn't open file to read\n");
  }
}
