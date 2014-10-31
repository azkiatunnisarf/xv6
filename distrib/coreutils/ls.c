#include <ulibc/ulibc.h>
#include <ulibc/stdio.h>
#include <ulibc/string.h>

#include <xv6/stdio.h>
#include <xv6/dirent.h>
#include <xv6/stat.h>
#include <xv6/fcntl.h>

char*
fmtname(char *path)
{
  static char buf[NAME_MAX+1];
  char *p;
  
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  
  // Return blank-padded name.
  if(strlen(p) >= NAME_MAX)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', NAME_MAX-strlen(p));
  return buf;
}

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  
  if((fd = open(path, 0)) < 0){
    __ulibc_printf(2, "ls: cannot open %s\n", path);
    return;
  }
  
  if(fstat(fd, &st) < 0){
    __ulibc_printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }
  
  switch(st.type){
  case T_FILE:
    __ulibc_printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
    break;
  
  case T_DIR:
    if(strlen(path) + 1 + NAME_MAX + 1 > sizeof buf){
      __ulibc_printf(2, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.d_ino == 0)
        continue;
      memmove(p, de.d_name, NAME_MAX);
      p[NAME_MAX] = 0;
      if(__ulibc_stat(buf, &st) < 0){
        __ulibc_printf(2, "ls: cannot stat %s\n", buf);
        continue;
      }
      __ulibc_printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    sysexit();
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  sysexit();
}
