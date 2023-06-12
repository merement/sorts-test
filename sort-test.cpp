#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <functional>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "sort_functions_test.hpp"

using std::cout;
using std::vector;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

// minimal size of sorted arrays
const auto SORT_MIN = 3;
// maximal size of sorted arrays
const auto SORT_MAX = 8;
// maximal size of samples for timing
const std::size_t MAX_SAMPLE = 5000000;

const std::array alpha_function{Sort3AlphaDev, Sort4AlphaDev, Sort5AlphaDev,
                          Sort6AlphaDev, Sort7AlphaDev, Sort8AlphaDev};

enum Engine {
    STD,
    ALPHA,
};

void print_usage()
{
    cout << "Compare int sorting algorithms\n";
    cout << "Usage:\n";
    cout << "./sort-test (--std | --alpha) [number_of_samples <= 5e6 | 3-8 space separated ints | --group 3-8 for standard input]" << std::endl;
}

template <class T>
std::optional<T> attempt_to_read(const std::string &input)
{
    try {
        if constexpr (std::is_same_v<T, int>) {
            return std::stoi(input);
        }
        else if constexpr (std::is_same_v<T, std::size_t>) {
            return static_cast<std::size_t>(std::stol(input));
        }
    }
    catch (const std::invalid_argument &_) {
        return {};
    }
}

int pass_err(std::string msg)
{
    std::cerr << "ERROR! " << msg << std::endl;
    print_usage();
    return 1;
}

void print_ar(auto &ar, std::string_view prefix)
{
    cout << prefix;
    for (const auto i : ar) {
        cout << "  " << i;
    }
    cout << std::endl;
}

int process_std(vector<int> &collection, std::size_t num_samples, int sort_size)
{
    auto t_start = high_resolution_clock::now();
    auto start_offset = collection.begin();
    for (std::size_t i = 0; i < num_samples; ++i) {
        std::sort(start_offset, start_offset + sort_size);
        start_offset += sort_size;
    }
    auto t_end = high_resolution_clock::now();
    auto t_interval = duration_cast<milliseconds>(t_end - t_start);
    return t_interval.count();
}

int process_alpha(vector<int> &collection, std::size_t num_samples, int sort_size)
{
    auto t_start = high_resolution_clock::now();
    auto start_offset = collection.data();
    for (std::size_t i = 0; i < num_samples; ++i) {
        alpha_function[sort_size - SORT_MIN](start_offset);
        start_offset += sort_size;
    }
    auto t_end = high_resolution_clock::now();
    auto t_interval = duration_cast<milliseconds>(t_end - t_start);
    return t_interval.count();
}

void process_sample(std::size_t num_samples, Engine engine)
{
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, 1000); // define the range

    for (int sort_size = SORT_MIN; sort_size <= SORT_MAX; ++sort_size)
    {
        auto num_ints = sort_size*num_samples;
        vector<int> collection(num_ints);
        for (std::size_t i = 0; i < num_ints; ++i) {
            collection[i] = distr(gen);
        }
        
        switch (engine) {
        case Engine::ALPHA:
            cout << "AlphaDev:" << sort_size << " sort: ";
            cout << process_alpha(collection, num_samples, sort_size)
                 << " ms" << std::endl;
            break;
            
        case Engine::STD:
            cout << "STD:" << sort_size << " sort: ";
            cout << process_std(collection, num_samples, sort_size)
                 << " ms" << std::endl;
            break;
        }
    }
}

int process_cin(std::size_t sort_size, Engine engine)
{
    std::string input;
    vector<int> container;
    int count_sorts = 1;
    while (std::cin >> input) {
        auto parse = attempt_to_read<int>(input);
        if (parse) {
            container.push_back(parse.value());
        }
        else { return pass_err("Expected 'int', received " + input); }

        if (container.size() >= sort_size) {
            switch (engine) {
            case Engine::ALPHA:
              alpha_function[sort_size - SORT_MIN](container.data());
              print_ar(container, std::to_string(count_sorts) + ": ");
              break;

            case Engine::STD:
              std::sort(container.begin(), container.end());
              print_ar(container, std::to_string(count_sorts) + ": ");
              break;
            }
            count_sorts++;
            container.clear();
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const vector<std::string_view> args(argv + 1, argv + argc);
    auto num_opts = static_cast<int>(args.size());
    if (num_opts < 2 || num_opts > SORT_MAX + 1) {
        print_usage();
        cout << "Example (an AlphaDev implementation)\n";
        std::array ar3{4, 13, 2, 3};
        print_ar(ar3, "Pre-sorting state:");
        alpha_function[ar3.size() - SORT_MIN](ar3.data());
        print_ar(ar3, "Post-sorting state:");
        return 0;
    }
    
    Engine engine = Engine::ALPHA;   
    if (args[0] == "--std") { engine = Engine::STD; }
    else if (args[0] == "--alpha") { engine = Engine::ALPHA; }
    else return pass_err("Unknown engine (implemented: --std and --alpha)");

    if (num_opts > SORT_MIN && num_opts <= SORT_MAX + 1)
    {
        vector<int> arr_ext;
        for (auto cursor = 1; cursor < num_opts; ++cursor) {
            std::string str_readout{args[cursor]};
            auto outcome = attempt_to_read<int>(str_readout);
            if (outcome) {
                arr_ext.push_back(outcome.value());
            }
            else {
                return pass_err("Expected 'int' at position "
                                + std::to_string(cursor+1) + ", got "
                                + str_readout + " instead");
            }
        }
        print_ar(arr_ext, "Initial state:");
        alpha_function[arr_ext.size() - 3](arr_ext.data());
        print_ar(arr_ext, "Post AlphaDev-sorted state:");
        return 0;
    }
    else if (num_opts == 2) {
            int cursor = 1;
            std::string str_readout{args[cursor]};
            auto outcome = attempt_to_read<std::size_t>(str_readout);
            if (outcome) {
                process_sample(std::min(MAX_SAMPLE, outcome.value()), engine);
                return 0;
            }
            else {
                return pass_err("Expected 'long' at position 2, got " +
                                std::string{args[cursor]} + " instead");
            }        
    }
    else if (num_opts == 3) {
        if (args[1] == "--group") {
            int cursor = 2;
            std::string str_readout{args[cursor]};
            auto outcome = attempt_to_read<std::size_t>(str_readout);
            if (outcome && outcome.value() >= SORT_MIN &&
                outcome.value() <= SORT_MAX) {
                return process_cin(outcome.value(), engine);
            }
            else {
                return pass_err("Expected '3 <= int <= 8' at position 3, got " +
                                std::string{args[cursor]} + " instead");
            }
        }
        else return pass_err("Unrecognized option " + std::string{args[1]});
    }
}
