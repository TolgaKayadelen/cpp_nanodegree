// This class defines an element that is stored
// in the garbage collection information list.
//
template <class T>
class PtrDetails
{
  public:
    PtrDetails(T* ptr, int size)
    {
      if (size > 0) {
        isArray = true;
        arraySize = size;
      }
      memPtr = ptr;
      refcount = 1;
    }

    // current reference count
    unsigned refcount;

    // pointer to allocated memory
    T *memPtr;

    /* isArray is true if memPtr points to an allocated array.*/
    bool isArray = false;

    /* If memPtr is pointing to an array, arraySize contains its size */
    unsigned arraySize;
};

// Overloading operator== allows two class objects to be compared.
// This is needed by the STL list class.
template <class T>
bool operator==(const PtrDetails<T> &obj1,
                const PtrDetails<T> &obj2)
{
    return obj1.memPtr == obj2.memPtr;
}
