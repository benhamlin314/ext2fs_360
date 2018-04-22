#include "globals.h"

int read_file(){
  //pathname contains the fd number and nbytes to read
  char *fdstr,*nbytesstr;
  fdstr = strtok(pathname," ");
  nbytesstr = strtok(0," ");
  int fd, nbytes;
  fd = atoi(fdstr);
  nbytes = atoi(nbytesstr);
  char buf[256*256*256];
  if(running->cwd->fd[fd] != 0){
    if(running->cwd->fd[fd]->mode == 0 || running->cwd->fd[fd]->mode == 2){
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
  int available = running->cwd->fd[fd]->mptr->INODE.i_size - running->cwd->fd[fd]->offset;
  char  *cp = buf, readbuf[BLKSIZE];
  mip2 = mip->fd[fd]->mptr;

  while(nbytes && available){
    int lbk = mip->fd[fd]->offset / BLKSIZE;
    int startbyte = mip->fd[fd]->offset % BLKSIZE;

    if(lbk < 12){
      blk = mip2->INODE.i_block[lbk];
    }
    else if(lbk >= 12 && lbk < 256 + 12){
      int indirect[256];
      get_block(mip2->dev,mip2->INODE.i_block[12],indirect);
      blk = indirect[lbk];
    }
    else{
      int dbl_indirect[256];
      get_block(mip2->dev,mip2->INODE.i_block[13],dbl_indirect);
      for(int j = 1; j<257; j++){
        if(lbk >= 256*j+12 && lbk < 256*(j+1) + 12)
        {
          int block_storage[256];
          get_block(mip2->dev,dbl_indirect[j-1],block_storage);
          blk = block_storage[lbk];
          break;
        }
      }
    }

    get_block(mip->dev, blk, readbuf);

    int optimizer = 64;//change this to alter optimization make it a

    char *store = readbuf + startbyte;
    remain = BLKSIZE - startbyte;
    while(remain > 0){
      if(nbytes-optimizer <= 0 || available-optimizer <= 0){
        break;
      }
      strncpy(buf,store,optimizer);
      mip->fd[fd]->offset+=optimizer;
      count += optimizer;
      available -= optimizer;
      nbytes -= optimizer;
      remain -= optimizer;
    }
    //transfers left over when less than 32 nbytes
    while(remain > 0){
      if(nbytes <= 0 || available <= 0){
        break;
      }
      *store++ = *cp++;
      mip->fd[fd]->offset++;
      count++;
      available--;
      nbytes--;
      remain--;
    }
  }
  return count;
}

int cat_file(){
  char buf[BLKSIZE], temp = 0;
  int n, i = 0;

  //int fd = open()

  while( n = my_read(int fd, buf, BLKSIZE)){
    buf[n] = temp;
    for(i=0;i<n;i++){
      if(buf[i] == '\\'){
        if(buf[i+1] == n){
          putchar('\n');
          i++;
        }
      }else{
        putchar(buf[i]);
      }
    }
  }
  close(fd);
}
