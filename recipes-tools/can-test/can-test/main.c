/**
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "can.h"
  
/**
 */
int main(int argc, char** argv)
{
  psCanHnd pCan;
  pCan = canOpen("can0", 0);
  if(!pCan){
    printf("ERROR: cannot open can\n");
  }
  return 0;
}