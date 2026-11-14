#ifndef ___DYN_ARRAY_H_
#define ___DYN_ARRAY_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define typeof(x) __typeof__(x)

#define ArrayForeach(node, array)                                                                                      \
    typeof(*array.data) node;                                                                                          \
    for (unsigned long i = 0; (i < (array.count)) && (node = ((typeof(*array.data) *)array.data)[i], 1); i++)

#define ArrayForeachIndex(i, node, array)                                                                              \
    typeof(*array.data) node = ((typeof(*array.data) *)array.data)[0];                                                 \
    for (unsigned long i = 0; (i < (array.count)) && (node = ((typeof(*array.data) *)array.data)[i], 1); i++)

#define ArrayGrow(type)                                                                                                \
    static inline void type##_array_grow(Array_##type *list) {                                                         \
        if (list->capacity == 0) {                                                                                     \
            list->capacity = sizeof(type);                                                                             \
        }                                                                                                              \
        list->capacity *= 2;                                                                                           \
        list->data = (typeof(list->data))realloc(list->data, list->capacity);                                          \
    }

#define ArrayPush(type)                                                                                                \
    static inline void type##_array_push(Array_##type *list, type elem) {                                              \
        while ((list->count + 1) * sizeof(type) > list->capacity) {                                                    \
            type##_array_grow(list);                                                                                   \
        }                                                                                                              \
        memcpy(&list->data[list->count], &elem, sizeof(type));                                                         \
        list->count += 1;                                                                                              \
    }

#define ArrayPushMultiple(type)                                                                                        \
    static inline void type##_array_push_multiple(Array_##type *list, type *elem, size_t len) {                        \
        while ((list->count + len) * sizeof(type) > list->capacity) {                                                  \
            type##_array_grow(list);                                                                                   \
        }                                                                                                              \
        memcpy(&list->data[list->count], elem, sizeof(type) * len);                                                   \
        list->count += len;                                                                                            \
    }

#define ArrayRemoveLast(type)                                                                                          \
    static inline void type##_array_remove_last(Array_##type *list) { list->count--; }

#define ArrayFree(type)                                                                                                \
    static inline void type##_array_free(Array_##type *array) {                                                        \
        if (array->data)                                                                                               \
            free(array->data);                                                                                         \
        array->count = 0;                                                                                              \
        array->capacity = 0;                                                                                           \
    }

#define ArrayInitializer {.count = 0, .capacity = 0, .data = NULL}

#define Array(type)                                                                                                    \
    typedef struct Array_##type {                                                                                      \
        size_t count;                                                                                                  \
        size_t capacity;                                                                                               \
        type *data;                                                                                                    \
    } Array_##type;                                                                                                    \
    ArrayGrow(type) ArrayPush(type) ArrayFree(type) ArrayRemoveLast(type) ArrayPushMultiple(type)

typedef struct RawArray {
    size_t count;
    size_t capacity;
    void *data;
} RawArray;

static inline void array_grow(RawArray *list) {
    if (list->capacity == 0) {
        list->capacity = 1;
    }
    list->capacity *= 2;
    list->data = realloc(list->data, list->capacity);
}

static inline void array_push(RawArray *list, void *elem, size_t size) {

    while ((list->count + 1) * size > list->capacity) {
        // print("Growing array from %lu to %lu", list->capacity, list->capacity *
        // 2);
        array_grow(list);
    }

    memcpy(&((uint8_t *)list->data)[list->count * size], elem, size);
    list->count += 1;
}

#define NEW_RAW_ARRAY() ((RawArray){.count = 0, .capacity = 0, .data = NULL})

#endif // ___DYN_ARRAY_H_
