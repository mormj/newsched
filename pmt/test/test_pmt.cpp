#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <pmt/pmt.hpp>

using namespace pmt;
using namespace std;

int main(int argc, char* argv[])
{
    auto int_pmt = pmt_scalar<int>(17);
    std::cout << (int_pmt == 17) << std::endl;
    std::cout << (int_pmt != 17) << std::endl;
    auto int_vec_pmt = pmt_vector<int>({4,5,6});
    auto vec_pmt_value = int_vec_pmt.value();
}
