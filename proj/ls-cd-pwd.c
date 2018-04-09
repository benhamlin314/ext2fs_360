
#include "globals.h"

char buf[BLKSIZE];
INODE *mip;

int ls_file(int ino){
  mip = iget(dev, ino);

  printf("%07o ",mip->i_mode);
  printf("%d ",mip->i_uid);
  printf("%d ",mip->i_gid);
  printf("%d ",mip->i_atime);
  
}

int ls_dir(int ino){
  mip = iget(dev, ino);

  char lsbuf[BLKSIZE], temp[256];
  char *cp;
  get_block(dev, mip->i_block[0], lsbuf);

  cp = lsbuf;
  dp = (DIR *)lsbuf;

  while(cp < lsbuf+1024){
    ls_file(dp->inode);
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    printf("%s\n", temp);

    cp += dp->rec_len;
    dp = (DIR *)cp;
  }
}

int parse_path(char *path, char *name[256]){
  int nlen = 0;
  char * temp;
  temp = strtok(path, "/");
  name[nlen++]= temp;
  
  while(temp = strtok(0,"/")){//parses path into name array
    name[nlen++] = temp;
  }
  return nlen;
}

int list_file(char *path){
  INODE *tip;
  char *name[256];
  int nlen = 0;
  int cur = 0;
  int ino =0;
  char * temp;
  if(path[0] == '/'){
    tip = iget(dev,2);
  }
  else{
      tip=running->cwd;
  }
  temp = strtok(path, "/");
  name[nlen++]= temp;
  
  while(temp = strtok(0,"/")){//parses path into name array
    name[nlen++] = temp;
  }
  
  if(name[cur] == 0){
    ls_dir(2);
  }
  else{
    //make sure path exists
    printf("Find path....\n");
    for(cur=0;cur<nlen;cur++){
      ino = search(tip,name[cur]);


      if(!ino){
	printf("Path not found at %s\n", name[cur]);
	return -1;
      }
      tip = iget(dev, ino);
      if ((tip->i_mode & 0xF000) != 0x4000){
	printf("ls failed: not a DIR\n");
	return -1;
      }
      printf("Found ino: %d\n", ino);
    }
    printf("OK\n");



    ls_dir(ino);
  }
  
  return 0;
}

int change_dir(char *path){
  INODE *tip;
  char *name[256];
  int nlen = 0;
  int cur = 0;
  char * temp;
  if(path[0] == '/'){
    tip = iget(dev,2);
  }
  else{
    tip = running->cwd;
  }
  temp = strtok(path, "/");
  name[nlen++]= temp;
  
  while(temp = strtok(0,"/")){//parses path into name array
    name[nlen++] = temp;
  }
  int ino = 0;

  
  //make sure path exists
  printf("Find path....\n");
  for(cur=0;cur<nlen;cur++){
    ino = search(tip,name[cur]);
    if(!ino){
      printf("Path not found at %s\n", name[cur]);
      return -1;
    }
    tip = iget(dev, ino);
    if ((tip->i_mode & 0xF000) != 0x4000){
      printf("ls failed: not a DIR\n");
      return -1;
    }
    printf("Found ino: %d\n", ino);
  }
  printf("OK\n");

  running->cwd = tip;
}

int rpwd(INODE *ip){
  get_block(dev,ip->i_block[0], buf);

  char *cp = buf;
  dp = (DIR *)buf;
  cp += dp->rec_len;
  dp = (DIR *)cp;//dp points to ..
  char *tempname;
  tempname = (char *)malloc(sizeof(dp->name) +1);
  strcpy(tempname,dp->name);
  tempname[dp->name_len] = 0;
  int temp = dp->inode;
  mip = iget(dev,dp->inode);
  printf("ino = %d\n", dp->inode);

  get_block(dev,mip->i_block[0], buf);

  cp = buf;
  dp = (DIR *)buf;

  if(dp->inode == 2){
    printf("/");
    free(tempname);
    return 0;
  }
  else{
    rpwd(mip);
    printf("%s/",tempname);
    free(tempname);
  }
}

int pwd(INODE *ip){
  rpwd(ip);
}
