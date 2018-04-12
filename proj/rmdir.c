#include "globals.h"
#include "type.h"

//This File Will House rmdir And All Related Functions.

int rmdir(char *pathname){
	MINODE *mip, *pip;
	INODE *ip;
	char buf[BLKSIZE];
	
	
	int ino = getino(dev, pathname);//currently assumes dev
	mip = iget(dev, ino);
	
	ip = &(mip->INODE);
	if(ip->i_uid == getuid() || getuid() == 0){//checks to see if user owns dir or if user is super user getuid() is a systemcall
		if(ip->i_mode != 0x41ED || mip->dirty == 1 || ip->i_links_count >= 2){//checks if not a dir or busy or not empty
			if(ip->i_links_count == 2 && ip->i_mode == 0x41ED && mip->dirty == 0){
				char *cp;
				get_block(dev, ip->i_block[0], buf);

				cp = buf+12;
				dp = (DIR *)cp;
				
				if(dp->rec_len > 12){//dir contains files
					empty = 0;
				}
				empty = 1;//flag to check empty
			}
			if(!empty){
				return -1;
			}
		}
		
		int i = 0;
		for(i = 0; i < 12; i++){//deallocate block loop
			if(ip->i_block[i] != 0){
				bdealloc(mip->dev, ip->i_block[i]);
			}
		}
		
		idealloc(mip->dev, mip->ino);
		iput(mip);
		
		pip = iget(mip->dev, dp->inode);//dp already points to ..
		char myname[256];
		getmyname(pip, mip->ino, myname);
		rm_child(pip, myname);
	}
}

int rm_child(MINODE * parent, char *name){
	INODE *ip = &(parent->INODE);
	char buf[BLKSIZE];
	char *cp, c;
	DIR *dpprev;//previous dp
	int i = 0, found = 0;
	for(i = 0; i<12; i++){//loop to find dp containing name
		get_block(parent->dev, ip->i_block[i], buf);
		cp = buf;
		dp = (DIR *)cp;
		while(cp < buf +BLKSIZE){
			c = dp->name[dp->name_len];
			dp->name[dp->name_len] = 0;
			if(strcmp(dp->name,name) == 0){
				found = 1;
			}
			dp->name[dp->name_len] = c;
			if(found){
				break;
			}
			dpprev = dp;
			cp += dp->rec_len;
			dp = (DIR *)cp;
		}
	}
	
	if(dp->rec_len == BLKSIZE){//beginning of block
		bdealloc(parent->dev, ip->i_block[i]);
	}
	else if(c + dp->rec_len == BLKSIZE){//end of block
		dpprev->rec_len += dep->rec_len;
		dp->ino = 0;
		dp->name_len = 0;
		dp->rec_len = 0;
		dp->name = 0;
	}
	else {//somewhere in the middle
		
	}
	put_block(parent->dev,ip->i_block[i],buf);
	parent->dirty = 1;
}