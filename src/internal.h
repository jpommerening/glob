#ifndef _GLOB_INTERNAL_H_
#define _GLOB_INTERNAL_H_

#define GLOB_ALLOC(mem,n,alloc) mem = _alloc( sizeof(*mem) * n, alloc )
#define GLOB_GROW(mem,n,alloc) mem = _grow( mem, sizeof(*mem) * n, alloc )
#define GLOB_CPY(dest,i,src,j,len) memcpy(&(dest[i]), &(src[j]), len*sizeof(dest[0]))

void* _alloc( size_t size, size_t* alloc );
void* _grow( void* mem, size_t size, size_t* alloc );

#endif
