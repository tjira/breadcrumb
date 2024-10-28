#include <argparse.hpp>
#include      <gmpxx.h>
#include       <chrono>

bool is_mersenne(mpz_class p) {
    mpz_class M = (mpz_class(2) << p.get_ui() - 1) - 1, s = 4;

    for (mpz_class i = 0; i < p - 2; i++) s = (s * s - 2) % M;

    return s == 0 || p == 2;
}

bool is_prime(mpz_class n) {
    for (mpz_class i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    } return true;
}

long elapsed(std::chrono::time_point<std::chrono::high_resolution_clock> from) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock().now() - from).count();
}

std::string format_ms(long ms) {
    long h = ms / 3600000, m = ms % 3600000 / 60000, s = ms % 60000 / 1000; ms = ms % 1000;

    std::stringstream ss; ss << std::setfill('0') << std::setw(2) << h <<  ":" << std::setw(2) << m << ":" << std::setw(2) << s << "." << std::setw(3) << ms;

    return ss.str();
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Mersenne", "1.0", argparse::default_arguments::none);

    program.add_argument("-c", "--check"   ).help("-- Check the Mersenne number with this exponent for primality.").scan<'i', int>();
    program.add_argument("-g", "--generate").help("-- Generate the mersenne primes and print the exponents."      ).default_value(false).implicit_value(true);
    program.add_argument("-h", "--help"    ).help("-- Show this help message."                                    ).default_value(false).implicit_value(true);

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    if (program.is_used("--check")) std::cout << std::boolalpha << is_mersenne(mpz_class(program.get<int>("-c"))) << std::endl;

    auto start_time = std::chrono::high_resolution_clock().now();

    if (program.is_used("--generate")) for (mpz_class n = 0, p = 2; p <= 136279841; p += p % 2 + 1) if (is_prime(p) && is_mersenne(p)) {
        std::cout << std::setw(2) << ++n << " (" << format_ms(elapsed(start_time)) << "): " << p.get_str() << std::endl;
    }
}
