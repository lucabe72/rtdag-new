#ifndef RTDAG_COMMAND_H
#define RTDAG_COMMAND_H

#include "input.h"
#include <getopt.h>

#include <optional>
#include <cassert>

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          Command Line Arguments                           ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void usage(char *program_name) {
    auto usage_format = R"STRING(Usage: %s [ OPTION %s]
Developed by ReTiS Laboratory, Scuola Sant'Anna, Pisa (2022).

Where OPTION can be (only) one of the following:
    -h, --help                  Display this helpful message
    -c USEC, --calibrate USEC   Run a calibration diagnostic for count_ticks
    -t USEC, --test USEC        Test calibration accuracy for count_ticks

If no OPTION is supplied, a DAG is run. The input mode for specifying the DAG
information is: %s.

The exception to the above rule is the following OPTION, which will instead
affect only runs that specify a DAG file:

    -e RATIO, --expected RATIO

The RATIO parameter is a float that shall be greater than 0 and less than or
equal to 1.0. It is used to compute the actual runtime of a task (in a
frequency-independent way) as a fraction of its WCET.
This OPTION must come BEFORE the DAG definition filename, if the input mode
compiled against this program requires one.

)STRING";

    if constexpr (input_type::has_input_file) {
        printf(usage_format, program_name,
               "| <INPUT_" INPUT_TYPE_NAME_CAPS "_FILE> ", INPUT_TYPE_NAME);
    } else {
        printf(usage_format, program_name, "", INPUT_TYPE_NAME);
    }
}

enum class command_action {
    HELP,
    RUN_DAG,
    CALIBRATE,
    TEST,
};

struct opts {
    command_action action = command_action::RUN_DAG;
    string in_fname = "";
    uint64_t duration_us = 0;
    float expected_wcet_ratio = 0;
    int exit_code = EXIT_SUCCESS;
};

template <class ReturnType>
optional<ReturnType> parse_argument_from_string(const char *str) {
    auto mstring = std::string(str);
    auto mstream = std::istringstream(mstring);

    ReturnType rt;
    mstream >> rt;

    return mstream ? optional<ReturnType>(rt) : nullopt;
}

opts parse_args(int argc, char *argv[]) {
    opts program_options;
    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"calibrate", required_argument, 0, 'c'},
            {"test", required_argument, 0, 't'},
            {"expected", required_argument, 0, 'e'},
            {0, 0, 0, 0}};

        int c = getopt_long(argc, argv, "hc:t:e:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 0:
            // No long option has no corresponding short option
            fprintf(stderr, "Error: ?? getopt returned character code 0%o ??\n",
                    c);
            program_options.exit_code = EXIT_FAILURE;
            assert(false);
            break;
        case 'h':
            program_options.action = command_action::HELP;
            goto end;
        case 'c':
        case 't': {
            auto duration_valid = parse_argument_from_string<uint64_t>(optarg);
            if (!duration_valid) {
                goto duration_error;
            }
            program_options.duration_us = *duration_valid;
            program_options.action =
                c == 'c' ? command_action::CALIBRATE : command_action::TEST;

            if (program_options.duration_us > 0.0) {
                goto end;
            }

        duration_error:
            fprintf(stderr, "Invalid argument to option: %s\n", optarg);
            program_options.action = command_action::HELP;
            program_options.exit_code = EXIT_FAILURE;
            goto end;
        }
        case 'e': {
            auto expected_valid = parse_argument_from_string<float>(optarg);
            if (!expected_valid) {
                goto expected_error;
            }

            program_options.expected_wcet_ratio = *expected_valid;
            if (program_options.expected_wcet_ratio < 0.0 ||
                program_options.expected_wcet_ratio > 1.0) {
                goto expected_error;
            }

            break;

        expected_error:
            fprintf(stderr, "Invalid argument to option: %s\n", optarg);
            program_options.action = command_action::HELP;
            program_options.exit_code = EXIT_FAILURE;
            goto end;
        }
        case '?':
            // fprintf(stderr, "Error: ?? getopt returned character code
            // 0%o ??\n", c);
            program_options.action = command_action::HELP;
            program_options.exit_code = EXIT_FAILURE;
            goto end;
        default:
            fprintf(stderr, "Error: ?? getopt returned character code 0%o ??\n",
                    c);
            program_options.exit_code = EXIT_FAILURE;
            goto end;
        }
    }

    // static check whether this input format does have an input file or not
    if constexpr (input_type::has_input_file) {
        if (optind < argc) {
            program_options.in_fname = argv[optind++];
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Error: too many arguments supplied!\n");
        program_options.action = command_action::HELP;
        program_options.exit_code = EXIT_FAILURE;
        goto end;
    }

    // static check whether this input format does have an input file or not
    if constexpr (input_type::has_input_file) {
        if (program_options.in_fname.length() < 1) {
            fprintf(stderr, "Error: too few arguments supplied!\n");
            program_options.action = command_action::HELP;
            program_options.exit_code = EXIT_FAILURE;
            goto end;
        }
    }

end:
    return program_options;
}

#endif // RTDAG_COMMAND_H
