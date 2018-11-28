# HTTP frameworks and servers overhead measurments

* ./epoll

```
Running 2m test @ http://159.69.156.237:8080/
  8 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   167.06ms  271.84ms   2.00s    89.24%
    Req/Sec     6.67k     2.28k   19.13k    68.83%
  Latency Distribution
     50%   28.02ms
     75%  239.70ms
     90%  463.76ms
     99%    1.46s 
  6369447 requests in 2.00m, 1.14GB read
  Socket errors: connect 0, read 0, write 0, timeout 8040
Requests/sec:      53044.78
Transfer/sec:      9.76MB
```

```
Running 5m test @ http://159.69.156.237:8080/
  8 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   175.00ms  276.94ms   2.00s    88.86%
    Req/Sec     6.73k     2.19k   19.06k    69.18%
  Latency Distribution
     50%   29.80ms
     75%  243.24ms
     90%  471.61ms
     99%    1.48s 
  16058907 requests in 5.00m, 2.89GB read
  Socket errors: connect 0, read 0, write 0, timeout 21579
Requests/sec:      53515.27
Transfer/sec:      9.85MB
```

**epoll_wait(0)**

* ./seastar_http -c2 --poll-mode --lock-memory 1

```
Running 2m test @ http://159.69.156.237:8080/
  8 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   160.85ms  213.16ms   2.00s    88.31%
    Req/Sec     5.99k     1.49k   12.91k    69.13%
  Latency Distribution
     50%   41.01ms
     75%  254.98ms
     90%  399.29ms
     99%    1.00s 
  5720763 requests in 2.00m, 1.08GB read
  Socket errors: connect 0, read 0, write 0, timeout 3654
Requests/sec:      47641.14
Transfer/sec:      9.22MB
```

```
Running 5m test @ http://159.69.156.237:8080/
  8 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   168.63ms  221.92ms   2.00s    88.54%
    Req/Sec     5.87k     1.46k   13.58k    68.59%
  Latency Distribution
     50%   43.39ms
     75%  260.00ms
     90%  417.22ms
     99%    1.04s 
  14014553 requests in 5.00m, 2.65GB read
  Socket errors: connect 0, read 0, write 0, timeout 10800
Requests/sec:      46700.43
Transfer/sec:      9.04MB
```
