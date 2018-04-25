#include "globals.h"

//This File Will House Link and Unlink Functions

<<<<<<< HEAD

my_link_creat(MINODE *pip, char *name, int ino){

  //Do Not Allocate A New ino Number
  printf("CHILD NAME: %s\n",name);
  //Begin Process For Writing INODE into a MINODE[]
  MINODE *mip = iget(dev,ino);
  INODE *ip = &(mip->INODE);
  ip->i_mode = 0x81A4;
  ip->i_uid = running->uid;
  ip->i_gid = running->gid;
  ip->i_size = 0;
  ip->i_links_count = 1;
  ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
  ip->i_blocks = 0;
  enter_name(pip,ino,name);
  mip->dirty = 1;
  iput(mip);

}

=======
>>>>>>> level-2-morgan
int my_link(char oldfile[], char newfile[]){
  //Input Structure: link oldFile newFile

  //Get INODE For oldfile
  int old_ino = getino(dev,oldfile);
  MINODE *old_ip = iget(dev,old_ino);

  if(old_ip->INODE.i_mode == 0x81A4){//Validate The oldfile INODE Is A File
    //Check NewFile Path (Minus BaseName)
    char * parent;
    char * child;
    char temp[64]; //Temp For Tokenizing
    strcpy(temp,newfile);
    parent = dirname(newfile);
    child = basename(temp);
    printf("Parent: %s Child: %s\n\n\n",parent,child);


    //Grab The Parent MINODE (The Parents dirname)
    int parent_ino = getino(dev,parent);
    MINODE *parent_ip = iget(dev,parent_ino);
    printf("Parent ino: %d\n",parent_ino);


    //Add Entry To New Dir
    if(parent_ip->INODE.i_mode == 0x41ED){ //Parent Is A Directory
      //Call my_link_creat to create the new file with the old inumber
      enter_name(parent_ip,old_ino,child);

      parent_ip->dirty = 1;
      parent_ip->INODE.i_atime = time(0L);
    }
    else{ //Parent Isn't A Dir
      printf("Parent Isn't A Directory\n");
      return -1;
    }
    //Mark As Dirty And Increase The Links For The Old File
    old_ip->dirty = 1;
    old_ip->INODE.i_links_count++;
    iput(parent_ip);
  }
  else{ //The oldFile Is Not A File
    printf("Input One Is Not A File\n");
  }

  //Write It Back To Disk
  iput(old_ip);
}



int my_unlink(char oldfile[]){

  //Get INODE For OldFile
  int old_ino = getino(dev,oldfile);
  MINODE *old_ip = iget(dev,old_ino);

  //Create Temp Char* For Parent And Child
  char temp[64];
  strcpy(temp,oldfile);
  char *parent = dirname(temp);
  char *child = basename(oldfile);

  //Validate Type
  if(old_ip->INODE.i_mode == 0x81A4){ //Check If oldfile Is File
    //Remove Links Count, Make It Dirty
    old_ip->INODE.i_links_count--;
    old_ip->dirty = 1;

    if(old_ip->INODE.i_links_count == 0){ //Must Remove Because Links = 0
      //Deallocate All Blocks
      truncate(old_ip);
      //Deallocate Inode Itself
      idealloc(dev, old_ip->ino);
<<<<<<< HEAD

      //Remove The Name From Parent
      int parent_ino = getino(dev,parent);
      MINODE *parent_ip = iget(dev,parent_ino);
      rm_child(parent_ip,child);
      iput(parent_ip);
=======
      
>>>>>>> level-2-morgan
    }
    //Remove The Name From Parent
    int parent_ino = getino(dev,parent);
    MINODE *parent_ip = iget(dev,parent_ino);
    rm_child(parent_ip,child);
    iput(parent_ip);
  }
  else{ //File Isnt File
    printf("Pathname Doesnt Lead To File\n");
    return -1;
  }

  //Write It Back To Disk
  iput(old_ip);
}
