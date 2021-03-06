#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.front());
    _queue.pop_front();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::lock_guard<std::mutex> lck(_mutex);

    _queue.push_back(std::move(msg)); // push_back? emplace_back?
    _condition.notify_one();
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
        // sleep every start of while loop to eliminate CPU burning up
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // calls the receive function on the message queue
        TrafficLightPhase msg = _messages.receive(); 

        if (msg == TrafficLightPhase::green) {
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(&TrafficLight::cycleThroughPhases, this);
    
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

     // random number between 4000 and 6000
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(4000, 6000); // define the range
    double cycle = distr(eng);

    auto time_before = std::chrono::system_clock::now();

    while (true) {
        // sleep every start of while loop to eliminate CPU burning up
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto time_now = std::chrono::system_clock::now();
        auto time_difference = time_now - time_before;
        auto time = std::chrono::duration_cast<std::chrono::milliseconds>(time_difference);

        if (time.count() >= cycle) {
            if (_currentPhase == TrafficLightPhase::red) {
                _currentPhase = TrafficLightPhase::green;
            } else {
                _currentPhase = TrafficLightPhase::red;
            }
            // send an update to message queue
            _messages.send(std::move(_currentPhase));
            time_before = std::chrono::system_clock::now();
        }
    }
}
