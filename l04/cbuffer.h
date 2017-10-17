#ifndef HAVE__CBUFFER_H
#define HAVE__CBUFFER_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

struct Buffer;
typedef struct Buffer Buffer;

Buffer* buffer_new(size_t size);
void buffer_ref(Buffer* buf);
void buffer_unref(Buffer* buf);

int buffer_refs(Buffer* buf);

char* buffer_data(Buffer* buf);
size_t buffer_size(Buffer* buf);

#ifdef __cplusplus
}
#endif

#endif //HAVE__CBUFFER_H
