#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// char*
// fmtname(char *path)
// {
//   static char buf[DIRSIZ+1];
//   char *p;

//   // Find first character after last slash.
//   for(p=path+strlen(path); p >= path && *p != '/'; p--)
//     ;
//   p++;

//   // Return blank-padded name.
//   if(strlen(p) >= DIRSIZ)
//     return p;
//   memmove(buf, p, strlen(p));
//   memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
//   return buf;
// }

void
ls(char *path, int cnt_file[], int cnt_dir[], char key)
{
  int max_len = sizeof(path) + 1 + 10 + 1;
  // int max_len = 64;
  char buf[max_len], *p;

  int fd;
  struct dirent de;
  struct stat st;

  int tmp_cnt = 0;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  // if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
  //   printf("ls: path too long\n");
  //   return;
  // }

  strcpy(buf, path);
  p = buf+strlen(buf);
  *p++ = '/';
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;

    // add constrain for '.', '..'
    if ( strcmp(de.name, ".") == 0 )
      continue;
    if ( strcmp(de.name, "..") == 0 )
      continue;

    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if(stat(buf, &st) < 0){
      printf("ls: cannot stat %s\n", buf);
      continue;
    }
    
    // count the key
    tmp_cnt = 0;
    for (int i=0;i<sizeof(buf);i++){
      if (buf[i] == key)
        tmp_cnt ++;
    }
    // printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    printf("%s %d\n", buf, tmp_cnt);

    if(st.type == T_DIR) {
      cnt_dir[0] ++;
      // continue ls
      ls(buf, cnt_file, cnt_dir, key);
    }
    else {
      cnt_file[0] ++;
    }
  
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  // int i;

  // if(argc < 2){
  //   ls(".");
  //   exit(0);
  // }
  // for(i=1; i<argc; i++)
  //   ls(argv[i]);
  // exit(0);

  int cnt_file[1] = {0};
  int cnt_dir[1] = {0};
  int fd_file[2];
  int fd_dir[2];
  int fd;
  int pid;
  int status;
  int tmp_cnt = 0;

  char key = *argv[2];

  if (pipe(fd_file) < 0){
    exit(1);
  }
  if (pipe(fd_dir) < 0){
    exit(1);
  }

  if ( (pid = fork()) < 0 ){
    exit(1);
  }

  // child
  else if (pid == 0){
    if((fd = open(argv[1], 0)) < 0){
      printf("%s [error opening dir]\n", argv[1]);
      exit(0);
    }

    tmp_cnt = 0;
    for (int i=0;i<sizeof(argv[1]);i++){
      if (argv[1][i] == key)
        tmp_cnt ++;
    }

    printf("%s %d\n", argv[1], tmp_cnt);
    ls(argv[1], cnt_file, cnt_dir, key);
    close(fd_file[0]);
    close(fd_dir[0]);
    write(fd_file[1], cnt_file, 1);
    write(fd_dir[1], cnt_dir, 1);
    close(fd_file[1]);
    close(fd_dir[1]);
    exit(0);
  }

  // parent
  else{
    if (wait(&status) != pid){
      exit(1);
    }

    close(fd_file[1]);
    close(fd_dir[1]);
    read(fd_file[0], cnt_file, 1);
    read(fd_dir[0], cnt_dir, 1);
    close(fd_file[0]);
    close(fd_dir[0]);

    printf("\n");
    printf("%d directories, ", cnt_dir[0]);
    printf("%d files\n", cnt_file[0]);

    exit(0);
  }
}
