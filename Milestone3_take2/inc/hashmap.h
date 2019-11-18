#ifndef HASHMAP_H_
#define HASHMAP_H_

struct map *createEmptyMap(int size);

int getHashCode(struct map *m, int key);

void insert(struct map *m, int key, int value);

int lookup(struct map *m,int key);

struct map *initManchesterMap(void);

struct map *initManchesterDecodeMap(void);

#endif /* HASHMAP_H_ */
