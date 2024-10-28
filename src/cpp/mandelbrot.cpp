#define STB_IMAGE_WRITE_IMPLEMENTATION

#include      <argparse.hpp>
#include          <mpreal.h>
#include <stb_image_write.h>
#include            <random>

unsigned int nthread = 1;

struct Image {
    std::vector<unsigned char> data; unsigned int width, height;
};

struct Density {
    double bailout_radius; unsigned int max_iterations, samples, seed; bool enable_smooth;
};

struct Escape {
    double bailout_radius; unsigned int max_iterations; bool enable_smooth;
};

struct Trap {
    double bailout_radius; unsigned int max_iterations, trap_index; bool fill_background;
};

template <typename T>
void draw_density(Image& image, const std::complex<T>& center, T zoom, const Density& density_algorithm) {
    std::uniform_real_distribution<double> uniform_real(-3.5, 2.0), uniform_imag(-1.9, 1.9); std::mt19937 mt(density_algorithm.seed);
    std::vector<unsigned int> data(image.width * image.height, 0); std::vector<std::vector<unsigned int>> thread_data(nthread, data);

    std::vector<std::complex<T>> ps(density_algorithm.samples); std::generate(ps.begin(), ps.end(), [&]() {return std::complex<double>(uniform_real(mt), uniform_imag(mt));});

    #pragma omp parallel for num_threads(nthread)
    for (const std::complex<T>& p : ps) {

        std::complex<T> z = T(0); std::vector<std::complex<T>> orbit;

        for(int n = 0; n < density_algorithm.max_iterations; n++) {
            z = z * z + p; if (std::norm(z) > density_algorithm.bailout_radius * density_algorithm.bailout_radius) break; orbit.push_back(z);
        }

        if (std::norm(z) > density_algorithm.bailout_radius * density_algorithm.bailout_radius) for (const std::complex<double>& o : orbit) {

            int i = std::round(((o.imag() + center.imag()) * image.height * zoom + 1.5 * image.height) / 3.0 - 0.5);
            int j = std::round(((o.real() - center.real()) * image.height * zoom + 1.5 * image.width)  / 3.0 - 0.5);

            if (i < 0 || j < 0 || i >= image.height || j >= image.width) continue; thread_data.at(0).at(i * image.width + j)++;
        }
    }

    for (int i = 0; i < nthread; i++) for (size_t j = 0; j < data.size(); j++) data.at(j) += thread_data.at(i).at(j);

    unsigned int max = *std::max_element(data.begin(), data.end());

    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {
        image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)(255.0 * data.at(i * image.width + j) / max);
        image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)(255.0 * data.at(i * image.width + j) / max);
        image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)(255.0 * data.at(i * image.width + j) / max);
    }
}

template <typename T>
void draw_escape(Image& image, const std::complex<T>& center, T zoom, const Escape& escape_algorithm) {
    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {

        T im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
        T re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

        std::complex<T> p = {re, im}, z = T(0); int n; T v;

        for (n = 0; n < escape_algorithm.max_iterations; n++) {
            z = z * z + p; if (std::norm(z) > escape_algorithm.bailout_radius * escape_algorithm.bailout_radius) break;
        }

        if (v = n; n < escape_algorithm.max_iterations && escape_algorithm.enable_smooth) v -= log2(0.5 * log(std::norm(z)));

        if (n < escape_algorithm.max_iterations) {
            image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(31.93 * v / escape_algorithm.max_iterations + 6.26) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(30.38 * v / escape_algorithm.max_iterations + 5.86) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(11.08 * v / escape_algorithm.max_iterations + 0.80) + 1) * 127.5);
        }
    }
}

