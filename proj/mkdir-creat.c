#inlcude "globals.h"

//Globals
MINODE minode[NMINODE];
MINODE *root;

PROC   proc[NPROC], *running;
MNTABLE mntable, *mntPtr;

SUPER *sp;
GD    *gp;
INODE *ip;

int fd, dev;
int nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[64];

char gpath[128];   // hold tokenized strings
char *name[64];    // token string pointers
int  n;            // number of token strings 

//Functions
int make_dir(){
  char *parent;
  char *child;

  //System Calls To Tokenize Pathname
  parent = dirname(pathname);
  child = basename(pathname);
 
  //Get INODE Number
  int pino = getino(dev,parent);
  //Get INODE Itself
  MINODE *pip = iget(dev,pino);
  if(pip->i_mode == 0x41ED){ //Is Dir 
    mymkdir(pip);
    //Update pip
    pip->i_links_count++;
    pip->dirty = 1;
    //Touch a_time ;)
    pip->i_atime = time(0L);
    iput(pip);
  }
  else{ //Is Not DIR
    printf("Parent Is Not A Directory\n");
    return -1;
  }  
}
int mymkdir(MINODE *pip, char *name){
  
  
  //Allocate An INODE AND DiskBlock For The New Directoy To Utilize
  int ino = ialloc(dev); //Number For INODE 
  int bno = balloc(dev); //Number For BlockNumber

  //mip = get(dev,ino); To Load The INODE Into A MINODE[] (This Is So You Can Write To The INODE In Memory)
  MINODE *mip = iget(dev,ino);
  
  //Overwtie Contents In mip->INODE To Make It A Dir
  INODE *ip = &mip->INODE;
  ip->i_mode = 0x41ED;                         //Or 040755 (DIR Type With Permissions)
  ip->i_uid = running->uid;                    //Owner uid
  ip->i_gid  = running->gid;                   //Group Id
  ip->i_size = BLKSIZE;                        //Size in bytes 
  ip->i_links_count = 2;                       //Links count=2 because of . and ..
  ip->i_atime = i_ctime = i_mtime = time(0L);  //Set to current time
  ip->i_blocks = 2;                            // LINUX: Blocks count in 512-byte chunks 
  ip->i_block[0] = bno;                        // new DIR has one data block   
  for(int i = 1; i <= 14; i++){
    ip->i_block[i]=0;                          //Set i_blocks Equal To Zero
  }
  mip->dirty = 1;
  
  //Write The New INODE Out To Disk
  iput(mip);

  //Now Begin The Process For Creating Data Blocks For The New Dir Containing . And ..
  //Write . and .. entries into a buf[] of BLKSIZE (1024)
  char tempname[EXT2_NAME_LEN];
  char buf[1024];
  char *cp = buf;
  DIR *dp;
  
  tempname[0] = '.';
  strcpy(dp->name, tempname);
  dp->rec_len = 12;
  dp->name_len = 1; //Needs To Be 1 Byte
  dp->inode = ino;
  buf[cp] = *dp; 
  
  char buf2[1024];
  DIR *dp2;
  get_block(dev,pip->i_block[0] ,buf2);
  dp2 = (DIR *)buf2;
  
  cp = dp->rec_len; //Increment cp       
  
  tempname[1] = '.';
  strcpy(dp->name,tempname);
  dp->rec_len = 1012;
  dp->name_len = 2;
  dp->inode = dp2->inode;
  buf[cp] = *dp;
  

  //Enter The Name ENTRY Into The Parent's Directory
  enter_name(pip,ino,name);
}
int enter_name(MINODE *pip, into myino, char *myname){
  int i = 0;
  for(i = 0; i < 12; i++){
    if(i_block[i]==0) 
      BREAK;
  }

  //Get The Parent's Data Block In A buf[]

  //Worth Noting IDEAL_LEN = 4*[(8+name_len+3)/4]
  

  //Traverse Through Till The Last Entry In Data Block
  //Get Parents ith Data Block Into A buf[]
  cp = buf;
  dp = (DIR *)buf;

  printf("Step To Last Entry In Data Block %d\n",blk);
  while(cp + dp->rec_len < buf + BLKSIZE){
    //Print And Set
    c = dp->name[dp->name_len];
    dp->name[dp->name_len] = 0;
    printf("%s ", dp->name);
    dp->name[dp->name_len] = name; //Is This Correct?


    //Increment
    cp += dp->rec_len;
    dp = (DIR *)cp;
  }


  //If You Reach Here There Is No Space In The Existing Data Blocks
  //Allocate a  new data block, Then Increase Parent's isze by 1024;
  //Enter The New Entry As The First Entry In The New Data Block With rec_len=BLKSIZE

  //Write The Data Block To The Disk
  
}
int creat_file(){

}
int my_creat(MINODE *pip, char *name){

}
