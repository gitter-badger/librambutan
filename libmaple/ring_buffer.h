/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2011 LeafLabs, LLC.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/

/**
 * @file ring_buffer.h
 * @brief Simple circular buffer
 *
 * This implementation is not thread-safe.  In particular, none of
 * these functions are guaranteed re-entrant.
 */

#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#ifdef __cplusplus
extern "C"{
#endif

/**
 * Ring buffer type.
 *
 * The buffer is empty when head == tail.
 *
 * The buffer is full when the head is one byte in front of the tail,
 * modulo buffer length.
 *
 * One byte is left free to distinguish empty from full. */
typedef struct ring_buffer {
    volatile uint8 *buf; /**< Buffer items are stored into */
    uint16 head;         /**< Index of the next item to remove */
    uint16 tail;         /**< Index where the next item will get inserted */
    uint16 size;         /**< Buffer capacity minus one */
} ring_buffer;

/**
 * Initialise a ring buffer.
 *
 *  @param rb   Instance to initialise
 *
 *  @param size Number of items in buf.  The ring buffer will always
 *              leave one element unoccupied, so the maximum number of
 *              elements it can store will be size - 1.  Thus, size
 *              must be at least 2.
 *
 *  @param buf  Buffer to store items into
 */
__attribute__((unused))
static void rb_init(ring_buffer *rb, uint16 size, uint8 *buf) {
    rb->head = 0;
    rb->tail = 0;
    rb->size = size - 1;
    rb->buf = buf;
}

/** Return the number of elements stored in the ring buffer. */
static inline uint16 rb_full_count(ring_buffer *rb) {
    volatile ring_buffer *arb = rb;
    int32 size = arb->tail - arb->head;
    if (arb->tail < arb->head) {
        size += arb->size + 1;
    }
    return (uint16)size;
}

/** Return true if and only if the ring buffer is full. */
static inline int rb_is_full(ring_buffer *rb) {
    return (rb->tail + 1 == rb->head) ||
        (rb->tail == rb->size && rb->head == 0);
}

/** Return true if and only if the ring buffer is empty. */
static inline int rb_is_empty(ring_buffer *rb) {
    return rb->head == rb->tail;
}

/** Append element onto the end of the ring buffer. */
static inline void rb_insert(ring_buffer *rb, uint8 element) {
    rb->buf[rb->tail] = element;
    rb->tail = (rb->tail == rb->size) ? 0 : rb->tail + 1;
}

/** Remove and return the first item from the ring buffer. */
static inline uint8 rb_remove(ring_buffer *rb) {
    uint8 ch = rb->buf[rb->head];
    rb->head = (rb->head == rb->size) ? 0 : rb->head + 1;
    return ch;
}

/**
 * If the ring buffer is nonempty, removes and returns its first item.
 * If it is empty, does nothing and returns a negative value.
 */
static inline int16 rb_safe_remove(ring_buffer *rb) {
    return rb_is_empty(rb) ? -1 : rb_remove(rb);
}

/**
 * If rb is not full, appends element and returns true; otherwise,
 * does nothing and returns false. */
static inline int rb_safe_insert(ring_buffer *rb, uint8 element) {
    if (rb_is_full(rb)) {
        return 0;
    }
    rb_insert(rb, element);
    return 1;
}

/**
 * Append an item onto the end of a non-full ring buffer.  If the
 * buffer is full, removes its first item, then inserts the new
 * element at the end.
 *
 * On success, returns -1.  If an element was popped, returns the
 * popped value. */
static inline int rb_push_insert(ring_buffer *rb, uint8 element) {
    int ret = -1;
    if (rb_is_full(rb)) {
        ret = rb_remove(rb);
    }
    rb_insert(rb, element);
    return ret;
}

/** Discard all items from the buffer */
static inline void rb_reset(ring_buffer *rb) {
    rb->tail = rb->head;
}

#ifdef __cplusplus
} // extern "C"
#endif



#endif

