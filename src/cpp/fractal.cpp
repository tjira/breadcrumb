#define STB_IMAGE_WRITE_IMPLEMENTATION

#include      <argparse.hpp>
#include          <mpreal.h>
#include <stb_image_write.h>
#include            <random>

unsigned int nthread = 1;

template <typename T>
struct FractalFunctions {
    std::unordered_map<std::string, std::function<std::tuple<std::complex<T>, std::complex<T>>(const std::complex<T>&, const std::complex<T>&, const std::complex<T>&, const std::complex<T>&)>> content = {
        {"buffalo", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            std::complex<T> abs_z = {abs(z.real()), abs(z.imag())}; return std::make_tuple(abs_z * abs_z - abs_z + p, z);
        }},
        {"burningship", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            std::complex<T> abs_z = {abs(z.real()), abs(z.imag())}; return std::make_tuple(abs_z * abs_z + p, z);
        }},
        {"julia", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            return std::make_tuple(z * z + c, z);
        }},
        {"mandelbrot", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            return std::make_tuple(z * z + p, z);
        }},
        {"manowar", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            return std::make_tuple(z * z + zp + p, z);
        }},
        {"phoenix", [](const std::complex<T>& p, const std::complex<T>& z, const std::complex<T>& zp, const std::complex<T>& c) {
            return std::make_tuple(z * z - T(0.5) * zp + T(0.5667), z);
        }}
    };
};

template <typename T>
struct FractalInitialConditions {
    std::unordered_map<std::string, std::function<std::tuple<std::complex<T>, std::complex<T>, std::complex<T>>(const std::complex<T>&, const std::complex<T>&)>> content = {
        {"buffalo",     [](const std::complex<T>& p, const std::complex<T>& c) {return std::make_tuple(p, T(0), T(0));}},
        {"burningship", [](const std::complex<T>& p, const std::complex<T>& c) {return std::make_tuple(p, T(0), T(0));}},
        {"julia",       [](const std::complex<T>& p, const std::complex<T>& c) {return std::make_tuple(p, p, T(0));}},
        {"mandelbrot",  [](const std::complex<T>& p, const std::complex<T>& c) {return std::make_tuple(p, T(0), T(0));}},
        {"manowar",     [](const std::complex<T>& p, const std::complex<T>& c) {return std::make_tuple(p, p, p);}},
        {"phoenix",     [](const std::complex<T>& p, const std::complex<T>& c) {std::complex t = {-p.imag(), p.real()}; return std::make_tuple(p, t, c);}}
    };
};

template <typename T>
struct TrapFunctions {
    std::vector<std::function<T(const std::complex<T>&)>> content = {
        [](const std::complex<T>& p) {return std::norm(p);},
        [](const std::complex<T>& p) {return std::min(abs(p.real()), abs(p.imag()));},
        [](const std::complex<T>& p) {return std::min(abs(p.real() - p.imag()), abs(p.real() + p.imag())) / sqrt(2);},
        [](const std::complex<T>& p) {return abs(std::norm(p) - 1.0);},
        [](const std::complex<T>& p) {return std::min(std::norm(p), abs(std::norm(p) - 1.0));}
    }; 
};

struct Image {
    std::vector<unsigned char> data; unsigned int width, height;
};

