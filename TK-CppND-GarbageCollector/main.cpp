#include "gc_pointer.h"
#include "LeakTester.h"

int main()
{
    Pointer<int> p = new int(19);
    p = new int(21);
    p = new int(28);
    p = new int(4);

    // test memory leakage with array pointers.
    Pointer<int, 4> array_ptr = new int[4];
    for (int i=0; i<4; i++) {
      array_ptr[i] = i;
    }
    array_ptr = new int[6];
    for (int i=0; i<6; i++) {
      array_ptr[i] = i;
    }

    return 0;
}
