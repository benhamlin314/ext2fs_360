
MINODE *my_iget(int dev, int ino)
{
  int i;
  MINODE *mip;
  char buf[BLKSIZE];
  int blk, disp;
  INODE *ip;

  //Search MINODE[] For Entry
  for(i=0; i<NMINODE; i++){
    mip = &minode[i];
    if(mip->dev == dev && mip->ino == ino){
      mip->refCount++;
      printf("Found INODE: %d At MINODE: %d\n",ino,i);
      return mip;
    }
  }

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount == 0){
      printf("Loaded INODE: %d Into MINODE: %d\n",ino,i);
      mip->refCount = 1;
      mip->dev = dev;
      mip->ino = ino;

      //Mailmans Alg.
      blk  = (ino-1)/8 + iblk;
      disp = (ino-1) % 8;

      //Get Block And Load Into Buf Then Set INODE Pointer Accordingly
      get_block(dev, blk, buf);
      ip = (INODE *)buf + disp;
      // copy INODE to mp->INODE
      mip->INODE = *ip;
      return mip;
    }
  }
  printf("Error MINODES[] Is Full\n");
  return 0;
}

my_iput(MINODE *mip)
{
  int i, block, offset;
  char buf[BLKSIZE];
  INODE *ip;

  if(mip==0){ //NULL Case
    return;
  }
  mip->refCount--;

  if(mip->refCount > 0){ //Being Referenced Case
    return;
  }
  if(!mip->dirty){ //Not Changed Case
    return;
  }
 
  /* write back */
  printf("my_iput ino: %d\n",mip->ino);

  //Mailmans Alg.
  block = ((mip->ino-1)/8)+iblk;
  offset = (mip->ino-1)%8;

  //Get Block Containing INODE
  get_block(mip->dev, block, buf);

  //Get The Actual INODE In Buf By Going To The Offest
  ip = (INODE *)buf + offset;
  *ip = mip->INODE;

  //Put Back
  put_block(mip->dev, block, buf);

}

int tokenize(char *pathname)
{
  char *s;
  printf("Tokenize Pathname:  %s\n", pathname);
  strcpy(gpath, pathname);
  n = 0;

  s = strtok(gpath, "/");

  while(s){
    name[n] = s;
    n++;
    s = strtok(0, "/");
  }
}

int my_search(MINODE *mip, char *name)
{
  int i;
  char *cp, c, sbuf[BLKSIZE];
  DIR *dp;
  INODE *ip;

  printf("my_search: Search For %s Inside MINODE[%d]\n", name,mip->ino);
  ip = &(mip->INODE);


  for(i=0; i<12; i++){ /* search direct blocks only */
    printf("my_search: i_block[%d]=%d\n",i, ip->i_block[i]);
    if(ip->i_block[i] == 0)
      return 0;
     
    //Get The i_block Load It Into sbuf For Parsing
    get_block(dev, ip->i_block[i], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    printf("   i_number rec_len name_len    name\n");

    while(cp < sbuf + BLKSIZE){
      c = dp->name[dp->name_len];
      dp->name[dp->name_len] = 0;

      printf("%8d%8d%8u        %s\n",dp->inode, dp->rec_len, dp->name_len, dp->name);
      if(strcmp(dp->name, name)==0){
	printf("Found Name: %s At ino: %d\n", name, dp->inode);
	return(dp->inode);
      }
      //Increment
      dp->name[dp->name_len] = c;
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }
  }
  return(0);
}

int my_getino(int dev, char *pathname)
{
  int i, ino, blk, disp;
  char buf[BLKSIZE];
  INODE *ip;
  MINODE *mip;

  printf("my_getino For Pathname: %s\n", pathname);
  if (strcmp(pathname, "/")==0){ //Just Root
    return 2;
  }
  if (pathname[0]=='/'){ //Absolute
    mip = my_iget(dev, 2);
  }
  else{ //Relative
    mip = my_iget(running->cwd->dev, running->cwd->ino);
  }
  strcpy(buf, pathname);
  tokenize(buf);

  for (i=0; i<n; i++){
    printf("my_getino name:%s\n",name[i]);

    ino = my_search(mip, name[i]);

    if(ino==0){
      my_iput(mip);
      printf("name %s does not exist\n", name[i]);
      return 0;
    }
    my_iput(mip);
    mip = my_iget(dev,ino);
  }
  return ino;
}
