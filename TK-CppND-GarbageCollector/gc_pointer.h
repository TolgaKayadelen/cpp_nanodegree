#include <iostream>
#include <list>
#include <typeinfo>
#include <cstdlib>
#include "gc_details.h"
#include "gc_iterator.h"
/*
    Pointer implements a pointer type that uses
    garbage collection to release unused memory.
    A Pointer must only be used to point to memory
    that was dynamically allocated using new.
    When used to refer to an allocated array,
    specify the array size.
*/
template <class T, int size = 0>
class Pointer{
private:
    // refContainer maintains the garbage collection list.
    static std::list<PtrDetails<T> > refContainer;

    // points to allocated memory to which this Pointer pointer points.
    T *addr;

    // is true if this Pointer points to an allocated array.
    bool isArray;

    // If this Pointer is pointing to an allocated array, arraySize contains its size.
    unsigned arraySize;

    // true when this pointer is first created.
    static bool first;

    // Return an iterator to pointer details in ref_container.
    typename std::list<PtrDetails<T> >::iterator findPtrInfo(T *ptr);

public:
    // Define an iterator type for Pointer<T>.
    typedef Iter<T> GCiterator;

    // Empty constructor
    Pointer(){
        Pointer(NULL);
    }
    Pointer(T*);

    // Copy constructor.
    Pointer(const Pointer &);

    // Destructor for Pointer.
    ~Pointer();

    // Collect garbage. Returns true if at least one object was freed.
    static bool collect();

    // Overload assignment of pointer to Pointer.
    T *operator=(T *t);

    // Overload assignment of Pointer to Pointer.
    Pointer &operator=(Pointer &rv);

    // Return a reference to the object pointed to by this Pointer.
    T &operator*(){
        return *addr;
    }

    // Return the address being pointed to.
    T *operator->() {
      return addr;
    }

    // Return a reference to the object at the index specified by i.
    T &operator[](int i){
       return addr[i];
     }

    // Conversion function to T *.
    operator T *() {
      return addr;
    }

    // Return an Iter to the start of the allocated memory.
    Iter<T> begin(){
        int _size;
        if (isArray) {
            _size = arraySize;
          }
        else {
            _size = 1;
          }
        return Iter<T>(addr, addr, addr + _size);
    }

    // Return an Iter to one past the end of an allocated array.
    Iter<T> end(){
        int _size;
        if (isArray) {
            _size = arraySize;
          }
        else {
            _size = 1;
          }
        return Iter<T>(addr + _size, addr, addr + _size);
    }

    // Return the size of ref_container for this type of Pointer.
    static int ref_container_size() {
       return refContainer.size();
     }

    // A utility function that displays refContainer.
    static void showlist();

    // Clear ref_container when program exits.
    static void shutdown();
};

// STATIC INITIALIZATION
// Creates storage for the static variables
template <class T, int size>
std::list<PtrDetails<T> > Pointer<T, size>::refContainer;
template <class T, int size>
bool Pointer<T, size>::first = true;

// Constructor for both initialized and uninitialized objects.
template<class T,int size>
Pointer<T,size>::Pointer(T *t) {
    // Register shutdown() as an exit function.
    if (first)
        atexit(shutdown);
    first = false;
    auto ptr = findPtrInfo(t);

    // if ptr not in ptr details list, add it.
    if (ptr == refContainer.end()) {
      PtrDetails<T> new_ptr(t, size);
      refContainer.emplace_back(new_ptr);
      if (size > 0) {
        isArray = true;
        arraySize = size;
      } // end if
    } // end if
    // already in the ptr_details list, increment ref_count
    else {
      ptr->refcount++;
    }
    addr = t;
}

// Copy constructor.
template< class T, int size>
Pointer<T,size>::Pointer(const Pointer &ob){
    auto ptr = findPtrInfo(ob.addr);
    ptr->refcount++;
    isArray = ob.isArray;
    arraySize = ob.arraySize;
    addr = ob.addr;
}

// Destructor for Pointer.
template <class T, int size>
Pointer<T, size>::~Pointer(){
  typename std::list<PtrDetails<T> >::iterator ptr;
  ptr = findPtrInfo(addr);
  if (ptr->refcount) {
    ptr->refcount--;
  }
  collect();
}

// Collect garbage. Returns true if at least
// one object was freed.
template <class T, int size>
bool Pointer<T, size>::collect(){
  bool memfreed = false;
  typename std::list<PtrDetails<T> >:: iterator p;
  do {
    // go through the garbage collection list to find unusud items.
    for (p=refContainer.begin(); p != refContainer.end(); p++) {
      if (p->refcount > 0) {
        continue;
      }
      refContainer.remove(*p);
      memfreed = true;
      if (p->memPtr) {
        if (p->isArray) {
          delete[] p->memPtr;
        }
        else {
          delete p->memPtr;
        }
      } // end if (p->memPtr)
      break;
    } // end for
  } while (p != refContainer.end());
  return memfreed;
}

// Overload assignment of pointer to Pointer.
template <class T, int size>
T *Pointer<T, size>::operator=(T *t){
  typename std::list<PtrDetails<T> >::iterator ptr = findPtrInfo(addr);
  ptr->refcount--;
  if(ptr->refcount == 0) {
    collect();
  }
  ptr = findPtrInfo(t);
  // if already in use, increment the refcount.
  if (ptr != refContainer.end()) {
    ptr->refcount++;
  }
  // else, add to the refContainer list.
  else {
    PtrDetails<T> new_ptr(t, size);
    refContainer.emplace_back(new_ptr);
  }
  addr = t;
  return t;

}
// Overload assignment of Pointer to Pointer.
template <class T, int size>
Pointer<T, size> &Pointer<T, size>::operator=(Pointer &rv){
  typename std::list<PtrDetails<T> >::iterator ptr = findPtrInfo(addr);
  // decrement the refcount of the memory block being pointed to.
  ptr->refcount--;
  if(ptr->refcount == 0) {
    collect();
  }
  ptr = findPtrInfo(rv.addr);
  ptr->refcount++;
  addr = rv.addr;
  isArray = rv.isArray;
  arraySize = rv.arraySize;
  return rv;

}

// A utility function that displays refContainer.
template <class T, int size>
void Pointer<T, size>::showlist(){
    typename std::list<PtrDetails<T> >::iterator p;
    std::cout << "refContainer<" << typeid(T).name() << ", " << size << ">:\n";
    std::cout << "memPtr refcount value\n ";
    if (refContainer.begin() == refContainer.end())
    {
        std::cout << " Container is empty!\n\n ";
    }
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        std::cout << "[" << (void *)p->memPtr << "]"
             << " " << p->refcount << " ";
        if (p->memPtr)
            std::cout << " " << *p->memPtr;
        else
            std::cout << "---";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
// Find a pointer in refContainer.
template <class T, int size>
typename std::list<PtrDetails<T> >::iterator
Pointer<T, size>::findPtrInfo(T *ptr){
    typename std::list<PtrDetails<T> >::iterator p;
    // Find ptr in refContainer.
    for (p = refContainer.begin(); p != refContainer.end(); p++)
        if (p->memPtr == ptr)
            return p;
    return p;
}
// Clear refContainer when program exits.
template <class T, int size>
void Pointer<T, size>::shutdown(){
    if (ref_container_size() == 0)
        return; // list is empty
    typename std::list<PtrDetails<T> >::iterator p;
    for (p = refContainer.begin(); p != refContainer.end(); p++)
    {
        // Set all reference counts to zero
        p->refcount = 0;
    }
    collect();
}
