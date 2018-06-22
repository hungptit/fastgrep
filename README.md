# Introduction
The fgrep utility is searches any given input files for lines that match one or more patterns. This command line tool is written using modern C++ and is built on top of [ioutils](https://github.com/hungptit/ioutils "A blazing fast fast file I/O library"), [utils](https://github.com/hungptit/utils "A very fast string related functionality written in modern C++."), [fmt](https://github.com/fmtlib/fmt "A modern formating library"), [cereal](https://github.com/USCiLab/cereal "A C++11 library for serialization"), [hyperscan](https://github.com/intel/hyperscan "High-performance regular expression matching library."), and [boost libraries](https://www.boost.org/ "boost.org"). Our benchmarks and unit tests are written using Google [benchmark](https://github.com/google/benchmark "A microbenchmark support library"), [Celero](https://github.com/DigitalInBlue/Celero "C++ Benchmark Authoring Library/Framework"), and [Catch2](https://github.com/catchorg/Catch2 "A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD."). 

# What is the different between fgrep and grep or ripgrep #

fgrep is written as a library thus fgrep core code can be reused in other projects. All core algorithms are templatized so we can have flexible and reusable algorithms without sacrificing the performance. 

# Why fgrep is fast? #

The fgrep command is fast because of many reasons and below are key factors

## High-performance regular rexpression matching engine ##

fgrep uses [hyperscan](https://github.com/intel/hyperscan) as a regular expression matching engine. Our performance benchmarks have shown that [hyperscan](https://github.com/intel/hyperscan) is 20x or more faster than that of std::regex. 

## High-performance exact text matching algorithm ##

fgrep use SSE2 and AVX2 optimized algorithms for exact text matching. Our SSE2 and AVX2 algorithms are forked from [this repository](https://github.com/WojciechMula/sse4-strstr). Our core algorithms is 4x faster then the standard string find algorithm.

## Efficient file I/O ##

fgrep use very fast algorithms for reading data from file. 

## Modern C++ ##

All core algorithms are implemented using [Policy Based Design](https://en.wikipedia.org/wiki/Policy-based_design). This approach allows compiler to generated classes at compile time and it also provide an opportunity for compiler to inline methods and functions at compile time.

# Benchmark results #

It is impossible to get a good benchmark for text searching tools since the performance of each benchmarked command is relied on the search patterns. We use all patterns mentioned in this [link](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/) and their test data. All benchmarks can be found from the [benchmark folder](https://github.com/hungptit/fastgrep/tree/master/benchmark) and our benchmark results are consistent in all test platforms such as Gentoo kernel 4.17, CentOS 6.x, and macOS.

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

# FAQs #

## Does fastgrep try to compete with grep and/or ripgrep? ##

No. I think grep or ripgrep are feature complete and it is impossible to keep up with these commands in term of usability. fastgrep written as a library so I can use it in other projects. I only implemented some critical features and it will take a lot of time and effort to create something similar to grep or ripgrep.

## Can I contribute to fastgrep? ##

Any contribution is very welcome.

