#include "thready_queue.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <map>
#include <fstream> 
#include <algorithm> 

using std::vector;
using std::string;
using std::unique_ptr;
using std::map;

using message_queue = thready_queue<string>;
using ptr_to_queue = unique_ptr<message_queue>; 

// types of news 
const vector<string> types {"NEWS", "SPORTS", "WEATHER"};
#define END "DONE" // magic message
#define CONF_PATH "config.txt"

// naive structs to keep config data
struct producer_config {
    size_t id;
    size_t number_of_products;
    size_t queue_size;
};

struct config {
    vector<producer_config> producer_data;
    size_t co_editor_queue_size;    
};

// ultra naive way of reading and parsing the config
// assumes file correcness. nothing in the assignment said otherwise.
// the format I support is 
// ----
// PRODUCER 
// [id]
// [num of products]
// [queue_size]
//
// .
// .
//
// Co-Editor 
// [queue size]
void read_config(const string& path); 

// produce a string :  "Producer [id] [random element from types] [amount]"
string make_message(int id, int amount); 

// Generates messages and throws them into the queue.
// Produces [n] messages using [produce]
// Has an id [id] and can produce messages from types [types]
void produce(int id, int n, message_queue& queue);

// Dispatches the producers messages to coeditors
// Round robins over the producers vector and pops messages
// Sorts the messages based on Type and sends them to the corresponding co-editor
void dispatch(vector<ptr_to_queue>& producers, map<string, ptr_to_queue>& co_editors);

// Takes in a news message, sorts it based on type
// and sends it to the respective queue 
void consume(const string& message, map<string, ptr_to_queue>& co_editors);

// forwards sorted messages to screen manager
void co_edit(message_queue& dispatcher, message_queue& screen_manager);

// pops messages from queue sent by co_editors and prints them
void screen_manage(size_t amount, message_queue& queue);

