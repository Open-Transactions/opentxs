#include <chrono>

auto main(int, char**) -> int
{
    [[maybe_unused]] const auto time = std::chrono::utc_clock::now();

    return 0;
}
