#inlcude "globals.h"
int make_dir(){
  //Parse Pathname
  //Determine Root Or CWD
  //Parent INODE (pino) = getino(dev,dirname(Pathname));
  //Parent INODE Pointer (pip) = iget(dev,Parent INODE);

  //Verify: That Parent INODE (pino) is a DiR
  //        No Child Exists In Parent Directory


  //After Verification Call mymkdir(Parent Inode Pointer)
  //Increment Parents INODES's Link Count By 1
  //Touch atime And Mark It DIRTY
  
  //Then Call iput And Pass Parent INODE Pointer(pip)
}
int mymkdir(MINODE *pip, char *name){
  //Parent INODE Pointer Points At The Parents MINODE[] Of Dirname "/A/B", The Name Is String Basename "c"
  //Allocate An INODE AND DiskBlock For The New Directoy To Utilize
  //Specifically ino = ialloc(dev), bno = balloc(dev)

  //mip = get(dev,ino); To Load The INODE Into A MINODE[] (This Is So You Can Write To The INODE In Memory)
  mip = iget(dev,ino);
  
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
  for(i = 1; i <= 14; i++){
    ip->i_block[i]=0;                          //Set i_blocks Equal To Zero
  }
  mip->dirty = 1;
  
  //Write The New INODE Out To Disk
  iput(mip);

  //Now Begin The Process For Creating Data Blocks For The New Dir Containing . And ..
  //Write . and .. entries into a buf[] of BLKSIZE (1024)
  

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
