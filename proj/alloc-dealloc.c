#include "type.h"
#include "globals.h"

//many functions taken from lab6 prework
int tst_bit(char *buf, int bit){
  int i, j;
  i = bit/8; j=bit%8;
  if (buf[i] & (1 << j))
     return 1;
  return 0;
}

int set_bit(char *buf, int bit){
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] |= (1 << j);
}

int clr_bit(char *buf, int bit){
  int i, j;
  i = bit/8; j=bit%8;
  buf[i] &= ~(1 << j);
}

int decFreeInodes(int dev){
  char buf[BLKSIZE];

  // dec free inodes count in SUPER
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count--;
  put_block(dev, 1, buf);

  // dec free inodes count in GD
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count--;
  put_block(dev, 2, buf);
}

int incFreeInodes(int dev){
  char buf[BLKSIZE];

  // inc free inodes count in SUPER
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_inodes_count++;
  put_block(dev, 1, buf);

  // inc free inodes count in GD
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_inodes_count++;
  put_block(dev, 2, buf);
}

int decFreeBlocks(int dev){
  char buf[BLKSIZE];

  // dec free inodes count in SUPER
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count--;
  put_block(dev, 1, buf);

  // dec free inodes count in GD
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count--;
  put_block(dev, 2, buf);
}

int incFreeBlocks(int dev){
  char buf[BLKSIZE];

  // inc free inodes count in SUPER
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;
  sp->s_free_blocks_count++;
  put_block(dev, 1, buf);

  // inc free inodes count in GD
  get_block(dev, 2, buf);
  gp = (GD *)buf;
  gp->bg_free_blocks_count++;
  put_block(dev, 2, buf);
}

int ialloc(int dev){
  int  i;
  char buf[BLKSIZE];

  // read inode_bitmap block
  get_block(dev, imap, buf);

  for (i=0; i < ninodes; i++){
    if (tst_bit(buf, i)==0){
       set_bit(buf,i);
       decFreeInodes(dev);

       put_block(dev, imap, buf);

       return i+1;
    }
  }
  printf("Error in ialloc(): no more free inodes\n");
  return 0;
}

int balloc(int dev){
	int  i;
	char buf[BLKSIZE];

	// read block_bitmap block
	get_block(dev, bmap, buf);

	for (i=0; i < nblocks; i++){
		if (tst_bit(buf, i)==0){
		   set_bit(buf,i);
		   decFreeBlocks(dev);

		   put_block(dev, bmap, buf);

		   return i+1;
		}
	}
	printf("Error in balloc(): no more free inodes\n");
	return 0;
}

int idealloc(int dev, int ino){
	char buf[BLKSIZE];

	// read inode_bitmap block
	get_block(dev, imap, buf);
	clr_bit(buf,ino);
	incFreeInodes(dev)
	
	
}

int bdealloc(int dev, int bno){
	char buf[BLKSIZE];

	// read block_bitmap block
	get_block(dev, bmap, buf);
	clr_bit(buf,bno);
	incFreeBlocks(dev);
	
	
}
