# Design of the Project

## Custom Data Types:

### Order : To store an order ( buy/sell, price , id)

## Memory Pool:

##### Custom Memory allocation & deallocation functions for efficient memory usage.

###### Result of custom vs default memory allocators:

```
Standard new/delete: 134371 us
Custom Memory Allocation: 7005 us
```

This test was done using the engine_test.exe file made especially for testing these functionalities.
