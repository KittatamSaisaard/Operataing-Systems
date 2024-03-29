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
const int TIME_ALLOWANCE = 100;  // allow to use up to this number of time slots at once
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;
    int response_ratio;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
        response_ratio = -1;
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
    const std::deque<int> &customer_queue)
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
    for (int i = 0; i < customer_queue.size(); i++)
    {
        std::cout << "\t" << customer_queue[i] << ", ";
    }
    std::cout << '\n';
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
    int time_out = 0; // time when current customer will be preempted
    std::deque<int> queue; // waiting queue

    // step by step simulation of each time slot
    bool all_done = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
            // int burst_time = customers[arrival_events[0].customer_id].slots_remaining;
            // int response_ratio = ((time_out - current_time)+burst_time)/burst_time;
            // customers[arrival_events[0].customer_id].response_ratio = response_ratio;
            queue.push_back(arrival_events[0].customer_id);
            arrival_events.pop_front();
        }

        // if multiple customers arrive at the begining at the same time, 
        // put the one with the higest response ratio at the front of the queue
        if (current_time == 0 && queue.size() > 1)
        {
            // update Response Ratio for every customer in the queue
            int burst_time, response_ratio;
            for (int customer = 0; customer < queue.size(); customer++)
            {
                burst_time = customers[queue[customer]].slots_remaining;
                response_ratio = ((time_out - customers[queue[customer]].arrival_time)+burst_time)/burst_time;
                customers[queue[customer]].response_ratio = response_ratio;
            }

            // the customer with the highest Response Ratio will be put infront of the queue
            int max_response_ratio = customers[queue[0]].response_ratio;
            int customer_index = 0;
            for (int customer = 0; customer < queue.size(); customer++){
                if (customers[queue[customer]].response_ratio > max_response_ratio){
                    max_response_ratio = customers[queue[customer]].response_ratio;
                    customer_index = customer;
                }
            }
            // add the customer to the front of the queue
            queue.push_front(queue[customer_index]);
            // remove the customer from the queue
            queue.erase(queue.begin()+customer_index+1);
        }

        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out)
            {
                // update Response Ratio for every customer in the queue
                int burst_time, response_ratio;
                for (int customer = 0; customer < queue.size(); customer++)
                {
                    burst_time = customers[queue[customer]].slots_remaining;
                    response_ratio = ((time_out - customers[queue[customer]].arrival_time)+burst_time)/burst_time;
                    customers[queue[customer]].response_ratio = response_ratio;
                }

                // the customer with the highest Response Ratio will be put infront of the queue
                int max_response_ratio = customers[queue[0]].response_ratio;
                int customer_index = 0;
                for (int customer = 0; customer < queue.size(); customer++){
                    if (customers[queue[customer]].response_ratio > max_response_ratio){
                        max_response_ratio = customers[queue[customer]].response_ratio;
                        customer_index = customer;
                    }
                }
                // add the customer to the front of the queue
                queue.push_front(queue[customer_index]);
                // remove the customer from the queue
                queue.erase(queue.begin()+customer_index+1);

                current_id = -1; // the machine is free now
            }
        }

        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue.empty()) // is anyone waiting?
            {
                current_id = queue.front();
                queue.pop_front();
                time_out = current_time + customers[current_id].slots_remaining;
                customers[current_id].playing_since = current_time;
            }
        }

        print_state(out_file, current_time, current_id, arrival_events, queue);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue.empty() && (current_id == -1));
    }

    return 0;
}
