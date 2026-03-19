### 1.High-Level Design (HLD)

##### The system acts as a central hub that accepts orders, checks memory efficiency, and matches them against resting orders in the book.

```
+-----------------+        +-----------------------+
|  Client / User  |------->|   MatchingEngine      |
+-----------------+        | (The Brain)           |
                           | - ProcessOrder()      |
                           | - matchOrder()        |
                           +----------+------------+
                                      |
                                      v
                           +-----------------------+
                           |      OrderBook        |
                           | (The Container)       |
                           | - Bids Map            |
                           | - Asks Map            |
                           +----------+------------+
                                      |
          +---------------------------+---------------------------+
          |                                                       |
          v                                                       v
+------------------+                                   +------------------+
| Bids (Buy Side)  |                                   | Asks (Sell Side) |
| Map<Price, Level>|                                   | Map<Price, Level>|
| Sorted: High->Low|                                   | Sorted: Low->High|
+------------------+                                   +------------------+
          ^                                                       ^
          |                                                       |
          +---------------------------+---------------------------+
                                      |
                           +----------+------------+
                           |      OrderPool        |
                           | (Memory Manager)      |
                           | - Pre-allocated Block |
                           | - Free List           |
                           +-----------------------+



```

### Workflow Sequence
#### Input: A new order request arrives.
#### Allocation: The OrderPool provides a memory slot instantly (O(1)).
#### Routing: MatchingEngine checks the side (Buy/Sell).
#### Matching: It looks at the opposite side of the book.
#### Buy Order looks at Asks (Lowest Price first).
#### Sell Order looks at Bids (Highest Price first).
#### Execution: If prices match, quantities are traded.
#### Resting: If quantity remains, the order is added to the OrderBook.

### 2. Low-Level Design (LLD)
#### This section details the specific data structures and memory layout used to achieve low latency.

#### A. The OrderPool (Memory Strategy)
##### Instead of asking the Operating System for memory every time (slow), you allocate one giant block once.

```
State: Empty Pool (Initialized)
+------+------+------+------+------+
| Slot | Slot | Slot | Slot | ... |
|  0   |  1   |  2   |  3   |      |
+------+------+------+------+------+
  |      |      |      |
  v      v      v      v
(next)->(next)->(next)->nullptr
  ^
  |
FreeListHead

State: After Allocating 1 Order
+------+------+------+------+------+
| USED | Slot | Slot | Slot | ... |
|Order |  1   |  2   |  3   |      |
+------+------+------+------+------+
         |      |      |
         v      v      v
       (next)->(next)->nullptr
         ^
         |
     FreeListHead (Moves to the next available slot)


```

### Why this is fast:

#### Allocation: Just move the FreeListHead pointer. No malloc call.
#### Deallocation: Point the deleted order's next to current FreeListHead, update FreeListHead to deleted order.

### B. The OrderBook & PriceLevel (The Core DS)
#### This is the most critical part. We use a Map of Doubly Linked Lists.

### 1. The Map Container
#### We use std::map because it keeps keys sorted automatically.

#### Bids: Sorted Greatest to Least (std::greater). The first element (begin()) is the Best Bid.
#### Asks: Sorted Least to Greatest (std::less). The first element (begin()) is the Best Ask.

### 2. The PriceLevel (Linked List)
#### At each price point, we have a queue of orders. We use a Doubly Linked List to maintain "Time #### Priority" (First In, First Out).

#### Visual Representation of the OrderBook:

#### Let's assume the current market state:

#### Bids (Buying): People want to buy at 100, 99, 98.
#### Asks (Selling): People want to sell at 101, 102, 103.

```
ORDER BOOK VISUALIZATION
========================

[ BIDS SIDE (Buyers) ]                  [ ASKS SIDE (Sellers) ]
Sorted: HIGH -> LOW                     Sorted: LOW -> HIGH

Key: 100 (Best Bid)                     Key: 101 (Best Ask)
+------------------+                    +------------------+
|   PriceLevel     |                    |   PriceLevel     |
|   Price: 100     |                    |   Price: 101     |
+------------------+                    +------------------+
|        ^                             |        ^
v        |                             v        |
[ Order A ] [ Order B ]                [ Order X ] [ Order Y ]
Qty: 10    Qty: 5                      Qty: 20    Qty: 15
Time: 1    Time: 2                     Time: 1    Time: 2
|        ^                             |        ^
v        |                             v        |
[HEAD]   [TAIL]                        [HEAD]   [TAIL]

   |                                    |
   v                                    v
Key: 99                               Key: 102
+------------------+                    +------------------+
|   PriceLevel     |                    |   PriceLevel     |
+------------------+                    +------------------+
|                                        |
v                                        v
[ Order C ]                               [ Order Z ]
Qty: 50                                   Qty: 30
C. The Order Node Structure
Every rectangle in the linked list above is an Order struct from your Types.hpp.

Order Object
+------------------------+
| ID: 101                |
| Price: 100.50          |
| Qty: 10                |
| Side: Buy              |
+------------------------+
| prev: 0xABC... --------+---> Points to Order ahead of it in line
| next: 0xFF2... --------+---> Points to Order behind it in line
+------------------------+


```


### 3. Workflow Execution: A Trade Example
#### Let's trace exactly what happens in your code when a New Sell Order arrives.

```
Scenario:

Current Book: Has a Bid at Price 100 with 2 orders: Order A (Qty 10), Order B (Qty 5).
New Order: Sell Order for 12 units at Price 99.
Step 1: Process Order
The engine sees Side::Sell. It calls matchOrder(newOrder, book.bids).

Step 2: Iterator Starts
The iterator it points to begin() of Bids.

Map Key: 100 (The highest price).
This allows a match because NewPrice (99) <= BidPrice (100).
Step 3: Traversing the Level
We enter the while loop inside the PriceLevel at price 100.
```

#### VISUALIZING THE MATCH LOOP:

### Iteration 1:
#### NewOrder (Sell 12)  vs  Order A (Buy 10)

#### PriceLevel (100):
#### [ Order A (Qty:10) ] -> [ Order B (Qty:5) ]
       ^
#### currentOppositeOrder

#### Action: Match 10 units.
#### Result: Order A is empty. Remove Order A. NewOrder Qty = 2.

#### Updated State:
#### [ Order B (Qty:5) ]
#### ^
#### currentOppositeOrder (Moved to next)

---

### Iteration 2:
#### NewOrder (Sell 2)  vs  Order B (Buy 5)

#### Action: Match 2 units.
#### Result: NewOrder Qty = 0 (Filled). Order B Qty = 3 (Partial Fill).

#### Updated State:
#### [ Order B (Qty:3) ]

### Step 4: Loop Termination

#### newOrder->quantity becomes 0.
#### The inner loop breaks.
#### The engine checks newOrder->quantity > 0. It is false, so the order is NOT added to the book.
### The trade is complete.

### Performance Note:
#### In Step 3, removing Order A is O(1) because we have the pointer to Order A and it is a Doubly Linked #### List. We simply update the neighbors:
#### Order A->prev->next = Order A->next
