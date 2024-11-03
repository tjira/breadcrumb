#include <argparse.hpp>
#include      <gmpxx.h>

mpz_class base_to_decimal(const std::string& number, mpz_class base, const std::string& character_map) {
    mpz_class converted_number, power_container;

    for (int i = (int)number.size() - 1, j = 0; i >= 0; i--, j++) {

        mpz_pow_ui(power_container.get_mpz_t(), base.get_mpz_t(), j);

        size_t index = character_map.find(number.at(i));

        if (index > base - 1) throw std::runtime_error("THE NUMBER '" + number + "' IS DEFINITELY NOT IN BASE '" + base.get_str() + "' WITH '" + character_map + "' CHARACTER MAP");

        converted_number += character_map.find(number.at(i)) * power_container;
    }

    return converted_number;
}

std::string decimal_to_base(mpz_class number, unsigned int base, const std::string& character_map) {
    std::string converted_number;

    while (number != 0) {
        converted_number = character_map.at(mpz_class(number % base).get_ui()) + converted_number, number /= base;
    }

    return converted_number;
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Bcon", "1.0", argparse::default_arguments::none);

    program.add_argument("-i", "--input").help("-- Input character map."             ).default_value("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    program.add_argument("-o", "--output").help("-- Output character map."           ).default_value("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    program.add_argument("-f", "--from").help("-- Base of the probided number."      ).default_value(10U).scan<'i', unsigned int>();
    program.add_argument("-t", "--to").help("-- Base of the number after conversion.").default_value(2U).scan<'i', unsigned int>();
    program.add_argument("-n", "--number").help("-- Number you want to convert."     ).required();
    program.add_argument("-h", "--help"  ).help("-- Show this help message."         ).default_value(false).implicit_value(true);

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    mpz_class number_decimal = base_to_decimal(program.get("--number"), program.get<unsigned int>("--from"), program.get("--input") );
    std::string number_base  = decimal_to_base(number_decimal,          program.get<unsigned int>("--to"),   program.get("--output"));

    std::cout << number_base << std::endl;
}