struct Fractal {
    std::string name; double parameter;
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

struct Linear {
    std::array<unsigned char, 3> from, to;
};

struct Periodic {
    std::array<double, 3> amplitude; std::array<double, 3> phase;
};

struct Solid {
    std::array<unsigned char, 3> color;
};

template <typename T, class C>
void draw_density(Image& image, const Fractal& fractal, const std::complex<T>& center, T zoom, const Density& density_algorithm, const C& color_algorithm) {
    auto fractal_function = FractalFunctions<T>().content.at(fractal.name); auto fractal_ic_function = FractalInitialConditions<T>().content.at(fractal.name);

    std::uniform_real_distribution<double> uniform_real(-3.9, 3.9), uniform_imag(-2.5, 2.5); std::mt19937 mt(density_algorithm.seed);
    std::vector<unsigned int> data(image.width * image.height, 0); std::vector<std::vector<unsigned int>> thread_data(nthread, data);

    std::vector<std::complex<T>> ps(density_algorithm.samples); std::generate(ps.begin(), ps.end(), [&]() {return std::complex<double>(uniform_real(mt), uniform_imag(mt));});

    #pragma omp parallel for num_threads(nthread)
    for (const std::complex<T>& p : ps) {

        std::complex<T> parameter = exp(std::complex<T>{0, 1} * T(fractal.parameter)), z, zp;

        std::tie(std::ignore, z, zp) = fractal_ic_function(p, parameter); std::vector<std::complex<T>> orbit;

        for (int n = 0; n < density_algorithm.max_iterations; n++) {
            std::tie(z, zp) = fractal_function(p, z, zp, parameter); if (std::norm(z) > density_algorithm.bailout_radius * density_algorithm.bailout_radius) break; orbit.push_back(z);
        }

        if (std::norm(z) > density_algorithm.bailout_radius * density_algorithm.bailout_radius) for (const std::complex<T>& o : orbit) {

            int i = (int)round(((o.imag() + center.imag()) * image.height * zoom + 1.5 * image.height) / 3.0 - 0.5);
            int j = (int)round(((o.real() - center.real()) * image.height * zoom + 1.5 * image.width)  / 3.0 - 0.5);

            if (i < 0 || j < 0 || i >= image.height || j >= image.width) continue; thread_data.at(0).at(i * image.width + j)++;
        }
    }

    for (int i = 0; i < nthread; i++) for (size_t j = 0; j < data.size(); j++) data.at(j) += thread_data.at(i).at(j);

    unsigned int max = *std::max_element(data.begin(), data.end());

    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {
        if constexpr (std::is_same<C, Linear>()) {
            image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((double)color_algorithm.from.at(0) + (double)data.at(i * image.width + j) / max * (color_algorithm.to.at(0) - color_algorithm.from.at(0)));
            image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((double)color_algorithm.from.at(1) + (double)data.at(i * image.width + j) / max * (color_algorithm.to.at(1) - color_algorithm.from.at(1)));
            image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((double)color_algorithm.from.at(2) + (double)data.at(i * image.width + j) / max * (color_algorithm.to.at(2) - color_algorithm.from.at(2)));
        } else if constexpr (std::is_same<C, Periodic>()) {
            image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(color_algorithm.amplitude.at(0) * (double)data.at(i * image.width + j) / max + color_algorithm.phase.at(0)) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(color_algorithm.amplitude.at(1) * (double)data.at(i * image.width + j) / max + color_algorithm.phase.at(1)) + 1) * 127.5);
            image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(color_algorithm.amplitude.at(2) * (double)data.at(i * image.width + j) / max + color_algorithm.phase.at(2)) + 1) * 127.5);
        } else if constexpr (std::is_same<C, Solid>()) {
            if (data.at(i * image.width + j)) image.data.at(3 * i * image.width + 3 * j + 0) = color_algorithm.color.at(0);
            if (data.at(i * image.width + j)) image.data.at(3 * i * image.width + 3 * j + 1) = color_algorithm.color.at(1);
            if (data.at(i * image.width + j)) image.data.at(3 * i * image.width + 3 * j + 2) = color_algorithm.color.at(2);
        } else throw std::runtime_error("COLORING ALGORITHM FOR THIS GENERATION ALGORITHM NOT SUPPORTED");
    }
}

