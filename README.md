# Introduction

# Benchmark #

## Test environments ##
* CPU: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
* System memory: 16GBytes
* Hard drive: Samsung EVO 930.
* OS: Gentoo Linux with kernel 4.17 with glibc-2.27 and gcc-7.3.

## Test data ##

Test data and patterns are obtained from this [article](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/).

## Experiment setup ##



## Results ##
``` text
hungptit@hunghien ~/working/fastgrep/benchmark $ ./all_tests
Celero
Timer resolution: 0.001000 us
-----------------------------------------------------------------------------------------------------------------------------------------------
     Group      |   Experiment    |   Prob. Space   |     Samples     |   Iterations    |    Baseline     |  us/Iteration   | Iterations/sec  |
-----------------------------------------------------------------------------------------------------------------------------------------------
mark_twain      | grep_brew       |               0 |               5 |               1 |         1.00000 |   1035293.00000 |            0.97 |
mark_twain      | fgrep_warm_up   |               0 |               5 |               1 |         0.11802 |    122185.00000 |            8.18 |
mark_twain      | ag              |               0 |               5 |               1 |         1.21962 |   1262663.00000 |            0.79 |
mark_twain      | ripgrep         |               0 |               5 |               1 |         0.61003 |    631556.00000 |            1.58 |
mark_twain      | fgrep           |               0 |               5 |               1 |         0.11480 |    118848.00000 |            8.41 |
Complete.
```
##  ##
