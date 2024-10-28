#define STB_IMAGE_WRITE_IMPLEMENTATION

#include      <argparse.hpp>
#include          <mpreal.h>
#include <stb_image_write.h>
#include           <complex>

unsigned int nthread = 1;

struct Image {
    std::vector<unsigned char> data; unsigned int width, height;
};

template <typename T>
void draw_escape(Image& image, const std::complex<T>& center, T zoom, unsigned int iterations, T bailout, bool smooth) {
    #pragma omp parallel for collapse(2) num_threads(nthread)
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {

            T im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
            T re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

            std::complex<T> p = {re, im}, z = T(0); int n; T v;

            for (n = 0; n < iterations; n++) {
                z = z * z + p; if (std::norm(z) > bailout * bailout) break;
            }

            if (v = n; n < iterations && smooth) v -= log2(0.5 * log(std::norm(z)));

            if (n < iterations) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (unsigned char)((sin(31.93 * v / iterations + 6.26) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 1) = (unsigned char)((sin(30.38 * v / iterations + 5.86) + 1) * 127.5);
                image.data.at(3 * i * image.width + 3 * j + 2) = (unsigned char)((sin(11.08 * v / iterations + 0.80) + 1) * 127.5);
            }
        }
    }
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Mandelbrot", "1.0", argparse::default_arguments::none);

    program.add_argument("-b", "--bailout"   ).help("-- Bailout radius for the escape time algorithm."       ).default_value(10.0).scan<'g', double>();
    program.add_argument("-c", "--center"    ).help("-- Center of the image."                                ).nargs(2).default_value(std::vector<std::string>{"-0.75", "0"});
    program.add_argument("-h", "--help"      ).help("-- This help message."                                  ).default_value(false).implicit_value(true);
    program.add_argument("-i", "--iterations").help("-- Number of iterations for iteration based algorithms.").default_value(80U).scan<'i', unsigned int>();
    program.add_argument("-m", "--mpfr"      ).help("-- Number of bits to represent a MPFR real number."     ).default_value(64U).scan<'i', unsigned int>();
    program.add_argument("-n", "--nthread"   ).help("-- Number of threads to use."                           ).default_value(1U).scan<'i', unsigned int>();
    program.add_argument("-o", "--output"    ).help("-- Output filename."                                    ).default_value("fractal.png");
    program.add_argument("-r", "--resolution").help("-- Resolution of the image."                            ).nargs(2).default_value(std::vector<unsigned int>{1920U, 1080U}).scan<'i', unsigned int>();
    program.add_argument("-s", "--smooth"    ).help("-- Smoothing of the iteration values."                  ).default_value(false).implicit_value(true);
    program.add_argument("-z", "--zoom"      ).help("-- Zoom of the image."                                  ).default_value("1.1");

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    nthread = program.get<unsigned int>("--nthread"); auto resolution = program.get<std::vector<unsigned int>>("--resolution"); int iterations = program.get<unsigned int>("--iterations");

    mpfr::mpreal center_real(program.get<std::vector<std::string>>("--center").at(0)), center_imag(program.get<std::vector<std::string>>("--center").at(1));

    Image image; image.width = resolution.at(0), image.height = resolution.at(1); image.data = std::vector<unsigned char>(3 * image.width * image.height, 0);

    #pragma omp parallel for
    for (int i = 0; i < nthread; i++) mpfr::mpreal::set_default_prec(program.get<unsigned int>("--mpfr"));

    if (program.is_used("--mpfr")) {
        draw_escape<mpfr::mpreal>(image, {center_real, center_imag}, program.get("--zoom"), iterations, program.get<double>("--bailout"), program.get<bool>("--smooth"));
    } else {
        draw_escape<double>(image, {center_real.toDouble(), center_imag.toDouble()}, std::stod(program.get("--zoom")), iterations, program.get<double>("--bailout"), program.get<bool>("--smooth"));
    }

    stbi_write_png(program.get("--output").c_str(), image.width, image.height, 3, image.data.data(), 3 * image.width);
}
