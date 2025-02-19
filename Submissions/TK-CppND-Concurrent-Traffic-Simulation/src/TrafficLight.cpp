#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> uniq_lock(_mutex);
  	_condition.wait(uniq_lock, [this] {return !_queue.empty(); });
  
  	T message = std::move(_queue.back());
  	_queue.pop_back();
  
  	return message;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> uniq_lock(_mutex);
  	_queue.push_back(std::move(msg)); // add msg to queue
  	_condition.notify_one(); // send notification
  
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  	while (true) {
      if (_msg_queue.receive() == TrafficLightPhase::green) {
      return;
      }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    unsigned long cycle_dur; // duration of the cycle
  
  	// random value between 4-6 seconds for the cycle duration.
    // picks out a value between 4000 - 6000 ms, based on the method described in
    // https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
    std::random_device r;
    std::mt19937 gen(r()); //Standard mersenne_twister_engine seeded with r()
    std::uniform_int_distribution<> distrib(4001, 6000);
    cycle_dur = distrib(gen);
  
  	std::chrono::time_point<std::chrono::system_clock> last_update = std::chrono::system_clock::now();
      
    while(true) {
      // wait for 1ms between two cycles.
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
      // compute time difference w.r.t. the last update.
      std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      auto time_diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);

      
      if (time_diff_in_ms.count() >= cycle_dur) {
        // change the traffic light
        if (_currentPhase == TrafficLightPhase::red) {
          _currentPhase = TrafficLightPhase::green;
        }
        else {
          _currentPhase = TrafficLightPhase::red;
        }
        // add to messsage queue
        _msg_queue.send(std::move(_currentPhase));
        
        // reset timer
        last_update = std::chrono::system_clock::now();
 
      }
    } // end while
} // end cycleThroughPhases