template <typename T, class C>
void draw_escape(Image& image, const Fractal& fractal, const std::complex<T>& center, T zoom, const Escape& escape_algorithm, const C& color_algorithm) {
    auto fractal_function = FractalFunctions<T>().content.at(fractal.name); auto fractal_ic_function = FractalInitialConditions<T>().content.at(fractal.name);

    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {

        std::complex<T> parameter = exp(std::complex<T>{0, 1} * T(fractal.parameter));

        T im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
        T re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

        auto [p, z, zp] = fractal_ic_function(std::complex<T>{re, im}, parameter); int n; T v;

        for (n = 0; n < escape_algorithm.max_iterations; n++) {
            std::tie(z, zp) = fractal_function(p, z, zp, parameter); if (std::norm(z) > escape_algorithm.bailout_radius * escape_algorithm.bailout_radius) break;
        }

        if (v = n; n < escape_algorithm.max_iterations && escape_algorithm.enable_smooth) v -= log2(0.5 * log(std::norm(z)));

        if (n < escape_algorithm.max_iterations) {
            if constexpr (std::is_same<C, Linear>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((double)color_algorithm.from.at(0) + v / escape_algorithm.max_iterations * (color_algorithm.to.at(0) - color_algorithm.from.at(0)));
                image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((double)color_algorithm.from.at(1) + v / escape_algorithm.max_iterations * (color_algorithm.to.at(1) - color_algorithm.from.at(1)));
                image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((double)color_algorithm.from.at(2) + v / escape_algorithm.max_iterations * (color_algorithm.to.at(2) - color_algorithm.from.at(2)));
            } else if constexpr (std::is_same<C, Periodic>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(color_algorithm.amplitude.at(0) * v / escape_algorithm.max_iterations + color_algorithm.phase.at(0)) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(color_algorithm.amplitude.at(1) * v / escape_algorithm.max_iterations + color_algorithm.phase.at(1)) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(color_algorithm.amplitude.at(2) * v / escape_algorithm.max_iterations + color_algorithm.phase.at(2)) + 1) * 127.5);
            } else if constexpr (std::is_same<C, Solid>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = color_algorithm.color.at(0);
                image.data.at(3 * i * image.width + 3 * j + 1) = color_algorithm.color.at(1);
                image.data.at(3 * i * image.width + 3 * j + 2) = color_algorithm.color.at(2);
            } else throw std::runtime_error("COLORING ALGORITHM FOR THIS GENERATION ALGORITHM NOT SUPPORTED");
        }
    }
}

