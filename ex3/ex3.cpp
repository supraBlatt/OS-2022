#include "ex3.h"

int read_config(const string& path, config& conf) {
    std::ifstream c_file(path);
    if (!c_file.is_open()) { return -1; } 
    string line;

    // read file line by line
    while (getline(c_file, line)) { 
        if (line.empty()) { continue; }

        // match it against either producer or co-editor
        if (line.compare("PRODUCER") == 0) { 

            // hella naive producer parsing
            vector<size_t> tmp(3);
            for (int i=0; i < 3; i++) {
                if (!getline(c_file, line) || line.empty()) { return -1; }
                std::istringstream tmpstream(line);
                tmpstream >> tmp[i];
            }

            // put the stuff in
            producer_config q = {tmp[0], tmp[1], tmp[2]};
            conf.producer_data.push_back(std::move(q));

        } else if (line.compare("Co-Editor") == 0) { 

            std::istringstream linestream(line);
            linestream >> conf.co_editor_queue_size;
        } 
    }
    return 0;
}

void screen_manage(size_t amount, message_queue& queue) {
    // keep running until all co-editors died
    size_t dead = 0;
    while (dead < amount) {

        // pop a message and print it
        string msg;
        queue.pop(msg);
        if (msg.compare(END) == 0) { dead++; }
        else { std::cout << msg << "\n"; }
    }
    std::cout << END << "\n";
}

void co_edit(message_queue& dispatcher, message_queue& screen_manager) { 
    string msg;
    do { 
        // forward all messages after a short time delay
        dispatcher.pop(msg);
        if (msg.compare(END) != 0) 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        screen_manager.push(string(msg)); // cheat perfect forward
    } while (msg.compare(END) != 0);
}

void consume(const string& message, map<string, ptr_to_queue>& co_editors) {
    
    // get the word describing the type
    string type;
    std::istringstream linestream(message); // assume the message is valid
    for (int i = 0; i < 3; i++) { linestream >> type; }

    // forward the message
    if (co_editors.find(type) != co_editors.end()) {
        // nothing said anything about the format which is passes on using
        // so i just forwarded the message itself. 
        // cheat instead of making a perfect forward
        co_editors[type]->push(string(message));
    }
}

// so sorry you had to read this
void dispatch(vector<ptr_to_queue>& producers, map<string, ptr_to_queue>& co_editors) { 

    // not a good algorithm. round robin and count the dead
    size_t dead = 0; 
    while (dead < producers.size()) {
        dead = 0; 

        // go over all producers
        for (size_t index=0; index < producers.size(); index++) {
            // pop the message out of the queue
            string message;
            producers[index]->pop(message);

            // count a dead producer
            if (message.compare(END) == 0) { producers[index]->push(END); dead++; }
            else { consume(message, co_editors);}
        }
    }
    /* // let all co-producers know you are done */
    for (auto const& pair: co_editors) { (pair.second)->push(END); }
}

void produce(int id, int n, message_queue& queue) { 
    // generate n messages
    int amount = 0; 
    map<string, int> 
    while (amount < n) {

        // try to push the produced message to shared queue
        queue.push(make_message(id, amount));
        amount++;
    }
   
    // send 'end' message
    queue.push(END);
}

string make_message(int id, int amount) { 
    // create the produced message and return it
    std::ostringstream oss;
    /* oss << "producer " << id << " " << types[rand() % types.size()] << " " << amount; */
    return oss.str();
}

// i don't think anything on the code is exception safe.. 
int main(int argc, char* argv[]) {

    // check amount of arguments 
    if (argc != 2 ) { return -1; }

    // vector of threads to wait to 
    vector<std::thread> threads;

    // a vector of unique pointers to queues. 
    // so each queue is movable 
    vector<unique_ptr<message_queue>> producers;

    // initialize config file 
    config c;
    if (read_config(argv[1], c) < 0) { return -1; }

    // does the order in which I make the threads matter? 
    // as the co_editors could already be 'full' when they start? 

    // initialize producers
    for (size_t i = 0; i < c.producer_data.size(); i++) { 

        // make the message queue and transfer ownership to the vector
        producer_config prod = c.producer_data[i];
        auto p = ptr_to_queue(new message_queue(prod.queue_size));
        /* auto p = std::make_unique<message_queue>(prod.queue_size); */
        producers.push_back(std::move(p));
      
        // thread for vector
        std::thread t(produce, prod.id, prod.number_of_products, std::ref(*producers[i]));
        threads.push_back(std::move(t));
    }


    // initialize screen-manager
    /* auto screen_manager = std::make_unique<message_queue>(); */
    auto screen_manager = ptr_to_queue(new message_queue());
    std::thread screen_t(screen_manage, types.size(), std::ref(*screen_manager)); 
    threads.push_back(std::move(screen_t));

    // initialize co-editors
    map<string, ptr_to_queue> co_editors;
    for (size_t i = 0; i < types.size(); i++) {

        // make the message queue and transfer ownership to the vector
        auto p = ptr_to_queue(new message_queue(c.co_editor_queue_size));
        /* auto p = std::make_unique<message_queue>(c.co_editor_queue_size); */
        co_editors.emplace(types[i], std::move(p));
        // thread for vector
        std::thread t(co_edit, std::ref(*co_editors[types[i]]), std::ref(*screen_manager));
        threads.push_back(std::move(t));
    }

    // initialize dispatcher - consumer 
    std::thread dispatch_t(dispatch, std::ref(producers), std::ref(co_editors)); 
    threads.push_back(std::move(dispatch_t));

    // join all threads 
    for (auto& t : threads) {
        t.join();
    }
}
