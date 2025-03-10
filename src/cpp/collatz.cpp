#include <argparse.hpp>
#include      <gmpxx.h>

mpz_class f(const mpz_class& n) {
    return n % mpz_class(2) == 0 ? mpz_class(n / 2) : 3 * n + 1;
}

std::vector<mpz_class> get_series(mpz_class n) {
    std::vector<mpz_class> series = {n}; while (n != 1) n = f(n), series.push_back(n); return series;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Mersenne", "1.0", argparse::default_arguments::none);

    program.add_argument("-l", "--length").help("-- Calculate the length of the series for the given number."   );
    program.add_argument("-m", "--maximum").help("-- Print the highest number in a series for the given number.");
    program.add_argument("-s", "--series").help("-- Print the series for the given number."                     );
    program.add_argument("-h", "--help"  ).help("-- Show this help message."                                    ).default_value(false).implicit_value(true);

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    std::vector<mpz_class> series;

    if (program.is_used("-l")) series = get_series(mpz_class(program.get("-l")));
    if (program.is_used("-m")) series = get_series(mpz_class(program.get("-m")));
    if (program.is_used("-s")) series = get_series(mpz_class(program.get("-s")));


    if (program.is_used("-l")) std::cout << series.size() << std::endl;
    if (program.is_used("-m")) std::cout << *std::max_element(series.begin(), series.end()) << std::endl;
    if (program.is_used("-s")) for (const mpz_class& i : series) std::cout << i << std::endl;
}