template <typename T>
void draw_trap(Image& image, const std::complex<T>& center, T zoom, const Trap& trap_algorithm) {
    static std::vector<std::function<T(const std::complex<T>&)>> traps {
        [](const std::complex<T>& p) -> T {return std::norm(p);},
        [](const std::complex<T>& p) -> T {return std::min(abs(p.real()), abs(p.imag()));},
        [](const std::complex<T>& p) -> T {return std::min(abs(p.real() - p.imag()), abs(p.real() + p.imag())) / sqrt(2);},
        [](const std::complex<T>& p) -> T {return abs(std::norm(p) - 1.0);},
        [](const std::complex<T>& p) -> T {return std::min(std::norm(p), abs(std::norm(p) - 1.0));}
    }; auto trap_function = traps.at(trap_algorithm.trap_index);

    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {

        T im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
        T re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

        std::complex<T> p = {re, im}, z = T(0); std::vector<std::complex<T>> orbit;

        for(int n = 0; n < trap_algorithm.max_iterations; n++) {
            z = z * z + p; if (std::norm(z) > trap_algorithm.bailout_radius * trap_algorithm.bailout_radius) break; orbit.push_back(z);
        }

        if (!trap_algorithm.fill_background || std::norm(z) > trap_algorithm.bailout_radius * trap_algorithm.bailout_radius) {

            T v = trap_function(*std::min_element(orbit.begin(), orbit.end(), [&](const std::complex<T>& a, const std::complex<T>& b) {return trap_function(a) < trap_function(b);}));
            
            image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(31.93 * 0.03 * log(v) + 6.26) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(30.38 * 0.03 * log(v) + 5.86) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(11.08 * 0.03 * log(v) + 0.80) + 1) * 127.5);
        }
    }
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Mandelbrot", "1.0", argparse::default_arguments::none);

    program.add_argument("-c", "--center").help("-- Center of the image.").nargs(2).default_value(std::vector<std::string>{"-0.75", "0"});
    program.add_argument("-d", "--density").help("-- Density algorithm with number of iterations, bailout radius, number of samples and seed.").nargs(4).default_value(std::vector<std::string>{"80", "10", "1e7", "1"});
    program.add_argument("-e", "--escape").help("-- Escape algorithm with number of iterations, bailout radius and smooth boolean.").nargs(3).default_value(std::vector<std::string>{"80", "10", "1"});
    program.add_argument("-h", "--help").help("-- This help message.").default_value(false).implicit_value(true);
    program.add_argument("-m", "--mpfr").help("-- Number of bits to represent a MPFR real number.").default_value(64U).scan<'i', unsigned int>();
    program.add_argument("-n", "--nthread").help("-- Number of threads to use.").default_value(1U).scan<'i', unsigned int>();
    program.add_argument("-o", "--output").help("-- Output filename.").default_value("fractal.png");
    program.add_argument("-r", "--resolution").help("-- Resolution of the image.").nargs(2).default_value(std::vector<unsigned int>{1920U, 1080U}).scan<'i', unsigned int>();
    program.add_argument("-t", "--trap").help("-- Trap algorithm with number of iterations, bailout radius, trap index and fill boolean.").nargs(4).default_value(std::vector<std::string>{"80", "100", "2", "0"});
    program.add_argument("-z", "--zoom").help("-- Zoom of the image.").default_value("1.1");

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    if (program.is_used("--escape") + program.is_used("--trap") + program.is_used("--density") > 1) {
        throw std::runtime_error("YOU CAN USE ONLY ONE ALGORITHM AT A TIME");
    }

    Image image; Density density_algorithm; Escape escape_algorithm; Trap trap_algorithm; std::complex<mpfr::mpreal> center; nthread = program.get<unsigned int>("--nthread");

    image.width  = program.get<std::vector<unsigned int>>("--resolution").at(0 );
    image.height = program.get<std::vector<unsigned int>>("--resolution").at(1 );
    image.data   = std::vector<unsigned char>(3 * image.width * image.height, 0);

    density_algorithm.max_iterations = std::stoi(program.get<std::vector<std::string>>("--density").at(0));
    density_algorithm.bailout_radius = std::stod(program.get<std::vector<std::string>>("--density").at(1));
    density_algorithm.samples        = std::stod(program.get<std::vector<std::string>>("--density").at(2));
    density_algorithm.seed           = std::stoi(program.get<std::vector<std::string>>("--density").at(3));

    escape_algorithm.max_iterations = std::stoi(program.get<std::vector<std::string>>("--escape").at(0));
    escape_algorithm.bailout_radius = std::stod(program.get<std::vector<std::string>>("--escape").at(1));
    escape_algorithm.enable_smooth  = std::stoi(program.get<std::vector<std::string>>("--escape").at(2));

    trap_algorithm.max_iterations  = std::stoi(program.get<std::vector<std::string>>("--trap").at(0));
    trap_algorithm.bailout_radius  = std::stod(program.get<std::vector<std::string>>("--trap").at(1));
    trap_algorithm.trap_index      = std::stoi(program.get<std::vector<std::string>>("--trap").at(2));
    trap_algorithm.fill_background = std::stoi(program.get<std::vector<std::string>>("--trap").at(3));

    center.real(mpfr::mpreal(program.get<std::vector<std::string>>("--center").at(0)));
    center.imag(mpfr::mpreal(program.get<std::vector<std::string>>("--center").at(1)));

    #pragma omp parallel for num_threads(nthread)
    for (int i = 0; i < nthread; i++) mpfr::mpreal::set_default_prec(program.get<unsigned int>("--mpfr"));

    if (program.is_used("--density")) {
        if (program.is_used("--mpfr")) {
            // draw_density<mpfr::mpreal>(image, center, program.get("--zoom"), density_algorithm);
        } else {
            draw_density<double>(image, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), density_algorithm);
        }
    } else if (program.is_used("--trap")) {
        if (program.is_used("--mpfr")) {
            draw_trap<mpfr::mpreal>(image, center, program.get("--zoom"), trap_algorithm);
        } else {
            draw_trap<double>(image, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), trap_algorithm);
        }
    } else {
        if (program.is_used("--mpfr")) {
            draw_escape<mpfr::mpreal>(image, center, program.get("--zoom"), escape_algorithm);
        } else {
            draw_escape<double>(image, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), escape_algorithm);
        }
    }

    stbi_write_png(program.get("--output").c_str(), image.width, image.height, 3, image.data.data(), 3 * image.width);
}
