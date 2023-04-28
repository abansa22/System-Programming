#include <stdio.h>

int multiply(int x){
  int m = 1;
  m = m * x;
  return m;
}

int main(){
  int i;
  for (i=1;i<5;i++){
    printf("i is %d, multiply returns %d\n", i, multiply(i));
  }
  return 0;
}
