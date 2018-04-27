#include "globals.h"

int write_file(){
  int fd, nbytes;
  fd = atoi(pathname);
  nbytes = strlen(tempPathName);
  printf("%d is bytes to be written\n",nbytes);
  if(running->fd[fd] != 0){
    if(running->fd[fd]->mode == 1 || running->fd[fd]->mode == 2 || running->fd[fd]->mode == 3){
      return my_write(fd,tempPathName,nbytes);
    }
    else{
      printf("ERROR: File desciptor not open to Write\n");
    }
  }
  else{
    printf("ERROR: File desciptor not open\n");
  }
  return -1;
}

int my_write(int fd, char buf[], int nbytes){
  int lbk, blk, startbyte, tot = 0;
  char writebuf[BLKSIZE];
  MINODE *mip = running->fd[fd]->mptr;
  INODE *ip = &(mip->INODE);
  while(nbytes > 0){
      lbk = running->fd[fd]->offset / BLKSIZE;
      startbyte = running->fd[fd]->offset % BLKSIZE;

      if(lbk < 12){
        if(ip->i_block[lbk] == 0){
          int nblk = balloc(mip->dev);
          if(!nblk){
            printf("ERROR: no more free blocks\n");
            return -1;
          }
          ip->i_block[lbk] = nblk;
        }
        blk = ip->i_block[lbk];
      }
      else if (lbk >= 12 && lbk < 256+12){//indirect
        int indirect[256];
        if(ip->i_block[12] == 0){
          int nblk = balloc(mip->dev);
          if(!nblk){
            printf("ERROR: no more free blocks\n");
            return -1;
          }
          ip->i_block[12] = nblk;
        }
        get_block(mip->dev,ip->i_block[12],indirect);
        blk = indirect[lbk-12];
        if(blk == 0){
          int nblk = balloc(mip->dev);
          if(!nblk){
            printf("ERROR: no more free blocks\n");
            return -1;
          }
          indirect[lbk-12] = nblk;
          blk = indirect[lbk-12];
        }

      }
      else{//double indirect
        int dbl_indirect[256];
        if(ip->i_block[13] == 0){
          int nblk = balloc(mip->dev);
          if(!nblk){
            printf("ERROR: no more free blocks\n");
            return -1;
          }
          ip->i_block[13] = nblk;
        }
        get_block(mip->dev,mip->INODE.i_block[13],dbl_indirect);
        for(int j = 1; j<257; j++){
          if(dbl_indirect[j-1] == 0){
            int blocky = balloc(mip->dev);
            if(!blocky){
              printf("ERROR: no more free blocks\n");
              return -1;
            }
            dbl_indirect[j-1] = blocky;
          }
          if(lbk >= 256*j+12 && lbk < 256*(j+1) + 12)
          {
            int block_storage[256];
            get_block(mip->dev,dbl_indirect[j-1],block_storage);
            blk = block_storage[lbk-(256*j+12)];
            if (blk == 0){
              int nblk = balloc(mip->dev);
              if(!nblk){
                printf("ERROR: no more free blocks\n");
                return -1;
              }
              block_storage[lbk-(256*j+12)] = nblk;
            }
            break;
          }
        }
      }

      printf("Block was gotten\n");
      get_block(mip->dev, blk, writebuf);
      char *cp = writebuf + startbyte;
      int remain = BLKSIZE - startbyte;

      if(nbytes >= BLKSIZE){
        printf("BLKSIZE to write\n");
        running->fd[fd]->offset += BLKSIZE;
        if(running->fd[fd]->offset > ip->i_size){
          ip->i_size += BLKSIZE;
        }
        strncpy(cp, buf, remain);
        tot += BLKSIZE;
        nbytes -= BLKSIZE;
        remain -= BLKSIZE;


        if(nbytes <= 0){
          break;
        }
      }
      else{
        printf("less than BLKSIZE to write\n");
        running->fd[fd]->offset += nbytes;
        if(running->fd[fd]->offset > ip->i_size){
          ip->i_size += nbytes;
        }
        strncpy(cp, buf, nbytes);
        tot += nbytes;
        nbytes -= nbytes;
        remain -= nbytes;


      }

      put_block(mip->dev, blk, writebuf);
  }
  mip->dirty = 1;
  printf("Wrote %d characters to fd=%d\n",tot,fd);
  return tot;
}

int my_cp(char *dest){
  int n = 0;
  char buf[BLKSIZE];
  int fd = open_file(0);//opens pathname for read
  strcpy(pathname, dest);//sets destination for Pathname
  //printf("pathname = %s\n", pathname);
  int gd = open_file(1);
  //printf("fd is %d, gd is %d\n",fd, gd);
  if(fd == -1 && gd == -1){//both failed
    printf("ERROR: copy failed both\n");
    return -1;
  }
  else if(fd == -1){//only fd failed
    close_file(gd);
    printf("ERROR: copy failed\n");
    return -1;
  }
  else if(gd == -1){//only gd failed
    strcpy(pathname, dest);
    //printf("pathname = %s\n", pathname);
    creat_file();
    gd = open_file(1);
    int tot = 0;
    while( n = my_read(fd,buf,BLKSIZE)){
      tot += my_write(gd,buf,n);
    }
    printf("%d is n\n",n);
    close_file(fd);
    close_file(gd);
    printf("Total bytes copied %d\n",tot);
    return tot;
  }
  else{//both succeeded
    printf("Both fd opened\n");
    int tot = 0;
    while( n = my_read(fd,buf,BLKSIZE)){
      tot += my_write(gd,buf,n);
    }
    close_file(fd);
    close_file(gd);
    printf("Total bytes copied %d\n",tot);
    return tot;
  }
}

int my_mv(){
  int ino = getino(dev,pathname);
  if(ino){
    if(my_link(pathname,tempPathName)){
      my_unlink(pathname);
    }
    else{
      printf("ERROR: move failed\n");
      return -1;
    }
  }
}
