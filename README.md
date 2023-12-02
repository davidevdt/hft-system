# hft-system

## High Frequency Trading System - An application of low latency systems

This project aims at building an HFT system. It consists of a demo of a HF trading platform. This project represents a practical implementation of a low latency application (whose core elements can be found in the utilities folder). 

The system consists of the following components: 

* `utils` which contains custom utility structures (lock-free queue, logger, memory pool, ...)
* `exchange` which contains objects and structures (publisher, order  book, matchin engine, ...) from the exchange side
* `trading` which contains the main functionalities on the market participant (trading strategies, risk managers, ...) 

The system is entirely implemented in c++. 

## References
The system is built following the methodologies presented in Sourav Ghosh's book [Building Low Latency Applications with C++](https://www.packtpub.com/product/building-low-latency-applications-with-c/9781837639359) (Packt, 2023). The original book's repo can be found [in this link](https://github.com/PacktPublishing/Building-Low-Latency-Applications-with-CPP).