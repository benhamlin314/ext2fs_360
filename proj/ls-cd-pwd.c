
#include "globals.h"

char buf[BLKSIZE];
MINODE *mip;
INODE *ip;

int ls_file(int ino){
  mip = iget(dev, ino);
  ip = &(mip->INODE);
  char *Permission = "rwxrwxrwx";
  printf("Permissions: ");
  for(int i = 0; i < 9; i++){
    if(ip->i_mode & (1 << (strlen(Permission)-1-i))){
      putchar(Permission[i]);
    }
    else{
      putchar('-');
    }
  }
  printf("%07o ",ip->i_mode);
  printf("%d ",ip->i_uid);
  printf("%d ",ip->i_gid);
  printf("%d ",ip->i_atime);

}

int ls_dir(int ino){
  mip = iget(dev, ino);
  ip = &(mip->INODE);

  char lsbuf[BLKSIZE], temp[256];
  char *cp;
  get_block(dev, ip->i_block[0], lsbuf);

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
  /*MINODE *tip;
  INODE *ip;
  char *name[256];
  int nlen = 0;
  int cur = 0;
  int ino =0;
  char * temp;
  if(path[0] == '/'){
    tip = iget(dev,2);
    ip = &(tip->INODE);
  }
  else{
    tip=(MINODE *)running->cwd;
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
      ino = search(ip,name[cur]);


      if(!ino){
	printf("Path not found at %s\n", name[cur]);
	return -1;
      }
      tip = iget(dev, ino);
      ip = &(tip->INODE);
      if ((ip->i_mode & 0xF000) != 0x4000){
	printf("ls failed: not a DIR\n");
	return -1;
      }
      printf("Found ino: %d\n", ino);
    }
    printf("OK\n");
*/
    int ino = 0;
    if(strlen(path)==0){
      ls_dir(running->cwd->ino);
    }
    else{
      ino = getino(dev, path);
      if( ino == 0){
        return 0;
      }
    }

    ls_dir(ino);

  return 0;
}

int change_dir(char *path){
  MINODE *tip;
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
    ino = search(&(tip->INODE),name[cur]);
    if(!ino){
      printf("Path not found at %s\n", name[cur]);
      return -1;
    }
    tip = iget(dev, ino);
    if ((tip->INODE.i_mode & 0xF000) != 0x4000){
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

  //Load Buf
  char *cp = buf;
  dp = (DIR *)buf;

  char charArray[256];

  //Begin Move Process
  int temp = dp->inode;
  cp += dp->rec_len; //Moving To ..
  dp = (DIR *)cp;

  //Grab Parent
  mip = iget(dev,dp->inode);
  ip = &(mip->INODE);
  //Grab Block
  get_block(dev,ip->i_block[0], buf);
  //Load Buf
  cp = buf;
  dp = (DIR *)buf;
  DIR *dp2;
  cp += dp->rec_len;
  dp2 = (DIR *)cp;
  DIR *dp3;
  dp3=dp2;
  while(temp != dp3->inode){
    cp += dp3->rec_len;
    dp3 = (DIR *)cp;
  }

  //Load Name Into charArray[] And Add NULL Char
  strcpy(charArray,dp3->name);
  charArray[dp3->name_len] = 0;

  //Begin Loop Process
  if(dp->inode == dp2->inode){ //Check . Against ..
    printf("/");
    printf("%s/",charArray);
    return 0;
  }
  else{
    rpwd(mip); //Otherwise Recursive Call Memory INODE Pointer
    printf("%s/",charArray);
  }
}

int pwd(INODE *ip){
  rpwd(ip);
}
