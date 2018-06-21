# Introduction

# Why fgrep is fast? #

fgrep command is fast because

1. It uses hyperscan, the fastest regular expression engine, for pattern matching.
2. It uses optimized string find algorithms for exact pattern matching.
3. It uses optimized algorithm for reading a file content.
4. All core algorithms are implemented using Policy Based Design approach. This allows compiler to generated classes and to provide an opportunity to inline methods and functions at compile time.

# Benchmark #

## Test environments ##
* CPU: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
* System memory: 16GBytes
* Hard drive: Samsung EVO 930.
* OS: Gentoo Linux with kernel 4.17 with glibc-2.27 and gcc-7.3.

## Test data ##

Test data and patterns are obtained from this [article](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/).

## Experiment setup ##

* All tests command are run using default options.
* The output of all commands are redirected to a temporary file.

## Results ##
``` text
hungptit@hunghien ~/working/fastgrep/benchmark $ ./all_tests
Celero
Timer resolution: 0.001000 us
-----------------------------------------------------------------------------------------------------------------------------------------------
Group           |   Experiment    |   Prob. Space   |     Samples     |   Iterations    |    Baseline     |  us/Iteration   | Iterations/sec  |
-----------------------------------------------------------------------------------------------------------------------------------------------
mark_twain      | grep_brew       |               0 |               5 |               1 |         1.00000 |   1018958.00000 |            0.98 |
mark_twain      | fgrep_warm_up   |               0 |               5 |               1 |         0.38867 |    396039.00000 |            2.53 |
mark_twain      | ag              |               0 |               5 |               1 |         1.23523 |   1258645.00000 |            0.79 |
mark_twain      | ripgrep         |               0 |               5 |               1 |         0.62198 |    633772.00000 |            1.58 |
mark_twain      | fgrep           |               0 |               5 |               1 |         0.39076 |    398173.00000 |            2.51 |
Complete.
```
##  ##
