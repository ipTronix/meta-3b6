/** ***************************************************************************
 * @file rng.g
 * @brief
 **************************************************************************** */

#include <stdlib.h>
#include <string.h>

#include "rng.h"

/**
 */
psRng rngCreate(int size)
{
  psRng pRng;

  pRng = (psRng)malloc(sizeof(sRng));
  if(!pRng){
    return NULL;
  }
  memset(pRng, 0, sizeof(sRng));

  pRng->item = (int*)malloc(sizeof(int)*size);
  if(!pRng->item){
    free(pRng);
    return NULL;
  }
  pRng->idxi = 0;
  pRng->idxo = 0;
  pRng->size = size;

  return pRng;
}

/**
 */
int rngDestroy(psRng pRng)
{
  free(pRng->item);
  free(pRng);
  return 0;
}

/**
 */
int rngPut(psRng pRng, int item)
{
  if(((pRng->idxi+1)%pRng->size)==pRng->idxo){
    return -1;
  }
  pRng->item[pRng->idxi] = item;
  pRng->idxi++;
  pRng->idxi %= pRng->size;
  return 0;
}

/**
 */
int rngGet(psRng pRng, int* item)
{
  if(pRng->idxo==pRng->idxi){
    return -1;
  }
  *item = pRng->item[pRng->idxo];
  pRng->idxo++;
  pRng->idxo %= pRng->size;
  return 0;
}

/**
 */
int rngClr(psRng pRng)
{
  pRng->idxo = 0;
  pRng->idxi = 0;
  return 0;
}

/**
 */
int rngLoad(psRng pRng)
{
  return (pRng->idxi - pRng->idxo + pRng->size) % pRng->size;
}
