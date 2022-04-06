
#include <getopt.h>
#include <stdio.h>

#include <valarray>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

static bool demo_mode = false;
static bool benchmark_mode = false;

/*
    Run arctic with --demo or -d to execute this editable demo code.

    A good place to run your own quick tests or use arctic without any wrappers.
    Remember to call make to recompile after editing.

    Demo version:
        + Make a test image and save it to a txt file.
        + Load the image from txt.
        + Add parallel and serial CTI.
        + Remove the CTI and save the result to file.
*/
int run_demo() {

    // Write an example image to a txt file
    save_image_to_txt(
        (char*)"image_test_pre_cti.txt",
        // clang-format off
        std::valarray<std::valarray<double>>{
            {0.0,   0.0,   0.0,   0.0},
            {200.0, 0.0,   0.0,   0.0},
            {0.0,   200.0, 0.0,   0.0},
            {0.0,   0.0,   200.0, 0.0},
            {0.0,   0.0,   0.0,   0.0},
            {0.0,   0.0,   0.0,   0.0},
        }  // clang-format on
    );

    // Load the image
    std::valarray<std::valarray<double>> image_pre_cti =
        load_image_from_txt((char*)"image_test_pre_cti.txt");
    print_v(1, "\n# Loaded test image from image_test_pre_cti.txt: \n");
    print_array_2D(image_pre_cti);

    // CTI model parameters
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<TrapInstantCapture> traps_ic = {trap};
    std::valarray<TrapSlowCapture> traps_sc = {};
    std::valarray<TrapInstantCaptureContinuum> traps_ic_co = {};
    std::valarray<TrapSlowCaptureContinuum> traps_sc_co = {};
    std::valarray<double> dwell_times = {1.0};
    bool empty_traps_between_columns = true;
    bool empty_traps_for_first_transfers = true;
    ROE roe(dwell_times, empty_traps_between_columns, empty_traps_for_first_transfers);
    CCD ccd(CCDPhase(1e3, 0.0, 1.0));
    int express = 0;
    int offset = 0;
    int start = 0;
    int stop = -1 / start;

    // Add parallel and serial CTI (ic = instant capture, sc = slow capture, co = continuum release)
    print_v(1, "\n# Add CTI \n");
    std::valarray<std::valarray<double>> image_post_cti = add_cti(
        image_pre_cti, 
        &roe, &ccd, &traps_ic, &traps_sc, &traps_ic_co, &traps_sc_co,
        express, offset, start, stop, start, stop, 
        &roe, &ccd, &traps_ic, &traps_sc, &traps_ic_co, &traps_sc_co, 
        express, offset, start, stop, start, stop, 0);
    print_v(1, "\n# Image with CTI added: \n");
    print_array_2D(image_post_cti);

    // Remove CTI
    print_v(1, "\n# Remove CTI \n");
    int n_iterations = 3;
    std::valarray<std::valarray<double>> image_remove_cti = remove_cti(
        image_post_cti, n_iterations, 
        &roe, &ccd, &traps_ic, &traps_sc, &traps_ic_co, &traps_sc_co, 
        express, offset, start, stop, start, stop, 
        &roe, &ccd, &traps_ic, &traps_sc, &traps_ic_co, &traps_sc_co, 
        express, offset, start, stop, start, stop);
    print_v(1, "\n# Image with CTI removed: \n");
    print_array_2D(image_remove_cti);

    // Save the final image
    save_image_to_txt((char*)"image_test_cti_removed.txt", image_remove_cti);
    print_v(1, "# Saved final image to image_test_cti_removed.txt \n");

    return 0;
}

