// a1758749, Shahmeer, Chaudhry
// a1754507, Harshit, Kaushal
// a1766686, Kittatam, Saisaard
// tacos
/*
created by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>

// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
/*const*/ int TIME_ALLOWANCE = 32;  // allow to use up to this number of time slots at once
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer

    Event(int par_event_time, int par_customer_id)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
    }
};

void initialize_system(
    std::ifstream &in_file,
    std::deque<Event> &arrival_events,
    std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id);
        arrival_events.push_back(arrival_event);

        customer_id++;
    }
}

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &arrival_events/*,
    const std::deque<int> &customer_queue*/)
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < arrival_events.size(); i++)
    {
        std::cout << "\t" << arrival_events[i].event_time << ", " << arrival_events[i].customer_id << ", ";
    }
    // std::cout << '\n';
    // for (int i = 0; i < customer_queue.size(); i++)
    // {
    //     std::cout << "\t" << customer_queue[i] << ", ";
    // }
    std::cout << '\n';
}

int ExAv(int previous, int current, double alpha){
    // cout << alpha << " * " << current << " + (1 - " << alpha << ") * " << previous << endl;
    return (alpha * current) + ((1 - alpha) * previous);
}

// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> arrival_events; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers);

    // TIME_ALLOWANCE = customers[arrival_events[0].customer_id].slots_remaining;
    std::cout << "TIME_ALLOWANCE: " << TIME_ALLOWANCE << std::endl;

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted
    std::deque<int> queue_regular; // regular waiting queue
    std::deque<int> queue_high; // high waiting queue

    // step by step simulation of each time slot
    bool all_done = false;
    // int previous_guess = customers[arrival_events[0].customer_id].slots_remaining;
    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            // high priority customer
            if (customers[arrival_events[0].customer_id].priority == 0)
            {
                queue_high.push_back(arrival_events[0].customer_id);
            } 
            else // regular priority customer
            {
                queue_regular.push_back(arrival_events[0].customer_id);
            }
            arrival_events.pop_front();
        }

        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    /* customer is not done yet, waiting for the next chance to play
                    in the regular queue
                    */
                    customers[current_id].priority = 1;
                    queue_regular.push_back(current_id);
                }
                current_id = -1; // the machine is free now
            }
        }

        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue_high.empty() || !queue_regular.empty()) // is anyone waiting?
            {
                // ensures that the queue with high priorty all goes first
                if (!queue_high.empty())
                {
                    current_id = queue_high.front();
                    queue_high.pop_front();
                } else {
                    current_id = queue_regular.front();
                    queue_regular.pop_front();
                }
                
                // if the first customer arrives, let them use their entire play time
                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                } else {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            }
        }

        print_state(out_file, current_time, current_id, arrival_events/*, queue*/);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue_regular.empty() && queue_high.empty() && (current_id == -1));
    }

    return 0;
}
