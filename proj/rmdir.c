#include "globals.h"

//This File Will House rmdir And All Related Functions.

int rmdir(){
  MINODE *mip, *pip;
  INODE *ip;
  char buf[BLKSIZE];
  int empty=1;
	
  int ino = getino(dev, pathname);//currently assumes dev
  mip = iget(dev, ino);
	
  ip = &(mip->INODE);
  printf("%d %d\n", ip->i_uid, running->uid);
  if(ip->i_uid == running->uid || running->uid == 0){//checks to see if user owns dir or if user is super user getuid() is a systemcall
    printf("User can rmdir\n");
    if(ip->i_mode != 0x41ED || ip->i_links_count >= 2){//checks if not a dir or busy or not empty
      printf("Might be empty %d %d\n", ip->i_links_count, mip->dirty);
      if(ip->i_links_count == 2 && ip->i_mode == 0x41ED){
	printf("Checking if truly empty\n");
	char *cp;
	get_block(mip->dev, ip->i_block[0], buf);

	cp = buf;
	cp += 12;
	dp = (DIR *)cp;
				
	if(dp->rec_len > 12){//dir contains files
	  empty = 1;
	}
	else{
	  empty = 0;//flag to check empty
	}
      }
      if(!empty){
	return -1;
      }
    }
    pip = iget(mip->dev, dp->inode);//dp already points to ..	
    int i = 0;
    for(i = 0; i < 12; i++){//deallocate block loop
      if(ip->i_block[i] != 0){
	printf("Deallocating block %d\n", i);
	bdealloc(mip->dev, ip->i_block[i]);
      }
    }
    printf("Deallocate inode\n");	
    idealloc(mip->dev, mip->ino);
    mip->dirty = 1;
    iput(mip);
		
    
    char myname[256];
    getmyname(pip, mip->ino, myname);
    printf("Removing %s from parents list\n", myname);
    rm_child(pip, myname);
  }
}

int rm_child(MINODE * parent, char *name){
  INODE *ip = &(parent->INODE);
  char buf[BLKSIZE], myname[256];
  char *cp, c, *cp2;
  DIR *dpprev;//previous dp
  int i = 0, found = 0, location = 0, size = 0;
  for(i = 0; i<12; i++){//loop to find dp containing name
    get_block(parent->dev, ip->i_block[i], buf);
    cp = cp2= buf;
    dp = (DIR *)cp;
    printf("%d\n", i);
    while(cp2 < buf + BLKSIZE){
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0;
      if(strcmp(dp->name,name) == 0){
	found = 1;
      }
      dp->name[dp->name_len] = c;
      if(found){
	break;
      }
      cp += dpprev->rec_len;
      dpprev = dp;
      location += dp->rec_len;
      cp2 += dp->rec_len;
      dp = (DIR *)cp2;
    }
    if(found){
      break;
    }
  }

  getmyname(parent, dp->inode, myname);
  printf("%s location %d, dp->rec_len %d\n",myname, location, dp->rec_len);
  size = dp->rec_len;
  if(dp->rec_len + 24 == BLKSIZE){//beginning of block
    printf("1\n");
    bdealloc(parent->dev, ip->i_block[i]);
    while(ip->i_block[i+1] != 0){
      ip->i_block[i] = ip->i_block[i+1];
    }
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
    printf("3\n");
    cp += dpprev->rec_len;
    dpprev = dp;//make dpprev the inode you are removing
    cp2 += dp->rec_len;
    dp = (DIR *)cp2;
    int k = 0;
    while(cp2 + dp->rec_len < buf + BLKSIZE){//shifts the other dp's to the left so there are no gaps
      printf("%d\n", k++);
      dpprev->inode = dp->inode;
      dpprev->rec_len = dp->rec_len;
      dpprev->name_len = dp->name_len;
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0;
      strcpy(dpprev->name,dp->name);
      dp->name[dp->name_len] = c;
      cp += dpprev->rec_len;
      dpprev = dp;
      cp2 += dp->rec_len;
      dp = (DIR *)cp2;
    }
    dpprev->rec_len += size;
    dp->inode = 0;
    dp->name_len = 0;
    dp->rec_len = 0;
    for(int j = 0; j < dp->name_len; j++){
      dp->name[j] = 0;
    }	
  }
  put_block(parent->dev,ip->i_block[i],buf);
  parent->dirty = 1;
}
