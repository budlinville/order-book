/*
SimpleCross - a process that matches internal orders

Overview:
    * Accept/remove orders as they are entered and keep a book of
      resting orders
    * Determine if an accepted order would be satisfied by previously
      accepted orders (i.e. a buy would cross a resting sell)
    * Output (print) crossing events and remove completed (fully filled)
      orders from the book

Inputs:
    A string of space separated values representing an action.  The number of
    values is determined by the action to be performed and have the following
    format:

    ACTION [OID [SYMBOL SIDE QTY PX]]

    ACTION: single character value with the following definitions
    O - place order, requires OID, SYMBOL, SIDE, QTY, PX
    X - cancel order, requires OID
    P - print sorted book (see example below)

    OID: positive 32-bit integer value which must be unique for all orders

    SYMBOL: alpha-numeric string value. Maximum length of 8.

    SIDE: single character value with the following definitions
    B - buy
    S - sell

    QTY: positive 16-bit integer value

    PX: positive double precision value (7.5 format)

Outputs:
    A list of strings of space separated values that show the result of the
    action (if any).  The number of values is determined by the result type and
    have the following format:

    RESULT OID [SYMBOL [SIDE] (FILL_QTY | OPEN_QTY) (FILL_PX | ORD_PX)]

    RESULT: single character value with the following definitions
    F - fill (or partial fill), requires OID, SYMBOL, FILL_QTY, FILL_PX
    X - cancel confirmation, requires OID
    P - book entry, requires OID, SYMBOL, SIDE, OPEN_QTY, ORD_PX (see example below)
    E - error, requires OID. Remainder of line represents string value description of the error

    FILL_QTY: positive 16-bit integer value representing qty of the order filled by
              this crossing event

    OPEN_QTY: positive 16-bit integer value representing qty of the order not yet filled

    FILL_PX:  positive double precision value representing price of the fill of this
              order by this crossing event (7.5 format)

    ORD_PX:   positive double precision value representing original price of the order (7.5 format)
              (7.5 format means up to 7 digits before the decimal and exactly 5 digits after the decimal)

Conditions/Assumptions:
    * The implementation should be a standalone Linux console application (include
      source files, testing tools and Makefile in submission)
    * The use of third party libraries is not permitted. 
    * The app should respond to malformed input and other errors with a RESULT
      of type 'E' and a descriptive error message
    * Development should be production level quality. Design and
      implementation choices should be documented
	* Performance is always a concern in software, but understand that this is an unrealistic test. 
	  Only be concerned about performance where it makes sense to the important sections of this application (i.e. reading actions.txt is not important).
    * All orders are standard limit orders (a limit order means the order remains in the book until it
      is either canceled, or fully filled by order(s) for its same symbol on the opposite side with an
      equal or better price).
    * Orders should be selected for crossing using price-time (FIFO) priority
    * Orders for different symbols should not cross (i.e. the book must support multiple symbols)

Example session:
    INPUT                                   | OUTPUT
    ============================================================================
    "O 10000 IBM B 10 100.00000"            | results.size() == 0
    "O 10001 IBM B 10 99.00000"             | results.size() == 0
    "O 10002 IBM S 5 101.00000"             | results.size() == 0
    "O 10003 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10003 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "O 10004 IBM S 5 100.00000"             | results.size() == 2
                                            | results[0] == "F 10004 IBM 5 100.00000"
                                            | results[1] == "F 10000 IBM 5 100.00000"
    "X 10002"                               | results.size() == 1
                                            | results[0] == "X 10002"
    "O 10005 IBM B 10 99.00000"             | results.size() == 0
    "O 10006 IBM B 10 100.00000"            | results.size() == 0
    "O 10007 IBM S 10 101.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 0
    "O 10008 IBM S 10 102.00000"            | results.size() == 1
                                            | results[0] == "E 10008 Duplicate order id"
    "O 10009 IBM S 10 102.00000"            | results.size() == 0
    "P"                                     | results.size() == 6
                                            | results[0] == "P 10009 IBM S 10 102.00000"
                                            | results[1] == "P 10008 IBM S 10 102.00000"
                                            | results[2] == "P 10007 IBM S 10 101.00000"
                                            | results[3] == "P 10006 IBM B 10 100.00000"
                                            | results[4] == "P 10001 IBM B 10 99.00000"
                                            | results[5] == "P 10005 IBM B 10 99.00000"
    "O 10010 IBM B 13 102.00000"            | results.size() == 4
                                            | results[0] == "F 10010 IBM 10 101.00000"
                                            | results[1] == "F 10007 IBM 10 101.00000"
                                            | results[2] == "F 10010 IBM 3 102.00000"
                                            | results[3] == "F 10008 IBM 3 102.00000"

So, for the example actions.txt, the desired output from the application with the below main is:
F 10003 IBM 5 100.00000
F 10000 IBM 5 100.00000
F 10004 IBM 5 100.00000
F 10000 IBM 5 100.00000
X 10002
E 10008 Duplicate order id
P 10009 IBM S 10 102.00000
P 10008 IBM S 10 102.00000
P 10007 IBM S 10 101.00000
P 10006 IBM B 10 100.00000
P 10001 IBM B 10 99.00000
P 10005 IBM B 10 99.00000
F 10010 IBM 10 101.00000
F 10007 IBM 10 101.00000
F 10010 IBM 3 102.00000
F 10008 IBM 3 102.00000

*/