template <typename T, class C>
void draw_trap(Image& image, const Fractal& fractal, const std::complex<T>& center, T zoom, const Trap& trap_algorithm, const C& color_algorithm) {
    auto fractal_function    = FractalFunctions<T>()        .content.at(fractal.name             );
    auto fractal_ic_function = FractalInitialConditions<T>().content.at(fractal.name             );
    auto trap_function       = TrapFunctions<T>()           .content.at(trap_algorithm.trap_index);

    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) for (int j = 0; j < image.width; j++) {

        std::complex<T> parameter = exp(std::complex<T>{0, 1} * T(fractal.parameter));

        T im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
        T re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

        auto [p, z, zp] = fractal_ic_function(std::complex<T>{re, im}, parameter); std::vector<std::complex<T>> orbit;

        for(int n = 0; n < trap_algorithm.max_iterations; n++) {
            std::tie(z, zp) = fractal_function(p, z, zp, parameter); if (std::norm(z) > trap_algorithm.bailout_radius * trap_algorithm.bailout_radius) break; orbit.push_back(z);
        }

        if (!trap_algorithm.fill_background || std::norm(z) > trap_algorithm.bailout_radius * trap_algorithm.bailout_radius) {

            T v = trap_function(*std::min_element(orbit.begin(), orbit.end(), [&](const std::complex<T>& a, const std::complex<T>& b) {return trap_function(a) < trap_function(b);}));
            
            if constexpr (std::is_same<C, Linear>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((double)color_algorithm.from.at(0) + 1 / (1 + 5 * v) * (color_algorithm.to.at(0) - color_algorithm.from.at(0)));
                image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((double)color_algorithm.from.at(1) + 1 / (1 + 5 * v) * (color_algorithm.to.at(1) - color_algorithm.from.at(1)));
                image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((double)color_algorithm.from.at(2) + 1 / (1 + 5 * v) * (color_algorithm.to.at(2) - color_algorithm.from.at(2)));
            } else if constexpr (std::is_same<C, Periodic>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(color_algorithm.amplitude.at(0) * 0.03 * log(v) + color_algorithm.phase.at(0)) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(color_algorithm.amplitude.at(1) * 0.03 * log(v) + color_algorithm.phase.at(1)) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(color_algorithm.amplitude.at(2) * 0.03 * log(v) + color_algorithm.phase.at(2)) + 1) * 127.5);
            } else if constexpr (std::is_same<C, Solid>()) {
                image.data.at(3 * i * image.width + 3 * j + 0) = color_algorithm.color.at(0);
                image.data.at(3 * i * image.width + 3 * j + 1) = color_algorithm.color.at(1);
                image.data.at(3 * i * image.width + 3 * j + 2) = color_algorithm.color.at(2);
            } else throw std::runtime_error("COLORING ALGORITHM FOR THIS GENERATION ALGORITHM NOT SUPPORTED");
        }
    }
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Fractal", "1.0", argparse::default_arguments::none);

    program.add_argument("-c", "--center").help("-- Center of the image.").nargs(2).default_value(std::vector<std::string>{"-0.75", "0"});
    program.add_argument("-d", "--density").help("-- Density algorithm with number of iterations, bailout radius, number of samples and seed.").nargs(4).default_value(std::vector<std::string>{"80", "10", "1e7", "1"});
    program.add_argument("-e", "--escape").help("-- Escape algorithm with number of iterations, bailout radius and smooth boolean.").nargs(3).default_value(std::vector<std::string>{"80", "10", "1"});
    program.add_argument("-f", "--fractal").help("-- Name of the fractal to generate and its parameter if needed.").nargs(3).default_value(std::vector<std::string>{"mandelbrot", "0"});
    program.add_argument("-h", "--help").help("-- This help message.").default_value(false).implicit_value(true);
    program.add_argument("-l", "--linear").help("-- Linear coloring algorithm with red, green and blue parameters for start and end of linear interpolation.").nargs(6).default_value(std::vector<unsigned int>{0, 0, 0, 255, 255, 255}).scan<'i', unsigned int>();
    program.add_argument("-m", "--mpfr").help("-- Number of bits to represent a MPFR real number.").default_value(64U).scan<'i', unsigned int>();
    program.add_argument("-n", "--nthread").help("-- Number of threads to use.").default_value(1U).scan<'i', unsigned int>();
    program.add_argument("-o", "--output").help("-- Output filename.").default_value("fractal.png");
    program.add_argument("-p", "--periodic").help("-- Periodic coloring algorithm with 6 parameters.").nargs(6).default_value(std::vector<double>{31.93, 6.26, 30.38, 5.86, 11.08, 0.81}).scan<'g', double>();
    program.add_argument("-r", "--resolution").help("-- Resolution of the image.").nargs(2).default_value(std::vector<unsigned int>{1920U, 1080U}).scan<'i', unsigned int>();
    program.add_argument("-s", "--solid").help("-- Solid coloring algorithm with red, green and blue parameters.").nargs(3).default_value(std::vector<unsigned int>{255, 255, 255}).scan<'i', unsigned int>();
    program.add_argument("-t", "--trap").help("-- Trap algorithm with number of iterations, bailout radius, trap index and fill boolean.").nargs(4).default_value(std::vector<std::string>{"80", "100", "2", "0"});
    program.add_argument("-z", "--zoom").help("-- Zoom of the image.").default_value("1.1");

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    if (program.is_used("--escape") + program.is_used("--trap") + program.is_used("--density") > 1) {
        throw std::runtime_error("YOU CAN USE ONLY ONE ALGORITHM AT A TIME");
    }

    if (program.is_used("--linear") + program.is_used("--periodic") + program.is_used("--solid") > 1) {
        throw std::runtime_error("YOU CAN USE ONLY ONE COLORING AT A TIME");
    }

    std::complex<mpfr::mpreal> center; nthread = program.get<unsigned int>("--nthread");

    Image image; Fractal fractal; Density density_algorithm; Escape escape_algorithm; Trap trap_algorithm; Linear linear_algorithm; Periodic periodic_algorithm; Solid solid_algorithm;

    image.width  = program.get<std::vector<unsigned int>>("--resolution").at(0 );
    image.height = program.get<std::vector<unsigned int>>("--resolution").at(1 );
    image.data   = std::vector<unsigned char>(3 * image.width * image.height, 0);

    fractal.name = program.get<std::vector<std::string>>("--fractal").at(0), fractal.parameter = std::stod(program.get<std::vector<std::string>>("--fractal").at(1));

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

    linear_algorithm.from.at(0) = program.get<std::vector<unsigned int>>("--linear").at(0);
    linear_algorithm.from.at(1) = program.get<std::vector<unsigned int>>("--linear").at(1);
    linear_algorithm.from.at(2) = program.get<std::vector<unsigned int>>("--linear").at(2);
    linear_algorithm.to.at(0)   = program.get<std::vector<unsigned int>>("--linear").at(3);
    linear_algorithm.to.at(1)   = program.get<std::vector<unsigned int>>("--linear").at(4);
    linear_algorithm.to.at(2)   = program.get<std::vector<unsigned int>>("--linear").at(5);

    periodic_algorithm.amplitude.at(0) = program.get<std::vector<double>>("--periodic").at(0);
    periodic_algorithm.phase.at(0)     = program.get<std::vector<double>>("--periodic").at(1);
    periodic_algorithm.amplitude.at(1) = program.get<std::vector<double>>("--periodic").at(2);
    periodic_algorithm.phase.at(1)     = program.get<std::vector<double>>("--periodic").at(3);
    periodic_algorithm.amplitude.at(2) = program.get<std::vector<double>>("--periodic").at(4);
    periodic_algorithm.phase.at(2)     = program.get<std::vector<double>>("--periodic").at(5);

    solid_algorithm.color.at(0) = program.get<std::vector<unsigned int>>("--solid").at(0);
    solid_algorithm.color.at(1) = program.get<std::vector<unsigned int>>("--solid").at(1);
    solid_algorithm.color.at(2) = program.get<std::vector<unsigned int>>("--solid").at(2);

    center.real(mpfr::mpreal(program.get<std::vector<std::string>>("--center").at(0)));
    center.imag(mpfr::mpreal(program.get<std::vector<std::string>>("--center").at(1)));

    #pragma omp parallel for num_threads(nthread)
    for (int i = 0; i < nthread; i++) mpfr::mpreal::set_default_prec(nthread);

    if (program.is_used("--linear")) {
        if (program.is_used("--density")) {
            if (program.is_used("--mpfr")) draw_density<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), density_algorithm, linear_algorithm);
            else draw_density<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), density_algorithm, linear_algorithm);
        } else if (program.is_used("--trap")) {
            if (program.is_used("--mpfr")) draw_trap<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), trap_algorithm, linear_algorithm);
            else draw_trap<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), trap_algorithm, linear_algorithm);
        } else {
            if (program.is_used("--mpfr")) draw_escape<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), escape_algorithm, linear_algorithm);
            else draw_escape<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), escape_algorithm, linear_algorithm);
        }
    } else if (program.is_used("--solid")) {
        if (program.is_used("--density")) {
            if (program.is_used("--mpfr")) draw_density<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), density_algorithm, solid_algorithm);
            else draw_density<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), density_algorithm, solid_algorithm);
        } else if (program.is_used("--trap")) {
            if (program.is_used("--mpfr")) draw_trap<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), trap_algorithm, solid_algorithm);
            else draw_trap<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), trap_algorithm, solid_algorithm);
        } else {
            if (program.is_used("--mpfr")) draw_escape<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), escape_algorithm, solid_algorithm);
            else draw_escape<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), escape_algorithm, solid_algorithm);
        }
    } else {
        if (program.is_used("--density")) {
            if (program.is_used("--mpfr")) draw_density<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), density_algorithm, periodic_algorithm);
            else draw_density<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), density_algorithm, periodic_algorithm);
        } else if (program.is_used("--trap")) {
            if (program.is_used("--mpfr")) draw_trap<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), trap_algorithm, periodic_algorithm);
            else draw_trap<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), trap_algorithm, periodic_algorithm);
        } else {
            if (program.is_used("--mpfr")) draw_escape<mpfr::mpreal>(image, fractal, center, program.get("--zoom"), escape_algorithm, periodic_algorithm);
            else draw_escape<double>(image, fractal, {center.real().toDouble(), center.imag().toDouble()}, std::stod(program.get("--zoom")), escape_algorithm, periodic_algorithm);
        }
    }

    stbi_write_png(program.get("--output").c_str(), image.width, image.height, 3, image.data.data(), 3 * image.width);
}