/*
    Run arctic with --benchmark or -b for this simple test, e.g. for profiling.

    Add CTI to a 10-column extract of an HST ACS image. Takes ~0.02 s.
*/
int run_benchmark() {

    // Download the test image
    const char* filename = "hst_acs_10_col.txt";
    FILE* f = fopen(filename, "r");
    if (!f) {
        const char* command =
            "wget http://astro.dur.ac.uk/~cklv53/files/hst_acs_10_col.txt";
        printf("%s\n", command);
        int status = system(command);
        if (status != 0) exit(status);
    } else
        fclose(f);

    // Load the image
    std::valarray<std::valarray<double>> image_pre_cti = load_image_from_txt(filename);

    // CTI model parameters
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    std::valarray<TrapInstantCapture> traps = {trap};
    std::valarray<double> dwell_times = {1.0};
    ROE roe(dwell_times, true, false, true, false);
    CCD ccd(CCDPhase(1e4, 0.0, 1.0));
    int express = 5;
    int offset = 0;
    int start = 0;
    int stop = -1;
    
    // Add parallel CTI
    std::valarray<std::valarray<double>> image_post_cti = add_cti(
        image_pre_cti, &roe, &ccd, &traps, nullptr, nullptr, nullptr, express, offset,
        start, stop, start, stop);

    return 0;
}

/*
    Print help information.
*/
void print_help() {
    printf(
        "ArCTIC \n"
        "====== \n"
        "AlgoRithm for Charge Transfer Inefficiency (CTI) Correction \n"
        "----------------------------------------------------------- \n"
        "Add or remove image trails due to charge transfer inefficiency in CCD "
        "detectors by modelling the trapping, releasing, and moving of charge along "
        "pixels. \n"
        "\n"
        "-h, --help \n"
        "    Print help information and exit. \n"
        "-v <int>, --verbosity=<int> \n"
        "    The verbosity parameter to control the amount of printed information: \n"
        "        0       No printing (except errors etc). \n"
        "        1       Standard. \n"
        "        2       Extra details. \n"
        "-d, --demo \n"
        "    Execute the demo code in the run_demo() function at the very top of \n"
        "    main.cpp. For manual editing to test or run arctic without using any \n"
        "    wrappers. The demo version adds then removes CTI from a test image. \n"
        "-b, --benchmark \n"
        "    Execute the run_benchmark() function in main.cpp, e.g. for profiling. \n"
        "\n"
        "See README.md for more information.  https://github.com/jkeger/arctic \n\n");
}

/*
    Parse input parameters. See main()'s documentation.
*/
void parse_parameters(int argc, char** argv) {
    // Short options
    const char* const short_opts = ":hv:db";
    // Full options
    const option long_opts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"verbosity", required_argument, nullptr, 'v'},
        {"demo", no_argument, nullptr, 'd'},
        {"benchmark", no_argument, nullptr, 'b'},
        {0, 0, 0, 0}};

    // Parse options
    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (opt == -1) break;

        switch (opt) {
            case 'h':
                print_help();
                exit(0);
            case 'v':
                set_verbosity(atoi(optarg));
                break;
            case 'd':
                demo_mode = true;
                break;
            case 'b':
                benchmark_mode = true;
                break;
            case ':':
                printf(
                    "Error: Option %s requires a value. Run with -h for help. \n",
                    argv[optind - 1]);
                exit(1);
            case '?':
                printf(
                    "Error: Option %s not recognised. Run with -h for help. \n",
                    argv[optind - 1]);
                exit(1);
        }
    }

    // Other parameters (currently unused)
    for (; optind < argc; optind++) {
        printf("Unparsed parameter: %s \n", argv[optind]);
    }
}

/*
    Main program.

    Parameters
    ----------
    -h, --help
        Print help information and exit.

    -v <int>, --verbosity=<int>
        The verbosity parameter to control the amount of printed information:
            0       No printing (except errors etc).
            1       Standard.
            2       Extra details.

    -d, --demo
        Execute the demo code in the run_demo() function at the very top of this
        file. For easy manual editing to test or run arctic without using any
        wrappers. The demo version adds then removes CTI from a test image.

    -b, --benchmark
        Execute the run_benchmark() function above, e.g. for profiling.
*/
int main(int argc, char** argv) {

    parse_parameters(argc, argv);

    if (demo_mode) {
        print_v(1, "# Running demo code! \n");
        return run_demo();
    }
    if (benchmark_mode) {
        print_v(1, "# Running benchmark code \n");
        return run_benchmark();
    }

    return 0;
}
