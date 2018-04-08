class: center, middle
# How to write a fast grep like command using modern C++

---

# Why?

* I frequently have to dive into our weblog files, which have more than 1 billions log messages per day, to find out issues with our asynchronous distributed task execution system.

* I want to build a fast system that can

	* Automatically identify issues within a given time period.

	* Track/check/validate the life cycle of messages and tasks.

	* Allow users to find their desired log message quickly.

---

# Goals

* Create an usable grep command that can be as fast as grep, ripgrep, and ag.

* Have reusable libraries that can be used in other projects.

---

# What are requirements for writing a grep like command?

* A file reading algorithm.

* Pattern matching algorithms.

* A user friendly command line interface.

---

class: center, middle

# How to write a fast file reading algorithms

---

# A simple C++ solution

``` c++
  template <typename Container> Container read_iostream(const std::string &afile) {
  std::ifstream t(afile);
  Container str;

  t.seekg(0, std::ios::end);
  str.reserve(t.tellg());
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)),
             std::istreambuf_iterator<char>());
  return str;
}
```

---

# A memory mapped solution

``` c++
    template <typename Container> Container read_memmap(const std::string &afile) {
        boost::iostreams::mapped_file mmap(afile, boost::iostreams::mapped_file::readonly);
        auto begin = mmap.const_data();
        auto end = begin + mmap.size();
        return Container(begin, end);
    }
```
---

# A solution that use low level I/O APIs

``` c++
    template <typename Container>
    void read(const char *afile, Container &buffer, char *buf, const size_t buffer_size) {
        int fd = ::open(afile, O_RDONLY);
        if (fd < 0) {
            fmt::MemoryWriter writer;
            writer << "Cannot open file \"" << afile << "\"";
            throw(std::runtime_error(writer.str()));
        }
        struct stat file_stat;
        if (fstat(fd, &file_stat) < 0) return;
        buffer.reserve(file_stat.st_size);
        while (true) {
            auto nbytes = ::read(fd, buf, buffer_size);
            if (nbytes < 0) {
                fmt::MemoryWriter writer;
                writer << "Cannot read file \"" << afile << "\"";
                throw(std::runtime_error(writer.str()));
            };
            buffer.append(buf, nbytes);
            if (nbytes != static_cast<decltype(nbytes)>(buffer_size)) {
                break;
            };
        }
        ::close(fd);
    }
```
---
# Benchmark results

``` text
Celero
Timer resolution: 0.001000 us
-----------------------------------------------------------------------------------------------------------------------------------------------
     Group      |   Experiment    |   Prob. Space   |     Samples     |   Iterations    |    Baseline     |  us/Iteration   | Iterations/sec  |
-----------------------------------------------------------------------------------------------------------------------------------------------
read            | iostream        |               0 |              10 |               1 |         1.00000 |      9185.00000 |          108.87 |
read            | boost_memmap    |               0 |              10 |               1 |         0.04725 |       434.00000 |         2304.15 |
read            | read_2_10       |               0 |              10 |               1 |         0.10659 |       979.00000 |         1021.45 |
read            | read_2_12       |               0 |              10 |               1 |         0.04377 |       402.00000 |         2487.56 |
read            | read_2_13       |               0 |              10 |               1 |         0.04268 |       392.00000 |         2551.02 |
read            | read_2_14       |               0 |              10 |               1 |         0.03201 |       294.00000 |         3401.36 |
read            | read_2_15       |               0 |              10 |               1 |         0.03125 |       287.00000 |         3484.32 |
read            | read_2_16       |               0 |              10 |               1 |         0.03081 |       283.00000 |         3533.57 |
read            | read_2_17       |               0 |              10 |               1 |         0.02961 |       272.00000 |         3676.47 |
read            | read_2_18       |               0 |              10 |               1 |         0.02994 |       275.00000 |         3636.36 |
read            | read_2_19       |               0 |              10 |               1 |         0.03081 |       283.00000 |         3533.57 |
read            | read_2_20       |               0 |              10 |               1 |         0.03125 |       287.00000 |         3484.32 |
Complete.
```

