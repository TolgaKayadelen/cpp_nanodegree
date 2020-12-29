float MemoryUtilizationHelper(float &memtotal, float &memfree) {
  float memused = memtotal - memfree;
  return memused / memtotal;
}
