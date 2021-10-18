#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

mutex M;
condition_variable CV;
string NEXT;

vector<pair<string, string>> makeQuotes(string name) {
    vector<pair<string, string>> quotes;
    if (name == "Jerry") {
        quotes.push_back(make_pair("Hi Marla", "Marla"));
        quotes.push_back(make_pair("George, Marla", "George"));
        quotes.push_back(make_pair("Stacy", "Stacy"));
        quotes.push_back(make_pair("George Stacy", "George"));
        quotes.push_back(make_pair("George", "George"));
    }
    if (name == "George") {
        quotes.push_back(make_pair("Marla", "Marla"));
        quotes.push_back(make_pair("Stacy", "Stacy"));
        quotes.push_back(make_pair("Jerry Marla Stacy", "END"));
    }
    if (name == "Marla") {
        quotes.push_back(make_pair("Jerry", "Jerry"));
        quotes.push_back(make_pair("George. Jerry, Stacy", "Jerry"));        
    }
    if (name == "Stacy") {
        quotes.push_back(make_pair("Jerry", "Jerry"));
        quotes.push_back(make_pair("George", "Jerry"));
    }
    return quotes;
}

class Person {    
    vector<pair<string, string>> quotes;
    int idx {0};
    string name {"default"};
public:
    Person(string _name, vector<pair<string, string>> _quotes) : name(_name), quotes(_quotes) {}
    Person() {}
    void speak() {
        unique_lock<mutex> lck {M};
        cout << name << " is here" << endl;
        while (true) {
            CV.wait(lck, [&](){ return (NEXT == this->name); } ); // wakeup only if I am next
            if (idx < quotes.size()) {
                cout << name << " -> " << quotes[idx].first << endl;
                // notify the next person
                NEXT = quotes[idx].second;
                idx++;
                CV.notify_all();
            }
        }
    }
};

class Conversation {
private:
    vector<Person*> friends;    
    vector<thread> threads;
public:    
    Conversation(vector<Person*> _friends) : friends(_friends) {}
    void start(string starter) {        
        for (auto person : friends) {
            thread t(&Person::speak, person);
            threads.push_back(move(t));
        }
        
        system("pause");
        NEXT = starter;
        CV.notify_all();
    }

    void end()  {
        for (auto& t: threads) {
            t.join();
        }
    }
};

int main() {
    Person* jerry = new Person("Jerry", makeQuotes("Jerry"));
    Person* george = new Person("George", makeQuotes("George"));
    Person* marla = new Person("Marla", makeQuotes("Marla"));
    Person* stacy = new Person("Stacy", makeQuotes("Stacy"));

    Conversation introduction { {jerry, george, marla, stacy} };
    
    introduction.start("Jerry");
    introduction.end();
}