---
# Summary

* Our simple benchmark shown that the third solution is the winner and the optimum buffer size is around 64KBytes. We will use this value a default value for our buffer size.

* The memory mapped solution has a very good performance.

* The first solution is 20x slower than that of the memory mapped solution. We should not use it in serious applications.

* The policy based design approach help to create a generic, flexible, and fast file reading algorithm.

---
# Our final file reading algorithm

``` c++
    template <size_t BUFFER_SIZE, typename Parser> class FileReader {
      public:
        void operator()(const char *datafile, Parser &parser, const long offset = 0) {
            char read_buffer[BUFFER_SIZE + 1];
            int fd = ::open(datafile, O_RDONLY);
			// Let the kernel know that we are going to read sequentially to the end of a file.
			posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
            while (true) {
                auto nbytes = ::read(fd, read_buffer, BUFFER_SIZE);
                if (nbytes < 0) {
                    std::stringstream writer;
                    writer << "Cannot read file \"" << datafile << "\"";
                    throw(std::runtime_error(writer.str()));
                };
                parser(read_buffer, read_buffer + nbytes); // Read buffer is processed using a templatized policy.
                if (nbytes != static_cast<decltype(nbytes)>(BUFFER_SIZE)) { break; };
            }
            ::close(fd);
        }
    };
```

---
# Is our file reading algorithm fast?

To show that our file reading algorithm is fast enough we will create a simple command which is similar to "wc -l" command and benchmark it with a reasonable big text file.

* Compute file size.

* Count the number of lines.

* Compute the maximum and minimum length of lines.

---
# A line counting filter.
``` c++
class LineStats {
public:
  void operator()(const char *begin, const char *end) {
    const char *ptr = begin;
    while (
        (ptr = static_cast<const char *>(memchr_avx2(ptr, EOL, end - ptr)))) {
      ++lines;
      const size_t new_eol = file_size + ptr - begin;
      const size_t len = new_eol - current_eol - 1;
      max_len = len > max_len ? len : max_len;
      min_len = len < min_len ? len : min_len;
      current_eol = new_eol;
      ++ptr;
    }
    file_size += end - begin;
  }
  size_t file_size = 0;
  size_t lines = 0;
  size_t max_len = std::numeric_limits<size_t>::min();
  size_t min_len = std::numeric_limits<size_t>::max();
  size_t current_eol = 0;

private:
  static constexpr char EOL = '\n';
};
```
---
# Benchmark results for wc -l

``` text
 Performance counter stats for 'wc -l /mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-03_00000' (5 runs):

       4300.374643 task-clock                #    0.999 CPUs utilized            ( +-  0.03% )
                11 context-switches          #    0.003 K/sec                    ( +-  7.68% )
                 0 cpu-migrations            #    0.000 K/sec                    ( +- 61.24% )
               170 page-faults               #    0.040 K/sec
     9,469,917,841 cycles                    #    2.202 GHz                      ( +-  0.03% )
   <not supported> stalled-cycles-frontend
   <not supported> stalled-cycles-backend
     3,910,542,147 instructions              #    0.41  insns per cycle          ( +-  0.02% )
       907,972,719 branches                  #  211.138 M/sec                    ( +-  0.02% )
        21,540,040 branch-misses             #    2.37% of all branches          ( +-  0.21% )

       4.305678789 seconds time elapsed                                          ( +-  0.03% )
```
---
# Benchmark results for linestats

``` text
 Performance counter stats for './linestats /mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-03_00000' (5 runs):

       2690.545592 task-clock                #    0.998 CPUs utilized            ( +-  0.14% )
                22 context-switches          #    0.008 K/sec                    ( +-  7.31% )
                 1 cpu-migrations            #    0.000 K/sec                    ( +- 31.18% )
               361 page-faults               #    0.134 K/sec                    ( +-  0.06% )
     5,924,807,135 cycles                    #    2.202 GHz                      ( +-  0.14% )
   <not supported> stalled-cycles-frontend
   <not supported> stalled-cycles-backend
     2,290,821,363 instructions              #    0.39  insns per cycle          ( +-  0.00% )
       474,603,918 branches                  #  176.397 M/sec                    ( +-  0.00% )
         9,564,301 branch-misses             #    2.02% of all branches          ( +-  0.92% )

       2.695044748 seconds time elapsed                                          ( +-  0.14% )
```