// Stub implementation and example driver for SimpleCross.
// Your crossing logic should be accesible from the SimpleCross class.
// Other than the signature of SimpleCross::action() you are free to modify as needed.
#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include <map>
#include <sstream>


//----------------------------------------------------------------------------------------------------------------------
// Logging
template <typename T>
void log(T t) {
  std::cout << std::to_string(t) << std::endl;
}

template<>
void log(std::string str) {
  std::cout << str << std::endl;
}

template<>
void log(const char* str) {
  std::cout << str << std::endl;
}

template <typename T>
void log(std::vector<T> ts) {
  std::cout << "----------" << std::endl;
  for (T t : ts) { log(t); }
  std::cout << "----------" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------
// Type Definitions
//----------------------------------------------------------------------------------------------------------------------
typedef std::list<std::string> results_t;

enum Action {
  PLACE = 'O',
  CACNEL = 'X',
  PRINT = 'P',
};

enum Side {
  BUY = 'B',
  SELL = 'S',
};

typedef uint32_t OrderId;
typedef std::string Symbol;
typedef uint16_t Quantity;
typedef double Price;

struct Order {
  OrderId oid;
  Symbol symbol;
  Side side;
  Quantity qty;
  Price px;

  Order(OrderId _oid, Symbol _symbol, Side _side, Quantity _qty, Price _px)
    : oid(_oid)
    , symbol(_symbol)
    , side(_side)
    , qty(_qty)
    , px(_px)
    {
      if (_side != Side::BUY && _side != Side::SELL) {
        throw std::invalid_argument("Invalid Order Side");
      }
    };
};

typedef std::deque<Order> OrderQueue;
typedef std::map<Price, OrderQueue> PriceLevels;
struct Sides { PriceLevels bids; PriceLevels asks; };
typedef std::map<Symbol, Sides> OrderBook;
typedef std::map<OrderId, Order> OrderCache;

//----------------------------------------------------------------------------------------------------------------------
// Simple Cross Order Book Driver
//----------------------------------------------------------------------------------------------------------------------
class SimpleCross {
public:
  results_t action(const std::string line);

private:
  void _placeOrder(Order &order);
  void _cancelOrder(OrderId oid);
  void _printSortedBook();

  void _placeOrderNewSymbol(Order &order);
  void _placeOrderExistingSymbol(Order &order);
  void _fillOrder(Order &order);
  void _fillBid(Order &order);
  void _fillAsk(Order &order);

  std::vector<std::string> _splitLine(const std::string line, const char delim=' ');

private:
  OrderBook orderBook;
  OrderCache orderCache;
};

//----------------------------------------------------------------------------------------------------------------------
results_t SimpleCross::action(const std::string line) {
  std::vector<std::string> instructions = _splitLine(line);
  if (instructions[0] == "O") {
    Order order = Order(
      static_cast<OrderId>(std::stoi(instructions[1])),
      static_cast<Symbol>(instructions[2]),
      static_cast<Side>(instructions[3].front()),
      static_cast<Quantity>(std::stoi(instructions[4])),
      static_cast<Price>(std::stod(instructions[5]))
    );
    _placeOrder(order);
  } else if (instructions[0] == "X") {
    _cancelOrder(std::stoi(instructions[1]));
  } else if (instructions[0] == "P") {
    _printSortedBook();
  }

  return results_t();
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_placeOrder(Order &order) {
  // Note: In a real system, all the traded symbols would probably be loaded on startup,
  //       but given the problem constraints, we will generate the book on the fly
  if (orderBook.find(order.symbol) == orderBook.end()) {
    _placeOrderNewSymbol(order);
  } else {
    _placeOrderExistingSymbol(order);
  }
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_placeOrderNewSymbol(Order &order) {
  log("------------------------------------------------------------");
  log("Symbol not in book!");
  log("New order id : " + std::to_string(order.oid));
  log("New order px : " + std::to_string(order.px));
  log("New order qty: " + std::to_string(order.qty));
  // Create Order Queue
  OrderQueue orderQueue;
  orderQueue.emplace_back(order);

  // Create PriceLevels and insert first price level
  PriceLevels bidPxLevels, askPxLevels;
  if (order.side == Side::BUY) {
    bidPxLevels.insert(std::pair<Price, OrderQueue>(order.px, orderQueue));
  } else if (order.side == Side::SELL) {
    askPxLevels.insert(std::pair<Price, OrderQueue>(order.px, orderQueue));
  }

  Sides sides;
  sides.bids = bidPxLevels;
  sides.asks = askPxLevels;

  orderBook.insert(std::pair<Symbol, Sides>(order.symbol, sides));

  _printSortedBook();
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_placeOrderExistingSymbol(Order &order) {
  log("------------------------------------------------------------");
  log("Symbol found!");

  // First attempt to fill order
  _fillOrder(order);

  log("New order id : " + std::to_string(order.oid));
  log("New order px : " + std::to_string(order.px));
  log("New order qty: " + std::to_string(order.qty));

  // TODO : Working here...
  if (order.qty != 0) { // order was not completely filled
    PriceLevels& pxLevels = order.side == Side::BUY
      ? orderBook[order.symbol].bids
      : orderBook[order.symbol].asks;

    if (pxLevels.find(order.px) == pxLevels.end()) {  // new price level
      OrderQueue orderQueue;
      orderQueue.emplace_back(order);
      pxLevels.insert(std::pair<Price, OrderQueue>(order.px, orderQueue));
    } else {  // existing price level
      pxLevels[order.px].emplace_back(order);
    }
  }
  
  _printSortedBook();

  // PriceLevels pxLevels = orderBook[order.symbol];
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_cancelOrder(OrderId oid) {
  log(oid);
}

/*---------------------------------------------------------------------------------------------------------------------
// Attempt to fill order in place. Order's quantity could have changed after calling this function
//   If order crosses entirely, order's shares will be 0
//   If partial fill, order's shares will be the remaining unfilled shares
//   If order does not cross book, order's shares will be their original value
// TODO: The buy and ask branches are similar. Could potentially generalize with templates
//---------------------------------------------------------------------------------------------------------------------*/
void SimpleCross::_fillOrder(Order &order) {
  if (order.side == Side::BUY) {
    _fillBid(order);
  } else if (order.side == Side::SELL) {
    _fillAsk(order);
  }
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_fillBid(Order &order) {
  log("Attempting to fill bid!");
  // Already know symbol is in orderBook from calling function
  PriceLevels& askPxLevels = orderBook[order.symbol].asks;

  for (std::pair<Price, OrderQueue> aPxLevel : askPxLevels) {
    Price askPrice = aPxLevel.first;
    OrderQueue& askOrderQueue = aPxLevel.second;

    if (order.px >= askPrice) {
      for (Order& restingOrder : askOrderQueue) {
        Quantity sharesExecuted = std::min(restingOrder.qty, order.qty);
        order.qty -= sharesExecuted;
        restingOrder.qty -= sharesExecuted;

        if (restingOrder.qty == 0) askOrderQueue.pop_front();
        if (order.qty == 0) break;
      }

      if (askOrderQueue.empty()) askPxLevels.erase(askPrice);
      if (order.qty == 0) break;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_fillAsk(Order &order) {
  log("Attempting to fill ask!");
  // Already know symbol is in orderBook from calling function
  PriceLevels& bidPxLevels = orderBook[order.symbol].bids;

  // Iterate in reverse order because bidPxLevels.end() is most competitive price
  for (auto pxlIt = bidPxLevels.rbegin(); pxlIt != bidPxLevels.rend(); ++pxlIt) {
    Price bidPrice = pxlIt->first;
    OrderQueue& bidOrderQueue = pxlIt->second;

    if (order.px <= bidPrice) {
      log("Crossing order!");
      for (Order& restingOrder : bidOrderQueue) {
        Quantity sharesExecuted = std::min(restingOrder.qty, order.qty);
        order.qty -= sharesExecuted;
        restingOrder.qty -= sharesExecuted;

        if (restingOrder.qty == 0) bidOrderQueue.pop_front();
        if (order.qty == 0) break;
      }

      if (bidOrderQueue.empty()) bidPxLevels.erase(bidPrice);
      if (order.qty == 0) break;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
std::vector<std::string> SimpleCross::_splitLine(const std::string line, const char delim) {
  std::vector<std::string> ret{};

  std::istringstream iss(line);
  std::string item = "";
  
  while (std::getline(iss, item, delim)) {
      ret.emplace_back(item);
  }

  return ret;
}

//----------------------------------------------------------------------------------------------------------------------
void SimpleCross::_printSortedBook() {
  if (orderBook.empty()) {
    log("Book empty!");
    return;
  }

  const std::string INDENT_1 = "|--";
  const std::string INDENT_2 = "|   ";
  const std::string INDENT_3 = "|    ";
  const std::string INDENT_4 = "|     ";

  log(" ________________________");
  log("| Order Book");
  for (std::pair<Symbol, Sides> symbolSides : orderBook) {
    log(INDENT_1 + symbolSides.first);

    log(INDENT_2 + "Bids");
    if (symbolSides.second.bids.empty()) log(INDENT_3 + "[EMPTY]");
    for (std::pair<Price, OrderQueue> pxLevel : symbolSides.second.bids) {
      log(INDENT_3 + "$" + std::to_string(pxLevel.first));
      log(INDENT_4 + "OID  \tQTY");
      for (Order order : pxLevel.second) {
        log(INDENT_4 + std::to_string(order.oid) + "\t" + std::to_string(order.qty));
      }
    }

    log(INDENT_2 + "Asks");
    if (symbolSides.second.asks.empty()) log(INDENT_3 + "[EMPTY]");
    for (std::pair<Price, OrderQueue> pxLevel : symbolSides.second.asks) {
      log(INDENT_3 + "$" + std::to_string(pxLevel.first));
      log(INDENT_4 + "OID\tQTY");
      for (Order order : pxLevel.second) {
        log(INDENT_4 + std::to_string(order.oid) + "\t" + std::to_string(order.qty));
      }
    }
  }
}


//----------------------------------------------------------------------------------------------------------------------
// Main
//----------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    SimpleCross scross;
    std::string line;
    std::ifstream actions("actions.txt", std::ios::in);
    while (std::getline(actions, line)) {
        results_t results = scross.action(line);
        for (results_t::const_iterator it=results.begin(); it!=results.end(); ++it) {
            std::cout << *it << std::endl;
        }
    }
    return 0;
}


