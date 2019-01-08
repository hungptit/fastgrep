# Introduction
The fgrep utility searches any given input files for lines that match one or more patterns. fgrep is written using modern C++ and is built on top of [ioutils](https://github.com/hungptit/ioutils "A blazing fast fast file I/O library"), [utils](https://github.com/hungptit/utils "A very fast string related functionality written in modern C++"), [fmt](https://github.com/fmtlib/fmt "A modern formating library"), [cereal](https://github.com/USCiLab/cereal "A C++11 library for serialization"), [hyperscan](https://github.com/intel/hyperscan "High-performance regular expression matching library."), and [boost libraries](https://www.boost.org/ "boost.org"). Our benchmarks and unit tests are written using Google [benchmark](https://github.com/google/benchmark "A microbenchmark support library"), [Celero](https://github.com/DigitalInBlue/Celero "C++ Benchmark Authoring Library/Framework"), and [Catch2](https://github.com/catchorg/Catch2 "A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD").

# What is the different between fgrep and grep or ripgrep #

fgrep is modular and it can be reused in other projects. All core algorithms are templatized so we can have flexible and reusable code without sacrificing the performance.

# Why fgrep is fast? #

The fgrep command is fast because of many reasons and below are key factors

## High-performance regular rexpression matching engine ##

fgrep uses [hyperscan](https://github.com/intel/hyperscan) as a regular expression matching engine. Our performance benchmarks have shown that [hyperscan](https://github.com/intel/hyperscan) is 20x or more faster than that of std::regex.

## High-performance exact text matching algorithm ##

fgrep uses SSE2 and AVX2 optimized algorithms for exact text matching. Our SSE2 and AVX2 algorithms are forked from [this repository](https://github.com/WojciechMula/sse4-strstr). Our exact matching algorithms is 2-4x faster than the standard string find algorithm.

## Efficient file I/O ##

fgrep uses very fast algorithms for reading data from file.

## Modern C++ ##

All core algorithms are implemented using [Policy Based Design](https://en.wikipedia.org/wiki/Policy-based_design). This approach allows compiler to generated classes at compile time and it also provides an opportunity for compiler to inline methods and functions at compile time.

# Benchmark results #

It is impossible to get a good benchmark for text searching tools since the performance of each benchmarked command is relied on search patterns. We use all patterns mentioned in this [link](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/) and their test data. All benchmarks can be found from the [benchmark folder](https://github.com/hungptit/fastgrep/tree/master/benchmark) and our benchmark results are consistent in all test platforms such as Gentoo kernel 4.17, CentOS 6.x, and macOS.

## Test environments ##
**Linux**
* CPU: * Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
* System memory: 792 GBytes
* Hard drive: Very fast network drives
* OS: Kernel 3.8.13
* Compiler: gcc-5.5.0

**MacBook Pro**
* CPU: 2.2 GHz Intel Core i7
* System memory: 16 GBytes
* Hard drive: SSD
* OS: Darwin Kernel Version 16.7.0
* Compiler: Apple LLVM version 9.0.0 (clang-900.0.39.2)

## Test data ##

Test data and patterns are obtained from this [article](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/). Benchmark results show that:
* fastgrep and ripgrep performance are comparable. The binary size of fastgrep is significantly larger than ripgrep so it might be slower for small text files.
* fastgrep and grep are similar interm of raw performance. 
* ag is the slower than grep, ripgrep, and fastgrep.

## Experiment setup ##

* All tests command are run using default options.
* The output of all commands are redirected to a temporary file.

## Results ##
**Linux**

Below benchmark results are collected with 
* GNU grep 3.1
* ag 2.2.0
* rg 0.10.0
* fgrep master branch.

```
./all_tests
Celero
Timer resolution: 0.001000 us
-----------------------------------------------------------------------------------------------------------------------------------------------
     Group      |   Experiment    |   Prob. Space   |     Samples     |   Iterations    |    Baseline     |  us/Iteration   | Iterations/sec  |
-----------------------------------------------------------------------------------------------------------------------------------------------
mark_twain      | grep            |               0 |               5 |               1 |         1.00000 |   1832815.00000 |            0.55 |
mark_twain      | ag              |               0 |               5 |               1 |         1.27726 |   2340976.00000 |            0.43 |
mark_twain      | ripgrep         |               0 |               5 |               1 |         0.44127 |    808766.00000 |            1.24 |
mark_twain      | fgrep_mmap      |               0 |               5 |               1 |         0.40452 |    741414.00000 |            1.35 |
mark_twain      | fgrep_stream    |               0 |               5 |               1 |         0.40558 |    743357.00000 |            1.35 |
mark_twain      | fgrep_default   |               0 |               5 |               1 |         0.40442 |    741225.00000 |            1.35 |
Complete.
```
**MacOS**

Below benchmark results are collected with 
* GNU grep 3.1
* ag 2.2.0
* rg 0.10.0
* fgrep master branch.

``` shell
./all_tests
Celero
Timer resolution: 0.001000 us
-----------------------------------------------------------------------------------------------------------------------------------------------
     Group      |   Experiment    |   Prob. Space   |     Samples     |   Iterations    |    Baseline     |  us/Iteration   | Iterations/sec  |
-----------------------------------------------------------------------------------------------------------------------------------------------
mark_twain      | grep_brew       |               0 |               5 |               1 |         1.00000 |   1290531.00000 |            0.77 |
mark_twain      | ag              |               0 |               5 |               1 |         1.69616 |   2188950.00000 |            0.46 |
mark_twain      | ripgrep         |               0 |               5 |               1 |         0.56342 |    727106.00000 |            1.38 |
mark_twain      | fgrep_mmap      |               0 |               5 |               1 |         0.48225 |    622353.00000 |            1.61 |
mark_twain      | fgrep_stream    |               0 |               5 |               1 |         0.48798 |    629750.00000 |            1.59 |
mark_twain      | fgrep_default   |               0 |               5 |               1 |         0.48698 |    628463.00000 |            1.59 |
Complete.
```

# FAQs #

## Does fastgrep try to compete with grep and/or ripgrep? ##

No. I think grep or ripgrep are feature complete and it is impossible to keep up with these commands in term of usability. fastgrep written as a library so I can use it in other projects. I only implemented some core features and it takes a lot of time and effort to create something similar to grep or ripgrep.

## Can I contribute to fastgrep? ##

Any contribution is very welcome. I use [google coding standard](https://google.github.io/styleguide/cppguide.html) for all fastgrep related repositories.
