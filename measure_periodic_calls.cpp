#include "performance.hpp"
#include "dlog.hpp"

#include <cmath>
#include <fstream>
#include <cstdio>

#include <unistd.h>

typedef dlog::logger<dlog::formatter> logger;

template <class Fun>
void measure(Fun fun, char const* timings_file_name)
{
    performance::logger<8192, performance::rdtscp_cpuid_clock, std::uint32_t> performance_log;

    for(int i=0; i!=6000; ++i) {
        usleep(1000);
        auto start = performance_log.start();
        fun("Hello, world!", 'A', i, M_PI);
        performance_log.end(start);
    }

    std::ofstream timings(timings_file_name);
    for(auto sample : performance_log) {
        timings << sample << std::endl;
    }
}

int main()
{
    unlink("fstream.txt");
    unlink("stdio.txt");
    unlink("alog.txt");
    performance::rdtscp_cpuid_clock::bind_cpu(0);

    std::ofstream ofs("fstream.txt");
    measure([&](char const* s, char c, int i, double d)
        {
            ofs << "string: " << s << " char: " << c << " int: "
                << i << " double: " << d << std::endl;
       }, "timings_periodic_calls_fstream.txt");
    ofs.close();

    FILE* stdio_file = std::fopen("stdio.txt", "w");
    measure([&](char const* s, char c, int i, double d)
        {
            fprintf(stdio_file, "string: %s char: %c int: %d double: %f\n",
                s, c, i, d);
            fflush(stdio_file);
       }, "timings_periodic_calls_stdio.txt");
    std::fclose(stdio_file);

    measure([&](char const* s, char c, int i, double d) { },
            "timings_periodic_calls_nop.txt");

    dlog::file_writer writer("alog.txt");
    dlog::initialize(&writer);
    measure([](char const* s, char c, int i, double d)
        {
            logger::write("string: %s char: %s int: %d double: %d\n",
                s, c, i, d);
            dlog::flush();
        }, "timings_periodic_calls_alog.txt");
    dlog::cleanup();

    performance::rdtscp_cpuid_clock::unbind_cpu();
    return 0;
}
