#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <chrono>
#include <stdlib.h>
#include <time.h>

/* Implementation of class "MessageQueue" */

/*
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
}
*/

/* Implementation of class "TrafficLight" */

/*
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
}

*/

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setTrafficLightPhase(TrafficLightPhase new_phase)
{
  _currentPhase = new_phase;

}
// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::unique_lock<std::mutex> lck(_mutex);
    srand ( time(NULL) ); //initialize random seed
    const int seconds[3] = {4,5,6};
    auto update = std::chrono::high_resolution_clock::now();
    std::future<void> ftr;
    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      std::chrono::duration<double> time_elapsed = (std::chrono::high_resolution_clock::now() - update);
      // pick a random value between 4 and 6 seconds
      int cycle_duration = seconds[rand() % 3];
      if (time_elapsed.count() >= cycle_duration) {
        if (TrafficLight::getCurrentPhase() == TrafficLightPhase::red) {
          TrafficLight::setTrafficLightPhase(TrafficLightPhase::green);
        }
        else {
          TrafficLight::setTrafficLightPhase(TrafficLightPhase::red);
        }
        // UPDATE THIS
        //ftr = std::async(std::launch::async, &MessageQueue<TrafficLightPhase>::send, _messages, std::move(getCurrentPhase()));
        //ftr.wait();
      } // end if
      update = std::chrono::high_resolution_clock::now();
      srand ( time(NULL) );
    } // end while
} // end cycleThroughPhases
