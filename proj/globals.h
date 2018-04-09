extern MINODE minode[NMINODE];
extern MINODE *root;

extern PROC   proc[NPROC], *running;
extern MNTABLE mntable, *mntPtr;

extern SUPER *sp;
extern GD    *gp;
extern INODE *ip;
extern DIR *dp, *mdp;

extern int fd, dev;
extern int nblocks, ninodes, bmap, imap, iblk;
extern char line[128], cmd[32], pathname[64];

extern char gpath[128];   // hold tokenized strings
extern char *name[64];    // token string pointers
extern int  n;            // number of token strings 
