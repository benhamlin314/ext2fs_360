#include "globals.h"

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
  
  if(pip->INODE.i_mode == 0x41ED){ //Is Dir 
  //*********** add check to see if child already exists
    mymkdir(pip, child);
    //Update pip
    pip->INODE.i_links_count++;
    pip->dirty = 1;
    //Touch a_time ;)
    pip->INODE.i_atime = time(0L);
    iput(pip);
  }
  else{ //Is Not DIR
    printf("Parent Is Not A Directory\n");
    return -1;
  }  
}
int mymkdir(MINODE *pip, char *name){
  
  
  //Allocate An INODE AND DiskBlock For The New Directoy To Utilize
  int ino = ialloc(pip->dev); //Number For INODE 
  int bno = balloc(pip->dev); //Number For BlockNumber

  //mip = get(dev,ino); To Load The INODE Into A MINODE[] (This Is So You Can Write To The INODE In Memory)
  MINODE *mip = iget(pip->dev,ino);
  
  //Overwrite Contents In mip->INODE To Make It A Dir
  INODE *ip = &mip->INODE;
  ip->i_mode = 0x41ED;                         //Or 040755 (DIR Type With Permissions)
  ip->i_uid = running->uid;                    //Owner uid
  ip->i_gid  = running->gid;                   //Group Id
  ip->i_size = BLKSIZE;                        //Size in bytes 
  ip->i_links_count = 2;                       //Links count=2 because of . and ..
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  //Set to current time
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
  char buf[BLKSIZE];
  char *cp = buf;
  DIR *dp = (DIR *)buf;
  
  tempname[0] = '.';
  strcpy(dp->name, tempname);
  dp->rec_len = 12;
  dp->name_len = 1; //Needs To Be 1 Byte
  dp->inode = ino;
  
  cp += dp->rec_len; //Increment cp
	dp = (DIR *) cp;
  
  tempname[1] = '.';
  strcpy(dp->name,tempname);
  dp->rec_len = BLKSIZE-12;
  dp->name_len = 2;
  findino(pip, &dp->inode);

  
  //puts block
  put_block(pip->dev,bno, buf);

  //Enter The Name ENTRY Into The Parent's Directory
  enter_name(pip,ino,name);
}
int enter_name(MINODE *pip, int myino, char *myname){
  char buf[BLKSIZE];
  char * cp;
  DIR * dp;
  INODE * ip = &(pip->INODE);
  int allonew = 0; //flag to see if you need to allocate new block
  int i = 0;	//allocated outside forloop to keep value of next empty block
  for(i = 0; i < 12; i++){
    if(ip->i_block[i]==0){
      allonew = 1;
      break;
    }
    get_block(pip->dev,ip->i_block[i],buf);
  
    //Traverse Through Till The Last Entry In Data Block
    //Get Parents ith Data Block Into A buf[]
    cp = buf;
    dp = (DIR *)buf;
    
    printf("Step To Last Entry In Data Block %d\n",i);
    while(cp + dp->rec_len < buf + BLKSIZE){
      //Increment
      //printf("%d\n", cp);
      cp += dp->rec_len;
      dp = (DIR *)cp;
    }

    printf("At last entry placing %s\n",myname);
    int needed_length = 4*((8+strlen(myname)+3)/4);
    int ideal_length = (4*((8+dp->name_len + 3)/4));
    int remain = dp->rec_len - ideal_length;
	
    if (remain >= needed_length){
      printf("There was room in last block\n");
      dp->rec_len = ideal_length;
      cp += dp->rec_len;
      //increments dp to the empty DIR structure
      dp = (DIR *)cp; 
      dp->inode = myino;
      dp->rec_len = remain;
      dp->name_len = strlen(myname);
      strcpy(dp->name, myname);
      break;
    }
  }
  //END FOR LOOP


  //If You Reach Here There Is No Space In The Existing Data Blocks
  //Allocate a  new data block, Then Increase Parent's size by 1024;
  //Enter The New Entry As The First Entry In The New Data Block With rec_len=BLKSIZE
  if(allonew){//if allonew is 1 then allocate new block
    printf("There was not room in last block for creation");
    int bno = balloc(pip->dev);
    ip->i_size += BLKSIZE;
    ip->i_block[i] = bno;
    get_block(pip->dev, ip->i_block[i], buf);
    cp = buf;
    dp = (DIR *)buf;
    dp->inode = myino;
    dp->rec_len = BLKSIZE;
    dp->name_len = strlen(myname);
    strcpy(dp->name, myname);
  }
  //Write The Data Block To The Disk
  put_block(pip->dev, ip->i_block[i], buf);
}

int creat_file(){
  char *parent;
  char *child;

  //System Calls To Tokenize Pathname                                                                                                                                                                       
  parent = dirname(pathname);
  child = basename(pathname);

  //Get INODE Number                                                                                                                                                                                        
  int pino = getino(dev,parent);
  //Get INODE Itself                                                                                                                                                                                        
  MINODE *pip = iget(dev,pino);
  if(pip->INODE.i_mode == 0x41ED){ //Parent Is Dir                                                                                                                                                                       
    my_creat(pip);
    //Update pip                                                                                                                                                                                            
    //Dont Increment Parent Count
    pip->dirty = 1; //Set To Dirty
    
    //Touch a_time                                                                                                                                                                           
    pip->INODE.i_atime = time(0L);
    iput(pip);
  }
  else{ //Parent Is Not DIR                                                                                                                                                                                
    printf("Parent Is Not A Directory Cannot Place File\n");
    return -1;
  }
}

int my_creat(MINODE *pip, char *name){
  //Allocate An INODE AND DiskBlock For The New Directoy To Utilize                                                                                                                                         
  int ino = ialloc(dev); //Number For INODE                                                                                                                                      

  //mip = get(dev,ino); To Load The INODE Into A MINODE[] (This Is So You Can Write To The INODE In Memory)                                                                                                 
  MINODE *mip = iget(dev,ino);

  //Overwrite Contents In mip->INODE To Make It A Dir                                                                                                                                                        
  INODE *ip = &(mip->INODE);
  ip->i_mode = 0x81A4;                         //Or 040755 (DIR Type With Permissions)                                                 
  ip->i_uid = running->uid;                    //Owner uid                                                                                                                                                 
  ip->i_gid  = running->gid;                   //Group Id                                                                                                                                                  
  ip->i_size = 0;                        //Size in bytes                                                                                                                                             
  ip->i_links_count = 1;                       //Links count=2 because of . and ..                                                                                                                         
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);  //Set to current time                                                                                                                                       
  ip->i_blocks = 0; //Can You Do This?                            // LINUX: Blocks count in 512-byte chunks                                                                                                                   
  //Put In THe Name In Parent Inode Pointer
  enter_name(pip, ino, name);


  mip->dirty = 1;
  //Write The New INODE Out To Disk                                                                                                                                                    
  iput(mip);
}

