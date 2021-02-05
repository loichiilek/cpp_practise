#include <iostream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <fstream> 
#include <algorithm> 

using namespace std;


enum OrderType {BUY, SELL};


class ClientOrder {
    string orderID, clientID, symbol;
    OrderType orderType;
    double price;
    int quantity;

    public:
        ClientOrder(string orderID, string clientID, string symbol, OrderType orderType, double price, int quantity)
        : orderID {orderID},
        clientID {clientID},
        symbol {symbol},
        orderType {orderType},
        price {price},
        quantity {quantity}
        {
        };

        void printClientOrder() {
            cout << orderID << "\t" << clientID << "\t" << symbol << "\t" << orderType << "\t" << price << "\t" << quantity << "\n";
        }

        string getSymbol(){
            return symbol;
        }

        OrderType getOrderType(){
            return orderType;
        }

        double getOrderPrice(){
            return price;
        }

        int getQuantity(){
            return quantity;
        }

        void setQuantity(int newQuantity){
            quantity = newQuantity;
        }
};


struct CompareOrderPrice {
    bool operator()(ClientOrder co1, ClientOrder co2)
    {

        return co1.getOrderPrice() < co2.getOrderPrice();
    }
};

struct ReverseCompareOrderPrice {
    bool operator()(ClientOrder co1, ClientOrder co2)
    {

        return co1.getOrderPrice() > co2.getOrderPrice();
    }
};

class OrderBook {
    string symbol;
    vector<ClientOrder> bidHeap, askHeap;

    public:
        OrderBook(string symbol) : symbol {symbol} 
        {   }

        // TODO: Change list to heap

        void addOrder(ClientOrder clientOrder){
            if (clientOrder.getOrderType() == OrderType::BUY) {
                bidHeap.push_back(clientOrder);
                push_heap(bidHeap.begin(), bidHeap.end(), CompareOrderPrice{});
                
            } else {
                askHeap.push_back(clientOrder);
                push_heap(askHeap.begin(), askHeap.end(), ReverseCompareOrderPrice{});
            }
        }

        void matchOrder() {
            if (bidHeap.empty() || askHeap.empty()){
                return;
            }

            pop_heap(bidHeap.begin(), bidHeap.end(), CompareOrderPrice{});

            pop_heap(askHeap.begin(), askHeap.end(), ReverseCompareOrderPrice{});

            // While there exists an overlap of price
            while(bidHeap.back().getOrderPrice() >= askHeap.back().getOrderPrice()){
                
                // More bids than asks
                if (bidHeap.back().getQuantity() > askHeap.back().getQuantity()){
                    // reduce bid quantity
                    bidHeap.back().setQuantity(bidHeap.back().getQuantity() - askHeap.back().getQuantity());
                    

                    // remove the order from askHeap
                    askHeap.pop_back();

                    // retrieve the next best ask
                    pop_heap(askHeap.begin(), askHeap.end(), ReverseCompareOrderPrice{});
                }

                // More asks than bids
                else if (askHeap.back().getQuantity() > bidHeap.back().getQuantity()){
                    askHeap.back().setQuantity(askHeap.back().getQuantity() - bidHeap.back().getQuantity());

                    // remove the order from bidHeap
                    bidHeap.pop_back();

                    // retrieve the next best bid
                    pop_heap(bidHeap.begin(), bidHeap.end(), ReverseCompareOrderPrice{});
                }

                // Equal asks and bids
                else {
                    // Remove both orders
                    bidHeap.pop_back();
                    askHeap.pop_back();

                    // retrieve the next best bid
                    pop_heap(bidHeap.begin(), bidHeap.end(), ReverseCompareOrderPrice{});
                    // retrieve the next best ask
                    pop_heap(askHeap.begin(), askHeap.end(), ReverseCompareOrderPrice{});

                }

            }

            // Push the largest/smallest back on top of the heap
            push_heap(bidHeap.begin(), bidHeap.end(), CompareOrderPrice{});
            push_heap(askHeap.begin(), askHeap.end(), ReverseCompareOrderPrice{});

        }

        void printOrderBook() {
            vector<ClientOrder>::iterator itr;
            cout << "====BID====\n";
            for (itr = bidHeap.begin(); itr != bidHeap.end(); ++itr){
                itr->printClientOrder();
            }
            
            
            cout << "====ASK====\n";
            for (itr = askHeap.begin(); itr != askHeap.end(); ++itr){
                itr->printClientOrder();
            }
        }

        // void testPopHeap() {
        //     cout << "\npop that shit\n";
        //     while(!bidHeap.empty()){
        //         pop_heap(bidHeap.begin(), bidHeap.end(), CompareOrderPrice{});
        //         ClientOrder o = bidHeap.back();
        //         o.printClientOrder();
        //         bidHeap.pop_back();
        //     }
        // }
};

class Broker {
    map<string, OrderBook> orderBooks;

    private:
        void processOrder(ClientOrder clientOrder){
            // Check if orderBooks has symbol from clientOrder, create new orderbook if does not exist
            map<string, OrderBook>::iterator itr = orderBooks.find(clientOrder.getSymbol());

            // orderBook for the particular symbol exists
            if (itr != orderBooks.end()) {
                itr->second.addOrder(clientOrder);
                itr->second.matchOrder();
            } 
            // orderBook does not exist
            else {
                OrderBook orderBook(clientOrder.getSymbol());
                orderBook.addOrder(clientOrder);
                orderBooks.insert(pair<string, OrderBook> (clientOrder.getSymbol(), orderBook));
            }

            // STILL REQUIRE MATCHING
        };

    public:
        void parseOrderCSVFile(string inputFileName) {
            ifstream inputFile(inputFileName);
            int l = 0;
        
            while (inputFile) {
                l++;
                string s;
                if (!getline(inputFile, s)) break;
                if (s[0] != '#') {
                    istringstream ss(s);

                    string orderID;
                    string clientID;
                    string symbol;
                    OrderType orderType;
                    double price;
                    int quantity;

                    string line;

                    // Parsing each field and process order
                    try {
                        getline(ss, line, ',');
                        orderID = line;
                        
                        getline(ss, line, ',');
                        clientID = line;
                        
                        getline(ss, line, ',');
                        symbol = line;
                        
                        getline(ss, line, ',');
                        orderType = OrderType(stoi(line));
                        
                        getline(ss, line, ',');
                        price = stof(line);
                        
                        getline(ss, line, ',');
                        quantity = stoi(line);

                        ClientOrder clientOrder(orderID, clientID, symbol, orderType, price, quantity);
                        processOrder(clientOrder);
                    }

                    catch (const std::invalid_argument e) {
                        cout << "NaN found in file " << inputFileName << " line " << l << endl;
                        e.what();
                    }
                    
                }
            }
        
            if (!inputFile.eof()) {
                cerr << "Could not read file " << inputFileName << "\n";
            }

        };

        void printOrderBooks(){

            map<string, OrderBook>::iterator itr;
            
            for (itr = orderBooks.begin(); itr!=orderBooks.end(); ++itr){
                cout << itr->first << '\n';

                itr->second.printOrderBook();
            }
        };
};




int main() {
    Broker broker;
    broker.parseOrderCSVFile("test.csv");
    broker.printOrderBooks();

    

};