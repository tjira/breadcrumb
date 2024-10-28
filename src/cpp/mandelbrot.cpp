#define STB_IMAGE_WRITE_IMPLEMENTATION

#include      <argparse.hpp>
#include           <gmpxx.h>
#include <stb_image_write.h>
#include           <complex>

struct Image {
    std::vector<unsigned char> data; unsigned int width, height;
};

void draw(Image& image, const std::complex<double>& center, double zoom, int iterations, double escape, bool smooth) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < image.height; i++) {
        for (int j = 0; j < image.width; j++) {

            double im = -center.imag() + (3.0 * (i + 0.5) - 1.5 * image.height) / zoom / image.height;
            double re =  center.real() + (3.0 * (j + 0.5) - 1.5 * image.width)  / zoom / image.height;

            std::complex<double> p = {re, im}, z = 0; int n; double v;

            for (n = 0; n < iterations; n++) {
                z = z * z + p; if (std::norm(z) > escape * escape) break;
            }

            if (v = n; n < iterations && smooth) v -= std::log2(0.5 * std::log(std::norm(z)));

            if (n < iterations) {
                image.data.at(3 * i * image.width + 3 * j + 0) = (std::sin(31.93 * v / iterations + 6.26) + 1) * 127.5;
                image.data.at(3 * i * image.width + 3 * j + 1) = (std::sin(30.38 * v / iterations + 5.86) + 1) * 127.5;
                image.data.at(3 * i * image.width + 3 * j + 2) = (std::sin(11.08 * v / iterations + 0.80) + 1) * 127.5;
            }
        }
    }
}

int main(int argc, char** argv) {
    argparse::ArgumentParser program("Mersenne", "1.0", argparse::default_arguments::none);

    program.add_argument("-i", "--iterations").help("-- Number of iterations for iteration based algorithms.").default_value(80U).scan<'i', unsigned int>();
    program.add_argument("-e", "--escape"    ).help("-- Escape radius for the escape time algorithm."        ).default_value(10.0).scan<'g', double>();
    program.add_argument("-z", "--zoom"      ).help("-- Zoom of the image."                                  ).default_value(1.1).scan<'g', double>();
    program.add_argument("-c", "--center"    ).help("-- Center of the image."                                ).nargs(2).default_value(std::vector<double>{-0.75, 0}).scan<'g', double>();
    program.add_argument("-r", "--resolution").help("-- Resolution of the image."                            ).nargs(2).default_value(std::vector<unsigned int>{1920, 1080}).scan<'i', unsigned int>();
    program.add_argument("-s", "--smooth"    ).help("-- Smoothing of the iteration values."                  ).default_value(false).implicit_value(true);
    program.add_argument("-h", "--help"      ).help("-- This help message."                                  ).default_value(false).implicit_value(true);
    program.add_argument("-o", "--output"    ).help("-- Output filename."                                    ).default_value("fractal.png");

    try {program.parse_args(argc, argv);} catch (const std::runtime_error& error) {
        if (!program.get<bool>("-h")) std::cerr << error.what() << std::endl, exit(EXIT_FAILURE);
    } if (program.get<bool>("-h")) std::cout << program.help().str(), exit(EXIT_SUCCESS);

    auto center     = program.get<std::vector<double>>      ("--center"    );
    auto escape     = program.get<double>                   ("--escape"    );
    auto iterations = program.get<unsigned int>             ("--iterations");
    auto resolution = program.get<std::vector<unsigned int>>("--resolution");
    auto zoom       = program.get<double>                   ("--zoom"      );
    auto smooth     = program.get<bool>                     ("--smooth"    );

    Image image; image.width = resolution.at(0), image.height = resolution.at(1); image.data = std::vector<unsigned char>(3 * image.width * image.height, 0);

    draw(image, {center.at(0), center.at(1)}, zoom, iterations, escape, smooth);

    stbi_write_png(program.get("--output").c_str(), image.width, image.height, 3, image.data.data(), 3 * image.width);
}
