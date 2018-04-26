#include "globals.h"

//This File Will House rmdir And All Related Functions.

int rmdir(){
  MINODE *mip, *pip;
  INODE *ip;
  char buf[BLKSIZE];
  int empty=1;

  int ino = getino(dev, pathname);//currently assumes dev
  if(ino == 0){
    printf("Path not found can't remove\n");
    return -1;
  }
  mip = iget(dev, ino);

  char myname[256];

  ip = &(mip->INODE);
  printf("%d %d\n", ip->i_uid, running->uid);
  if(ip->i_uid == running->uid ||  running->uid == 0){//checks to see if user owns dir or if user is super user getuid() is a systemcall
    printf("User can rmdir\n");
    if(ip->i_mode == 0x41ED){
      printf("Refcount is %d\n",mip->refCount);
      if( mip->refCount == 1 ){
        if( ip->i_links_count == 2){//checks if not a dir or busy or not empty
          printf("Might be empty link count is %d\n", ip->i_links_count);
        	printf("Checking if truly empty\n");
        	char *cp;
        	get_block(mip->dev, ip->i_block[0], buf);

        	cp = buf;
        	cp += 12;//points to ..
        	dp = (DIR *)cp;

        	if(dp->rec_len == 12){//dir contains files
        	  empty = 0;
        	}
        	else{
        	  empty = 1;//flag to check empty
        	}
          if(!empty){
            printf("ERROR: Dir not empty\n");
            iput(mip);
            return -1;
          }
        }
        else{
          printf("ERROR: Dir not empty\n");
          iput(mip);
          return -1;
        }
      }
      else{
        printf("ERROR: Dir is busy\n");
        iput(mip);
        return -1;
      }
    }
    else
    {
      printf("ERROR: Not a dir\n");
      iput(mip);
      return -1;
    }
    pip = iget(mip->dev, dp->inode);//dp already points to ..
    getmyname(pip, mip->ino, myname);
    truncate(mip);
    printf("Deallocate inode\n");
    idealloc(mip->dev, mip->ino);
    mip->dirty = 1;
    iput(mip);

    printf("Removing %s from parent's list\n", myname);
    rm_child(pip, myname);
    pip->dirty = 1;
    pip->INODE.i_links_count--;
    pip->INODE.i_atime = pip->INODE.i_mtime = time(0L);
    iput(pip);
  }
}

int rm_child(MINODE * parent, char *name){
  INODE *ip = &(parent->INODE);
  char buf[BLKSIZE], myname[256];
  char *cp, c, *cp2;
  DIR *dpnext, *dpprev;//next dp
  int i = 0, found = 0, location = 0, size = 0;
  for(i = 0; i<12; i++){//loop to find dp containing name
    get_block(parent->dev, ip->i_block[i], buf);
    cp = cp2 = buf;
    dp = (DIR *)cp;
    cp2 += dp->rec_len;
    dpnext = (DIR *)cp2;

    while(cp < buf + BLKSIZE){
      //printf("%d\n", dp->inode);//inode of current thing
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0;
      //printf("%s , %d compare to %s\n",dp->name, dp->inode, name);
      if(strcmp(dp->name,name) == 0){
        printf("FOUND IT\n");
	       found = 1;
      }
      dp->name[dp->name_len] = c;
      printf("%d\n", dp->inode);
      if(found){
	       break;
      }
      dpprev = dp;
      cp += dp->rec_len;
      location += dp->rec_len;
      dp = dpnext;
      cp2 += dpnext->rec_len;

      dpnext = (DIR *)cp2;
    }
    if(found){
      break;
    }
  }
  //printf("%s found in block %d\n",name, i);
  getmyname(parent, dp->inode, myname);
  //printf("%s location %d, dp->rec_len %d\n", myname, location, dp->rec_len);
  size = dp->rec_len;

  if(dp->rec_len == BLKSIZE){//beginning of block
    printf("1\n");
    bdealloc(parent->dev, ip->i_block[i]);
    while(ip->i_block[i+1] != 0){//shifts existing blocks over
      ip->i_block[i] = ip->i_block[i+1];
    }
    ip->i_block[i] = ip->i_block[i+1];
  }
  else if(location + dp->rec_len == BLKSIZE){//end of block
    printf("2\n");
    dpprev->rec_len += dp->rec_len;
    dp->inode = 0;
    dp->name_len = 0;
    dp->rec_len = 0;
    for(int j = 0; j < dp->name_len; j++){
      dp->name[j] = 0;
    }
  }
  else {//somewhere in the middle
    //printf("%d size of removed record\n", size);
    //printf("3\n");
    while(size + cp2 + dp->rec_len < buf + BLKSIZE){//shifts the other dp's to the left so there are no gaps
      cp += dpnext->rec_len;
      c = dpnext->name[dpnext->name_len];
      dpnext->name[dpnext->name_len] = 0;
      printf("Shifting %s to replace  %d \n", dpnext->name, dp->inode);
      dpnext->name[dpnext->name_len] = c;
      dp->inode = dpnext->inode;
      dp->rec_len = dpnext->rec_len;
      dp->name_len = dpnext->name_len;
      strncpy(dp->name, dpnext->name, dpnext->name_len);

      dpprev = dp;
      cp2 += dpnext->rec_len;
      dp = dpnext;
      dpnext = (DIR *)cp2;
    }

    dpprev->rec_len += size;
  }
  put_block(parent->dev,ip->i_block[i],buf);
  parent->dirty = 1;

}
