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
const int REGULAR_TIME_ALLOWANCE = 10;
const int HIGH_TIME_ALLOWANCE = 20;
const int PRINT_LOG = 0; // print detailed execution trace
int TIME_ALLOWANCE = HIGH_TIME_ALLOWANCE;  // allow the customer to use up to this number of time slots at once


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
    const std::deque<Event> &arrival_events,
    const std::deque<int> &regular_customer_queue,
    const std::deque<int> &high_customer_queue)
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
    std::cout << '\n';
    for (int i = 0; i < regular_customer_queue.size(); i++)
    {
        std::cout << "\t" << regular_customer_queue[i] << ", ";
    }
    std::cout << '\n';
    for (int i = 0; i < high_customer_queue.size(); i++)
    {
        std::cout << "\t" << high_customer_queue[i] << ", ";
    }
    std::cout << '\n';
}

// Shortest-Job-First Scheduling Algorithm
// Orders the queue in ascending order by the customer's remaining slot time
void SJF(std::vector<Customer> &customers, std::deque<int> &queue, int customer_id, int queue_index){

    if (queue.empty())
    {
        queue.push_back(customer_id);
        return;
    }

    if (customers[queue[queue_index]].slots_remaining >= customers[customer_id].slots_remaining)
    {
        std::deque<int>::iterator it = queue.begin()+queue_index;
        queue.insert(it, customer_id);
        return;
    }

    if (customers[queue[queue_index]].slots_remaining < customers[customer_id].slots_remaining)
    {
        if (queue_index == queue.size()-1)
        {
            std::deque<int>::iterator it = queue.end();
            queue.insert(it, customer_id);
            return;
        }
        SJF(customers, queue, customer_id, queue_index+1);
    }
    return;
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

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted
    std::deque<int> queue_regular; // regular priority waiting queue
    std::deque<int> queue_high; // high priority waiting queue
    std::deque<int> queue_same; // customers that arrive at the same time will be placed in this queue

    // step by step simulation of each time slot
    bool all_done = false;
    int same_arrival_count, new_arrival_between_same_arrival_playing = 0;
    for (int current_time = 0; !all_done; current_time++)
    {
        // check if we need to take a customer off the machine
        if (current_id >= 0 && current_time == time_out)
        {
            int last_run = current_time - customers[current_id].playing_since;
            customers[current_id].slots_remaining -= last_run;
            if (customers[current_id].slots_remaining > 0)
            {
                if (customers[current_id].priority == 0)
                {
                    SJF(customers, queue_high, current_id, 0);
                } else {
                    SJF(customers, queue_regular, current_id, 0);
                }
            }
             current_id = -1; // the machine is free now
        }

        int same_arrival_time = -1;
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            if (current_id != -1 && arrival_events[0].customer_id > 0)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    if (customers[current_id].priority == 0)
                    {
                        if (new_arrival_between_same_arrival_playing > 0)
                        {
                            queue_high.push_back(current_id);
                        } else {
                            SJF(customers, queue_high, current_id, 0); 
                        }
                    } else {
                       if (new_arrival_between_same_arrival_playing > 0)
                        {
                            queue_regular.push_back(current_id);
                        } else {
                            SJF(customers, queue_regular, current_id, 0); 
                        }
                    }
                }
                // set machine to empty and reset timeout
                current_id = time_out = -1;
            }

            if (same_arrival_time == current_time)
            {
                queue_same.push_back(arrival_events[0].customer_id);
            } else if (arrival_events.size() > 1 && arrival_events[0].event_time == arrival_events[1].event_time)
            {
                queue_same.push_back(arrival_events[0].customer_id);
                same_arrival_time = current_time;
            } else {
                if (!queue_same.empty())
                {
                    new_arrival_between_same_arrival_playing++;
                }
                queue_high.push_front(arrival_events[0].customer_id);
            }
            arrival_events.pop_front();
        }

        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue_same.empty() || !queue_high.empty() || !queue_regular.empty()) // is anyone waiting?
            {
                // ensures that customers that arrive at the same time all go first and then 
                // the customers in the high queue all use the machine first before the regular queue
                if (!queue_same.empty())
                {
                    current_id = queue_same.front();
                    queue_same.pop_front();
                    TIME_ALLOWANCE = 1;
                    if (new_arrival_between_same_arrival_playing > 0 && queue_same.empty())
                    {
                        for (int i = 0; i < new_arrival_between_same_arrival_playing; i++)
                        {
                           int temp_customer = queue_high.front();
                           queue_high.pop_front();
                           queue_same.push_back(temp_customer);
                        }
                        new_arrival_between_same_arrival_playing = 0;
                    }
                } else if (!queue_high.empty())
                {
                    current_id = queue_high.front();
                    queue_high.pop_front();
                    TIME_ALLOWANCE = HIGH_TIME_ALLOWANCE;
                } else {
                    current_id = queue_regular.front();
                    queue_regular.pop_front();
                    TIME_ALLOWANCE = REGULAR_TIME_ALLOWANCE;
                }

                // allocate playing time to the customer
                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                } else {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                    
                customers[current_id].playing_since = current_time;
            }
        }
        print_state(out_file, current_time, current_id, arrival_events, queue_regular, queue_high);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue_regular.empty() && queue_high.empty() && queue_same.empty() && (current_id == -1));
    }

    return 0;
}
