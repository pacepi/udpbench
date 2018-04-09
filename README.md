# udpbench
## a multi tasks benchmark tool for udp. (For now, only support on Linux)
### Ex, run benchmark for echo server

> udpbench -p 7 -a 192.168.1.100 -c 10 -t 30
```
      -p 7, port 7 (echo service)
      -a 192.168.1.100, host address 192.168.1.100
      -c 10, run 10 clients
      -t 30, run 30 seconds
```