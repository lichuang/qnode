/*
 * Copyright (C) codedump
 */

#ifndef __QNODE_CORE_ATOMIC_H__
#define __QNODE_CORE_ATOMIC_H__

#include "base/base.h"

inline void *atomic_xchg_ptr (void **ptr_,
                              void *const val_) {
    void *old;
    __asm__ volatile("lock; xchg %0, %2"
                     : "=r"(old), "=m"(*ptr_)
                     : "m"(*ptr_), "0"(val_));
    return old;
}

inline void *atomic_cas (void *volatile *ptr_,
                         void *cmp_,
                         void *val_) {
    void *old;
    __asm__ volatile("lock; cmpxchg %2, %3"
                     : "=a"(old), "=m"(*ptr_)
                     : "r"(val_), "m"(*ptr_), "0"(cmp_)
                     : "cc");
    return old;
}

//  This class encapsulates several atomic operations on pointers.
template <typename T>
class atomic_ptr_t {
public:
  //  Initialise atomic pointer
  inline atomic_ptr_t () { _ptr = NULL; }

  //  Set value of atomic pointer in a non-threadsafe way
  //  Use this function only when you are sure that at most one
  //  thread is accessing the pointer at the moment.
  inline void set (T *ptr_) { _ptr = ptr_; }

  //  Perform atomic 'exchange pointers' operation. Pointer is set
  //  to the 'val_' value. Old value is returned.
  inline T *xchg (T *val_) 
  {
    return (T *) atomic_xchg_ptr ((void **) &_ptr, val_);
  }

  //  Perform atomic 'compare and swap' operation on the pointer.
  //  The pointer is compared to 'cmp' argument and if they are
  //  equal, its value is set to 'val_'. Old value of the pointer
  //  is returned.
  inline T *cas (T *cmp_, T *val_) {
    return (T *) atomic_cas ((void **) &_ptr, cmp_, val_);
  }

private:
  volatile T *_ptr;

  DISALLOW_COPY_AND_ASSIGN(atomic_ptr_t);
};

struct atomic_value_t {
    atomic_value_t (const int value_): _value (value_) {}

    atomic_value_t (const atomic_value_t &src_)
        : _value (src_.load ()) {
    }

    void store (const int value_) {
        atomic_xchg_ptr ((void **) &_value, (void *) (ptrdiff_t) value_);
    }

    int load () const {
        return (int) (ptrdiff_t) atomic_cas ((void **) &_value, 0, 0);
    }

  private:
    volatile ptrdiff_t _value;
  
    DISALLOW_COPY_AND_ASSIGN(atomic_value_t);
};

class atomic_counter_t {
  public:
    typedef uint32_t integer_t;

    inline atomic_counter_t (integer_t value_ = 0)
      : _value (value_) {
    }

    //  Set counter _value (not thread-safe).
    inline void set (integer_t value_) { _value = value_; }

    //  Atomic addition. Returns the old _value.
    inline integer_t add (integer_t increment_) {
        integer_t old_value;

        __asm__ volatile("lock; xadd %0, %1 \n\t"
                         : "=r"(old_value), "=m"(_value)
                         : "0"(increment_), "m"(_value)
                         : "cc", "memory");
        return old_value;
    }

    //  Atomic subtraction. Returns false if the counter drops to zero.
    inline bool sub (integer_t decrement_) {
        integer_t oldval = -decrement_;
        volatile integer_t *val = &_value;
        __asm__ volatile("lock; xaddl %0,%1"
                         : "=r"(oldval), "=m"(*val)
                         : "0"(oldval), "m"(*val)
                         : "cc", "memory");
        return oldval != decrement_;
    }

    inline integer_t get () const { return _value; }

  private:
    volatile integer_t _value;

    DISALLOW_COPY_AND_ASSIGN(atomic_counter_t);
} __attribute__ ((aligned (sizeof (void *))));

#endif // __QNODE_CORE_ATOMIC_H__
