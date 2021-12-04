// Example program
#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <deque>
#include <vector>
#include <condition_variable>
using namespace std::chrono;
using namespace std;

const unsigned int NUM_CHAIRS = 3;
const unsigned int NUM_CUSTOMERS = 4;

// In a barbershop, customers are coming in and there is only one barber. If the barber is cutting a
// customers hair, the customer will wait. There are only a limited amount of chairs for the customer
// to wait on. If the chairs are full, then the customer will leave and try again later.
    
struct Barbershop
{
    public:
        Barbershop(): num_waiting(0), has_customers(true){};
        condition_variable cv_barber; // Acts like a receptionist
        condition_variable cv_customer;
        atomic<int> num_waiting; // Number of customers waiting on chairs
        mutex cout_mtx;
        atomic<bool> has_customers;

};


class Barber
{
    public:
        Barber(Barbershop& shop): mem_shop(shop), hislife(&Barber::work, this)
        {
            mem_shop.cout_mtx.lock();
            cout << "Barber has been born!" << endl;    
            mem_shop.cout_mtx.unlock();
        };
        void work()
        {
            while(mem_shop.has_customers){
            // cout << "Dan initialised" << endl;
            unique_lock<mutex> lk(b_mu);
            if(mem_shop.num_waiting == 0)
            {
                mem_shop.cout_mtx.lock();
                cout << "Barber sleeping...\n";
                mem_shop.cout_mtx.unlock();
            }
            mem_shop.cv_barber.wait(lk, [this]{return mem_shop.num_waiting > 0;}); // []{return mem_shop.num_waiting}
            mem_shop.cout_mtx.lock();
            auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            cout << "Time to cut some hair! : at " << millisec_since_epoch << endl;
            mem_shop.cout_mtx.unlock();
            lk.unlock();
            mem_shop.cv_customer.notify_one();
            this_thread::sleep_for(chrono::milliseconds(100));
            mem_shop.num_waiting--;
            }
        };

        ~Barber()
        {
            hislife.join();
        }
    private:
        int jn;
        mutex b_mu;
        Barbershop& mem_shop;
        thread hislife;
        
};

class Customer
{
    public:
        Customer(Barbershop& shop, int name_id): mem_shop(shop), name(name_id), hislife(&Customer::tryGetHaircut, this)
        {
            // this_thread::sleep_for(chrono::milliseconds(1000));
            mem_shop.cout_mtx.lock();
            cout << "Customer " << name_id << " has been born!" << endl;    
            mem_shop.cout_mtx.unlock();
        };
        void tryGetHaircut()
        {
            
            if(mem_shop.num_waiting == NUM_CHAIRS)
            {
                mem_shop.cout_mtx.lock();
                cout << name <<"'s wait was too long. Leaving!\n";
                mem_shop.cout_mtx.unlock();
            }
            else
            {
                mem_shop.num_waiting++;
                std::unique_lock<mutex> lk(c_mu);
                mem_shop.cv_barber.notify_one();
                mem_shop.cv_customer.wait(lk);
                
                mem_shop.cout_mtx.lock();
                auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                cout << "Customer " << name << " currnt time: " << millisec_since_epoch << endl;
                cout << name << " is getting haircut!\n";
                mem_shop.cout_mtx.unlock();
            }
        };
        ~Customer()
        {
            hislife.join();
        }
        mutex c_mu;
        Barbershop& mem_shop;
        thread hislife;
        unsigned int name;

};

int main()
{
    Barbershop mybarbershop;
    Barber dan(mybarbershop);
    vector<Customer*> customers;
    customers.reserve(NUM_CUSTOMERS-1);
    auto millisec_since_epoch = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    cout << millisec_since_epoch << endl;
    for(unsigned int i=0; i<NUM_CUSTOMERS; i++)
    {

        customers.push_back(new Customer(mybarbershop, i+1));
    }

    for(auto s : customers)
    {
        delete s;
    }
    cout << "Program Finished!\n";
    return 0;
}