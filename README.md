# timemory-ping

Logs ping info via timemory and generates a JSON which can be processed in Python.

## Building

Very basic w/o installation of timemory, no CMake. Lots of templates in timemory so
it will take a while despite how little code there is here. From this directory:

```console
$ git clone https://github.com/NERSC/timemory.git timemory-source
$ time clang++ -I. -Itimemory-source/source -O2 ting.cpp -o ting

real    0m51.789s
user    0m50.291s
sys	    0m1.017s
```

## Running

```console
$ sudo ./ting --limit 10 google.com

Resolving DNS..

Trying to connect to 'google.com' IP: 172.217.6.78

Reverse Lookup domain: sfo07s17-in-f14.1e100.net
Socket file descriptor 3 received

Socket set to TTL..
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 1 ttl = 64 rtt = 10.602000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 2 ttl = 64 rtt = 9.626000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 3 ttl = 64 rtt = 9.708000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 4 ttl = 64 rtt = 10.019000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 5 ttl = 64 rtt = 9.859000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 6 ttl = 64 rtt = 14.099000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 7 ttl = 64 rtt = 10.291000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 8 ttl = 64 rtt = 10.066000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 9 ttl = 64 rtt = 10.471000 msec
64 bytes from sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) msg_seq = 10 ttl = 64 rtt = 9.677000 msec

===172.217.6.78 ping statistics===

10 packets sent, 10 packets received, 0.000000 percent packet loss.Total time: 10133.087000 ms.

 >>>  sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) : 10 packets sent, 10 packets received, 0% packet loss,    0.104 sec monotonic_clock sec ping_data [laps: 10]
[ping_data]|0> Outputting 'timemory-ting-output/ping_data.json'...
[ping_data]|0> Outputting 'timemory-ting-output/ping_data.tree.json'...
[ping_data]|0> Outputting 'timemory-ting-output/ping_data.txt'...

|----------------------------------------------------------------------------------------------------------------------------------------------------------|
|                                                                 PROVIDES PING STATISTICS                                                                 |
|----------------------------------------------------------------------------------------------------------------------------------------------------------|
|                           LABEL                             | COUNT  | DEPTH  |  METRIC   | UNITS  |  SUM   | MEAN   |  MIN   |  MAX   | STDDEV | % SELF |
|-------------------------------------------------------------|--------|--------|-----------|--------|--------|--------|--------|--------|--------|--------|
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.011 |  0.011 |  0.011 |  0.011 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.014 |  0.014 |  0.014 |  0.014 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
| >>> sfo07s17-in-f14.1e100.net (h: google.com)(172.217.6.78) |      1 |      0 | ping_data | sec    |  0.010 |  0.010 |  0.010 |  0.010 |  0.000 |  100.0 |
|----------------------------------------------------------------------------------------------------------------------------------------------------------|

```

## Notes

- Output will be in `timemory-ting-output` folder
  - Add `tim::settings::time_output() = true` to source if you want a sub-directory with a timestamp
- `ping_data.json` is easier to parse manually in Python
- `ping_data.tree.json` is the JSON layout for Hatchet (which will convert the data to pandas dataframes)
- Compile time could be easily reduced by replacing direct usage of timemory with function calls to timemory in another source
  - i.e. all references to timemory are in `impl.cpp` and then any changes in `ting.cpp` don't require re-compiling timemory

## Limitations

Current implementation has a fixed packet size.

## Credits

Ping implementation adapted from here: https://www.geeksforgeeks.org/ping-in-c/