---
# What have we done so far?

* We have created a generic file reading algorithm which might be one of the fastest available solution. See this [link](https://lemire.me/blog/2012/06/26/which-is-fastest-read-fread-ifstream-or-mmap/ "Lemire's blog") for more information. Note that we reuse our algorithm easily for different purposes. 

* Our benchmark results have shown that **linestats** command is about 50% faster than **wc -l**. The performance gain comes from below facts

	* Tuning: We choose the best value for our buffer from the file reading benchmark.

	* Inlining: The ability to inline functions at compile in C++ does contribute to the performance gain since our approach and the algorithm used in wc are mostly the same.

---
class: center, middle
# How to write a fast string search algorithm?

---
# First version of [fastgrep](https://github.com/hungptit/scribe_parser "A very fast grep like command")

We have already had a fast file reading algorithm and to be able to create our first working version of fastgrep command we need below classes

* MessageFilter: This class is called within the file reading algorithm. It will take a string buffer break it into lines then display lines that match a given string pattern.

* utils::Contains: This class check that a pattern is matched with a given text line.

---
# A message filter class

``` c++
    template <typename Constraints> class MessageFilter {
      public:
        ~MessageFilter() {
            if (!buffer.empty()) process();
        }
        void operator()(const char *begin, const char *end) {
            const char *start = begin;
            const char *ptr = begin;
            while ((ptr = static_cast<const char *>(memchr_avx2(ptr, EOL, end - ptr)))) {
                buffer.append(start, ptr - start + 1);
                ++lines;
                process();
                start = ++ptr;
                if (start == end) break;
            }
            if (start != end) { buffer.append(start, end - start); }
        }

      private:
        std::string buffer;
        size_t lines;
        Constraints constraints;
        static constexpr char EOL = '\n';
        void process() {
            if (constraints(buffer)) fmt::print("{}", buffer.data());
            buffer.clear(); // Reset the buffer.
        }
    };
```
---
# An exact pattern matching class

``` c++
    namespace baseline {
        // Search for a sub string.
        class Contains {
          public:
            explicit Contains(const std::string &patt) : pattern(patt) {}
            bool operator()(const std::string &line) {
                if (line.size() < pattern.size()) { return false; }
                return line.find(pattern) != std::string::npos;
            }

          private:
            const std::string pattern;
        };
    } // namespace baseline
```
---
class: center, middle
# Benchmark results

---
# grep
``` text
        Command being timed: "grep Starting1 /mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-03_00000"
        User time (seconds): 5.27
        System time (seconds): 1.20
        Percent of CPU this job got: 99%
        Elapsed (wall clock) time (h:mm:ss or m:ss): 0:06.48
        Average shared text size (kbytes): 0
        Average unshared data size (kbytes): 0
        Average stack size (kbytes): 0
        Average total size (kbytes): 0
        Maximum resident set size (kbytes): 1924
        Average resident set size (kbytes): 0
        Major (requiring I/O) page faults: 0
        Minor (reclaiming a frame) page faults: 743
        Voluntary context switches: 2
        Involuntary context switches: 13
        Swaps: 0
        File system inputs: 0
        File system outputs: 0
        Socket messages sent: 0
        Socket messages received: 0
        Signals delivered: 0
        Page size (bytes): 4096
        Exit status: 1
```
---
# fastgrep
``` text
        Command being timed: "fastgrep --no-regex -p Starting1 /mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-03_00000"
        User time (seconds): 42.98
        System time (seconds): 1.41
        Percent of CPU this job got: 99%
        Elapsed (wall clock) time (h:mm:ss or m:ss): 0:44.45
        Average shared text size (kbytes): 0
        Average unshared data size (kbytes): 0
        Average stack size (kbytes): 0
        Average total size (kbytes): 0
        Maximum resident set size (kbytes): 3048
        Average resident set size (kbytes): 0
        Major (requiring I/O) page faults: 0
        Minor (reclaiming a frame) page faults: 929
        Voluntary context switches: 11
        Involuntary context switches: 78
        Swaps: 0
        File system inputs: 0
        File system outputs: 0
        Socket messages sent: 0
        Socket messages received: 0
        Signals delivered: 0
        Page size (bytes): 4096
```

---
# Why our fastgrep command is very slow?

Benchmark results have shown that our fastgrep command is about 7x slower than GNU grep. Our profiling results using perf command show that fastgrep has spent a large portion of time on std::string::find command. It is obvious that we need a better string find algorithm.

``` text
# Overhead         Command        Shared Object
# ........  ..............  ...................  ..............................................................................................
#
    80.09%  message_filter  libstdc++.so.6.0.21  [.] _ZNKSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE4findEPKcmm
    10.11%  message_filter  libc-2.23.so         [.] __memcmp_sse4_1
     2.86%  message_filter  message_filter       [.] _ZN6scribe10FileReaderILm65536ENS_13MessageFilterINS_17SimpleConstraintsIN5utils8baseline8
     1.82%  message_filter  [kernel.kallsyms]    [k] copy_user_enhanced_fast_string
     0.94%  message_filter  libc-2.23.so         [.] __memcpy_avx_unaligned
     0.38%  message_filter  libstdc++.so.6.0.21  [.] _ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEE9_M_appendEPKcm
     0.32%  message_filter  [kernel.kallsyms]    [k] radix_tree_lookup_slot
     0.22%  message_filter  [kernel.kallsyms]    [k] ii_iovec_copy_to_user
     0.19%  message_filter  libstdc++.so.6.0.21  [.] memcmp@plt
     0.16%  message_filter  [kernel.kallsyms]    [k] do_generic_file_read.clone.0
     0.15%  message_filter  ld-2.23.so           [.] _dl_lookup_symbol_x
     0.13%  message_filter  [kernel.kallsyms]    [k] find_get_page
     0.13%  message_filter  [kernel.kallsyms]    [k] put_page
```
---
# How can we improve the performance of std::string::find?

![alt text](pictures/pic1.png)

---

# SSE2 version of std::string::find

``` c++
size_t sse2_strstr_anysize(const char *s, size_t n, const char *needle,
                           size_t k) {
  const __m128i first = _mm_set1_epi8(needle[0]);
  const __m128i last = _mm_set1_epi8(needle[k - 1]);
  for (size_t i = 0; i < n; i += 16) {
    const __m128i block_first =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(s + i));
    const __m128i block_last =
        _mm_loadu_si128(reinterpret_cast<const __m128i *>(s + i + k - 1));
    const __m128i eq_first = _mm_cmpeq_epi8(first, block_first);
    const __m128i eq_last = _mm_cmpeq_epi8(last, block_last);
    uint16_t mask = _mm_movemask_epi8(_mm_and_si128(eq_first, eq_last));
    while (mask != 0) {
      const auto bitpos = bits::get_first_bit_set(mask);
      if (memcmp(s + i + bitpos + 1, needle + 1, k - 2) == 0) {
        return i + bitpos;
      }
      mask = bits::clear_leftmost_set(mask);
    }
  }
  return std::string::npos;
}
```
---
# AVX2 version of std::string::find

``` c++
size_t FORCE_INLINE avx2_strstr_anysize(const char *s, size_t n,
                                        const char *needle, size_t k) {
  const __m256i first = _mm256_set1_epi8(needle[0]);
  const __m256i last = _mm256_set1_epi8(needle[k - 1]);
  for (size_t i = 0; i < n; i += 32) {
    const __m256i block_first =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s + i));
    const __m256i block_last =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(s + i + k - 1));
    const __m256i eq_first = _mm256_cmpeq_epi8(first, block_first);
    const __m256i eq_last = _mm256_cmpeq_epi8(last, block_last);
    uint32_t mask = _mm256_movemask_epi8(_mm256_and_si256(eq_first, eq_last));
    while (mask != 0) {
      const auto bitpos = bits::get_first_bit_set(mask);
      if (memcmp(s + i + bitpos + 1, needle + 1, k - 2) == 0) {
        return i + bitpos;
      }
      mask = bits::clear_leftmost_set(mask);
    }
  }
  return std::string::npos;
}
```

---
# Micro-benchmark results

``` text
2018-04-04 01:08:09
Running ./string
Run on (88 X 2199.81 MHz CPU s)
CPU Caches:
  L1 Data 32K (x44)
  L1 Instruction 32K (x44)
  L2 Unified 256K (x44)
  L3 Unified 56320K (x2)
--------------------------------------------------------
Benchmark                 Time           CPU Iterations
--------------------------------------------------------
std_string_find        1153 ns       1152 ns     607371
sse2_string_find        170 ns        170 ns    4098694
avx2_string_find        113 ns        113 ns    6204882
```

---
# AVX2 fastgrep benchmark results

``` text
        Command being timed: "fastgrep --no-regex -p Starting1 /mnt/weblogs/scribe/workqueue-execution/workqueue-execution-2018-04-03_00000"
        User time (seconds): 4.74
        System time (seconds): 1.31
        Percent of CPU this job got: 99%
        Elapsed (wall clock) time (h:mm:ss or m:ss): 0:06.07
        Average shared text size (kbytes): 0
        Average unshared data size (kbytes): 0
        Average stack size (kbytes): 0
        Average total size (kbytes): 0
        Maximum resident set size (kbytes): 3048
        Average resident set size (kbytes): 0
        Major (requiring I/O) page faults: 0
        Minor (reclaiming a frame) page faults: 929
        Voluntary context switches: 9
        Involuntary context switches: 11
        Swaps: 0
        File system inputs: 0
        File system outputs: 0
        Socket messages sent: 0
        Socket messages received: 0
        Signals delivered: 0
        Page size (bytes): 4096
        Exit status: 0
```

---
class: center, middle
# Benchmar results in my Macbook Pro

---
# grep

``` text
015249:commands hdang$ /usr/bin/time -lp grep Starting1 ../data/workqueue-execution_current
real         3.27
user         3.20
sys          0.04
   3162112  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
       821  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         2  voluntary context switches
      1470  involuntary context switches
```

---
# Brew grep

``` text
015249:commands hdang$ /usr/bin/time -lp ggrep Starting1 ../data/workqueue-execution_current
real         0.12
user         0.08
sys          0.03
   3264512  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
       813  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         2  voluntary context switches
        44  involuntary context switches
```

---
# ag

``` text
015249:commands hdang$ /usr/bin/time -lp ag Starting1 ../data/workqueue-execution_current
real         0.16
user         0.03
sys          0.12
 190394368  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
     47975  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         9  voluntary context switches
        39  involuntary context switches
```

---
# ripgrep

``` text
015249:commands hdang$ /usr/bin/time -lp rg Starting1 ../data/workqueue-execution_current
real         0.14
user         0.09
sys          0.05
   4915200  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
      1216  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         2  voluntary context switches
        47  involuntary context switches
```

---
# fastgrep

``` text
015249:commands hdang$ /usr/bin/time -lp ./fastgrep --no-regex -p Starting1 ../data/workqueue-execution_current
real         0.07
user         0.02
sys          0.03
   3198976  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
       797  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         2  voluntary context switches
        48  involuntary context switches
```

---
# What have we learned?

* Benchmarking/profiling tools are your friends when debugging/investigating performance issues.

* Optimized string matching algorithms do significantly improve the performance of fastgrep.

* C++ allows us to create reusable, flexible, and high performance algorithms.

---
# Are we done?

* We do need to support regular expression.

* Allow users to filter out log messages using timestamps.

---
# Regular expression

* [Regular Expression Matching Can Be Simple And Fast](https://swtch.com/~rsc/regexp/regexp1.html)

* [Comparison of regex engines.](https://rust-leipzig.github.io/regex/2017/03/28/comparison-of-regex-engines/)

---
# A simple regex matcher using hyperscan

``` c++
class RegexMatcher {
public:
  explicit RegexMatcher(const std::string &patt) {
    pattern = patt;
    hs_compile_error_t *compile_err;
    hs_compile(pattern.c_str(), HS_FLAG_DOTALL, HS_MODE_BLOCK, NULL, &database,
               &compile_err);
    hs_alloc_scratch(database, &scratch);
  }
  bool operator()(const std::string &data) {
    if (data.empty())
      return false;
    char *ptr = const_cast<char *>(pattern.c_str());
    auto errcode = hs_scan(database, data.data(), data.size(), 0, scratch,
                           event_handler, ptr);
    if (errcode == HS_SUCCESS) {
      return false;
    } else if (errcode == HS_SCAN_TERMINATED) {
      return true;
    } else {
      throw std::runtime_error("Unable to scan input buffer");
    }
  }
private:
  hs_database_t *database = NULL;
  hs_scratch_t *scratch = NULL;
  std::string pattern;
};
```

---
class: center, middle
# Benchmark results

---
# fastgrep using regex

``` text
015249:commands hdang$ /usr/bin/time -lp ./fastgrep -p Starting1 ../data/workqueue-execution_current
real         0.07
user         0.03
sys          0.03
   4308992  maximum resident set size
         0  average shared memory size
         0  average unshared data size
         0  average unshared stack size
      1068  page reclaims
         0  page faults
         0  swaps
         0  block input operations
         0  block output operations
         0  messages sent
         0  messages received
         0  signals received
         2  voluntary context switches
        34  involuntary context switches
```

---
class: center, middle
# Demo

---
# Summary

* fastgrep's raw performance is as good as the best grep like commands i.e GNU grep and [ripgrep][ripgrep].

* The standard C++ functions for string handling are inefficient i.e std::find, std::ifstream.

* Our benchmark results show that our fastgrep command is at least as fast as grep and best grep like command such as ag and ripgrep.

* Creating efficient solutions using C++ is not a trivial task.

	* A bad C++ code might be 10x slower than using other a similar code using other compiled languages such as C, Rust, or Go.

	* A good C++ code will be the fastest solution with very high reusability.

---

[ripgrep]: https://github.com/BurntSushi/ripgrep "ripgrep"
# Todo list

* Improve the usability of fastgrep command.

* Need a detail benchmark.

* Update the build system so users can build it with minimum amount of work.

---
class: center, middle
# Q/A

---
# Acknowledgement

* SSE2/AVX2 code is the modified version of [sse4-strstr](https://github.com/WojciechMula/sse4-strstr "sse4-strstr")

* I have learned the idea of a fast file reading algorithm idea from this [blog post](https://lemire.me/blog/2012/06/26/which-is-fastest-read-fread-ifstream-or-mmap/ "Lemire's blog") and [GNU wc](https://www.gnu.org/software/coreutils/manual/html_node/wc-invocation.html "wc") command.

* Below are libraries and tools used in my project:
  * [Catch2](https://github.com/catchorg/Catch2 "Catch2")
  * [hyperscan](https://www.hyperscan.io/ "hyperscan")
  * [utils](https://github.com/hungptit/utils "utils")
  * [ioutils](https://github.com/hungptit/ioutils "A blazing fast file I/O library")
  * [Boost](https://www.boost.org/ "Boost libraries")
  * [fmt](https://github.com/fmtlib/fmt "A modern formatting library")
  * [cereal](https://github.com/USCiLab/cereal "A C++11 library for serialization")
  * [CMake](https://cmake.org/ "CMake")
  * [benchmark](https://github.com/google/benchmark)
  * [Celero](https://github.com/DigitalInBlue/Celero)
