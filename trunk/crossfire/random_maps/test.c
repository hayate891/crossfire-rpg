#include <stdio.h>
#include <global.h>

/* this is a testing program for layouts.  It's
   included here for convenience only.  */
char **map_gen_spiral(int,int,int);

main() {
  int Xsize, Ysize;
  int i,j,k;
  char **layout;
  SRANDOM(time(0));

  Xsize= RANDOM() %30 + 20;
  Ysize= RANDOM() %30 + 20;
  /* put your layout here */
  layout = map_gen_spiral(Xsize,Ysize,0);

  for(i=0;i<Xsize;i++) {
      for(j=0;j<Ysize;j++) {
        if(layout[i][j]==0) layout[i][j]=' ';
        if(layout[i][j]=='*') layout[i][j]='D';
        printf("%c",layout[i][j]);
      }
      printf("\n");
  }
}
