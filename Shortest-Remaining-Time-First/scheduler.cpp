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
/*const */int TIME_ALLOWANCE = 10;  // allow to use up to this number of time slots at once
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

int time_allowance(std::vector<Customer> &customers)
{
    int sum = 0;
    for (int i = 0; i < customers.size(); i++)
    {
       sum += customers[i].slots_remaining; 
    }

    return sum/customers.size();
}

void reorder(std::vector<Customer> &customers, std::deque<int> &queue, int customer_id, int queue_index){

    std::cout << "queue size: " << queue.size() << ", queue: " << std::endl;
    for (int i = 0; i < queue.size(); i++)
    {
        std::cout << queue[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "customers[queue[" << queue_index << "]].slots_remaining: " << customers[queue[queue_index]].slots_remaining << std::endl;
    std::cout << "customers[" << customer_id << "].slots_remaining: " << customers[customer_id].slots_remaining << std::endl;
    if (customers[queue[queue_index]].slots_remaining >= customers[customer_id].slots_remaining)
    {
        std::cout << customers[queue[queue_index]].slots_remaining << " >= " << customers[customer_id].slots_remaining << std::endl;
        std::cout << "Inserting customer_id: " << customer_id << " at index: " << queue_index << std::endl;
        std::deque<int>::iterator it = queue.begin()+queue_index;
        queue.insert(it, customer_id);
        std::cout << "queue size: " << queue.size() << ", queue: " << std::endl;
        for (int i = 0; i < queue.size(); i++)
        {
            std::cout << queue[i] << " ";
        }
        std::cout << std::endl;
        return;
    }

    if (customers[queue[queue_index]].slots_remaining < customers[customer_id].slots_remaining)
    {
        std::cout << customers[queue[queue_index]].slots_remaining << " < " << customers[customer_id].slots_remaining << std::endl;
        std::cout << "queue_index: " << queue_index << std::endl;
        if (queue_index == queue.size()-1)
        {
            std::cout << "Inserting customer_id: " << customer_id << " at index: " << queue_index << std::endl;
            std::deque<int>::iterator it = queue.end();
            queue.insert(it, customer_id);
            return;
        }
        reorder(customers, queue, customer_id, queue_index+1);
    }
    return;
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
    /*std::cout << '\n';
    for (int i = 0; i < customer_queue.size(); i++)
    {
        std::cout << "\t" << customer_queue[i] << ", ";
    }
    std::cout << '\n';*/
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

    // TIME_ALLOWANCE = time_allowance(customers);

    // std::cout << "The average TIME_ALLOWANCE is: " << TIME_ALLOWANCE << std::endl;

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted
    std::deque<int> queue; // waiting queue

    // step by step simulation of each time slot
    bool all_done = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        // std::cout << "current_time: " << current_time << std::endl;
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            if (customers[current_id].playing_since > 0)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining < 0)
                {
                    customers[current_id].slots_remaining = 0;
                    queue.pop_front();
                }
            }
            std::cout << "New Customer Arrived with customer_id: " << arrival_events[0].customer_id << std::endl;
            std::cout << "queue.size(): " << queue.size() << std::endl;
            if (queue.empty())
            {
                queue.push_back(arrival_events[0].customer_id);
            } else {
                if (customers[current_id].slots_remaining == 0)
                {
                    queue.pop_front();
                }
                reorder(customers, queue, arrival_events[0].customer_id, 0);
                current_id = queue.front();
                time_out = current_time + customers[current_id].slots_remaining;
                customers[current_id].playing_since = current_time;
            }         
            arrival_events.pop_front();
        }
        // check if we need to take a customer off the machine
        // if (current_id >= 0)
        // {
            if (current_id >= 0 && current_time == time_out)
            {
                // std::cout << "Triggered current_id: " << current_id << std::endl;
                // int last_run = current_time - customers[current_id].playing_since;
                // customers[current_id].slots_remaining -= last_run;
                // std::cout << "customers[" << current_id << "].slots_remaining: " << customers[current_id].slots_remaining << std::endl;
                // if (customers[current_id].slots_remaining <= 0)
                // {
                queue.pop_front();
                // }
                current_id = -1; // the machine is free now
            }
        // }
        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue.empty()) // is anyone waiting?
            {
                current_id = queue.front();// queue_regular.pop_front();
                
                // if the first customer arrives, let them use their entire play time
                // if (current_id == 0)
                // {
                //     time_out = current_time + customers[current_id].slots_remaining;
                // } 
                // else 
                // if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                // {
                    time_out = current_time + customers[current_id].slots_remaining;
                // }
                // else
                // {
                //     time_out = current_time + TIME_ALLOWANCE;
                // }
                customers[current_id].playing_since = current_time;
            }
        }
        std::cout << "OUTPUT: " << current_time << " " << current_id << std::endl;
        print_state(out_file, current_time, current_id, arrival_events/*, queue*/);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue.empty() && (current_id == -1));
    }

    return 0;
}
