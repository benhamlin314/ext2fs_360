/*********** util.c file ****************/

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


/*********** YOU MUST write code for these **********************
int findmyname(MINODE *parent, u32 myino, char *myname) 
{
  // search for myino in parent INODE;
  // copy its name string into myname[ ]
}


int findino(MINODE *mip, u32 *myino) 
{
  // get DIR's ino into myino AND return parent's ino
}
**************************************************************/
