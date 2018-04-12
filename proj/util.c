/*********** util.c file ****************/
#include "globals.h"

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}   
int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk*BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}

int search(INODE *ip, char *name){
  char *cp2;
  char dirname[256], buf2[BLKSIZE];
  int i = 0;
  
  
  while(ip->i_block[i] > 0){ // assume: DIRs have at most 12 direct blocks
    printf("i_block[%d] = %d\n", i, ip->i_block[i]);
    
    get_block(dev, ip->i_block[i], buf2);
    cp2 = buf2;
    dp = (DIR *)buf2;
    
    while(cp2 < buf2+BLKSIZE){
      strncpy(dirname, dp->name, dp->name_len);
      dirname[dp->name_len] = 0;
      printf("%4d %4d %4d   %s\n", dp->inode, dp->rec_len, dp->name_len,dirname);
      printf("dirname = %s; name = %s\n",dirname,name);
      if(strcmp(name,dirname)==0){//found
	return dp->inode;
      }
      cp2 += dp->rec_len;
      dp = (DIR *)cp2;
    }
    i++;
  }
  return 0;//not found
}


int getmyname(MINODE *parent, int myino, char *myname) 
{
 int i;
 char buf[BLKSIZE], temp[256], *cp;  
 DIR    *dp;
 MINODE *mip = parent;

 //**********  search for a file name ***************
 for (i=0; i<12; i++){ // search direct blocks only
     if (mip->INODE.i_block[i] == 0) 
           return -1;

     get_block(mip->dev, mip->INODE.i_block[i], buf);
     dp = (DIR *)buf;
     cp = buf;

     while (cp < buf + BLKSIZE){
       strncpy(temp, dp->name, dp->name_len);
       temp[dp->name_len] = 0;
       //printf("%s  ", temp);

       if (dp->inode == myino){
           strncpy(myname, dp->name, dp->name_len);
           myname[dp->name_len] = 0;
           return 0;
       }
       cp += dp->rec_len;
       dp = (DIR *)cp;
     }
 }
 return -1;
}


int findino(MINODE *mip, int *myino) 
{
  char buf[BLKSIZE], *cp;   
  DIR *dp;

  get_block(mip->dev, mip->INODE.i_block[0], buf);
  cp = buf; 
  dp = (DIR *)buf;
  *myino = dp->inode;
  cp += dp->rec_len;
  dp = (DIR *)cp;
  return dp->inode;
}
