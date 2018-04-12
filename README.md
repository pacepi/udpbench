# udpbench
## a multi tasks benchmark tool for udp. (For now, only support on Linux)

### Ex, run echo service on server
> udpserver -p 7 -s echo
```
      -p 7, port 7 (echo service)
      -s echo, run echo service
```

### Ex, run benchmark for echo server
> udpbench -p 7 -a 192.168.1.100 -c 10 -t 30
```
      -p 7, port 7 (echo service)
      -a 192.168.1.100, host address 192.168.1.100
      -c 10, run 10 clients
      -t 30, run 30 seconds
```

### Ex, test nginx UDP load balance performence
```
    run udpserver on 192.168.1.100 & 192.168.1.101 & 192.168.1.102

    run nginx on 192.168.1.110, nginx.conf like this :
        worker_processes  1;
        events {
            worker_connections  1024;
        }
        stream {
            upstream echo_upstreams {
                server 192.168.1.100:7;
                server 192.168.1.101:7;
                server 192.168.1.102:7;
            }
            server {
                listen 7 udp;
                proxy_pass echo_upstreams;
                proxy_timeout 1s;
                proxy_responses 1;
                error_log logs/echo-service.log;
            }
        }

    run udpbench on 192.168.1.120
```
