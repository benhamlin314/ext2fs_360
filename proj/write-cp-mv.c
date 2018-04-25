#include "globals.h"

int write_file(){
  int fd, nbytes;
  fd = atoi(pathname);
  nbytes = strlen(tempPathName);
  if(running->fd[fd] != 0){
    if(running->fd[fd]->mode == 1 || running->fd[fd]->mode == 2){
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

      get_block(mip->dev, blk, writebuf);
      char *cp = writebuf + startbyte;
      int remain = BLKSIZE - startbyte;

      if(remain == BLKSIZE){
        if(running->fd[fd]->offset > ip->i_size){
          ip->i_size += BLKSIZE;
        }
        strncpy(cp, buf, remain);
        nbytes -= BLKSIZE;
        remain -= BLKSIZE;
        tot += BLKSIZE;
        running->fd[fd]->offset += BLKSIZE;
        if(nbytes <= 0){
          break;
        }
      }
      else{
        if(running->fd[fd]->offset > ip->i_size){
          ip->i_size += BLKSIZE;
        }
        strncpy(cp, buf, nbytes);
        nbytes -= nbytes;
        remain -= nbytes;
        tot += nbytes;
        running->fd[fd]->offset += nbytes;
      }

      put_block(mip->dev, blk, writebuf);
  }
  mip->dirty = 1;
  printf("Wrote %d characters to fd=%d\n",tot,fd);
}

int my_cp(char *dest){
  int n = 0;
  char buf[BLKSIZE];
  int fd = open_file(0);//opens pathname for read
  strcpy(pathname, dest);//sets destination for Pathname
  int gd = open_file(1);
  if(fd == -1 && gd == -1){//both failed
    printf("ERROR: copy failed\n");
  }
  else if(fd == -1){//only fd failed
    close_file(gd);
    printf("ERROR: copy failed\n");
    return -1;
  }
  else if(gd == -1){//only gd failed
    close_file(fd);
    printf("ERROR: copy failed\n");
    return -1;
  }
  else{//both succeeded
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
  if(!ino){
    if(my_link(pathname,tempPathName)){
      my_rm();
    }
    else{
      printf("ERROR: move failed\n");
      return -1;
    }
  }
}
