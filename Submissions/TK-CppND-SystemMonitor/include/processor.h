#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  Processor();
  float Utilization();  // TODO: See src/processor.cpp
  long prev_idle_jiff;
  long prev_total_jiff;
};

#endif
