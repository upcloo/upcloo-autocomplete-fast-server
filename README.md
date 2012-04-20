#UpCloo Autocomplete Fast Server

The UpCloo Autocomplete feature must reply very fast.

This implementation is undev development and is currently
completely unstable and maybe have memory problems. 

***Please do not use in production.***

## Event-driven impl

This library use libevent and memcached and is only a C event-driven
server that read keys from memcached and evelop its into a valid
JSONP package for the JS sdk of UpCloo Search.

## Compile it

You can use the ```Makefile```

```
make
``` 

For compile you need ```libevent``` and ```libmemcached```

Tipical library compile

```
./configure --prefix=/usr
```

Now you can launch the server.

```
./autocomplete upcloo-search.conf
```

### Debian Install

On Debian you can use the apt-get

```
apt-get install libevent-dev libmemcached-d
```

After that you can compile and install UpCloo Autocomplete

```
make && make install
```

The install copy the application under ```/usr/local/bin```, create a 
folder under ```/etc``` with a default configuration and after that
update the rc configuration for start on boot.

## Configuration file

Actually you can pass an argument to the daemon for
read configuration from file

```
./autocomplete upcloo-search.conf
```

Or you can skip completely external conf

```
./autocomplete
```

In single mode it binds the localhost at port 8080

### Configuration example

```
# UpCloo Search Autocomplete Configuration

# Start as daemon
daemonize yes

# Bind address and port
bind 0.0.0.0
port 8080

# Memcached servers
memcached.0.host 127.0.0.1
memcached.0.port 11211

memcached.1.host 192.168.2.155
memcached.1.port 11211
```

## Tests and Benchmarks

The follow bench is a test result for a valid memcached result for word
"ab" and a common response is the follow:

```
walter(["abatantuomo", "abba", "abbozzo"])
```

Now see results:

Virtual Machine:

 * 384MB RAM
 * One virtual CPU on MacBook White > 2009 DualCore Duo with 2GB RAM

Running localhost memcached daemon with few keys

```
This is ApacheBench, Version 2.3 <$Revision: 655654 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 1000 requests
Completed 2000 requests
Completed 3000 requests
Completed 4000 requests
Completed 5000 requests
Completed 6000 requests
Completed 7000 requests
Completed 8000 requests
Completed 9000 requests
Completed 10000 requests
Finished 10000 requests


Server Software:        
Server Hostname:        127.0.0.1
Server Port:            8080

Document Path:          /sitekey=th16RBthw&word=ab&callback=walter
Document Length:        16 bytes

Concurrency Level:      100
Time taken for tests:   1.393 seconds
Complete requests:      10000
Failed requests:        0
Write errors:           0
Total transferred:      800000 bytes
HTML transferred:       160000 bytes
Requests per second:    7179.87 [#/sec] (mean)
Time per request:       13.928 [ms] (mean)
Time per request:       0.139 [ms] (mean, across all concurrent requests)
Transfer rate:          560.93 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   3.0      0      12
Processing:     0   13   6.5     12      32
Waiting:        0   12   6.7     12      32
Total:          0   14   6.2     12      32

Percentage of the requests served within a certain time (ms)
  50%     12
  66%     16
  75%     20
  80%     20
  90%     24
  95%     24
  98%     28
  99%     28
 100%     32 (longest request)
```


