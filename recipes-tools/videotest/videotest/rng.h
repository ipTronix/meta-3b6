/** ***************************************************************************
 * @file rng.g
 * @brief
 **************************************************************************** */
#ifndef __RNG_H
#define __RNG_H

/**
 * ring queue definition
 */
#define RNG_SIZE 8
typedef struct {
  int         idxi;
  int         idxo;
  int         size;
  int        *item;
}sRng, *psRng;

psRng rngCreate(int size);
int rngDestroy(psRng pRng);
int rngPut(psRng pRng, int item);
int rngGet(psRng pRng, int* item);
int rngLoad(psRng pRng);
int rngClr(psRng pRng);

#endif /* __RNG_H */
