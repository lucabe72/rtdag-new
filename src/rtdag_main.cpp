#include <cstdlib>

#include "rtdag_calib.h"
#include "rtdag_command.h"
#include "rtdag_run.h"

#include "rtgauss.h"

float expected_wcet_ratio_override = 0.0;

int main(int argc, char *argv[]) {
    auto program_options = parse_args(argc, argv);
    expected_wcet_ratio_override = program_options.expected_wcet_ratio;

    switch (program_options.action) {
    case command_action::HELP:
        usage(argv[0]);
        return program_options.exit_code;

    case command_action::CALIBRATE:
        // FIXME: pre-charge code on the GPU
        rtgauss_init(program_options.rtg_msize, program_options.rtg_type,
                     program_options.rtg_target);
        return calibrate(program_options.duration_us);

    case command_action::TEST:
        rtgauss_init(program_options.rtg_msize, program_options.rtg_type,
                     program_options.rtg_target);
        return test_calibration(program_options.duration_us);

    case command_action::RUN_DAG:
        if (program_options.exit_code != EXIT_SUCCESS) {
            return program_options.exit_code;
        }

        return run_dag(program_options.in_fname);
    }

    assert(false);
    cerr << "There's an error in the implementation!" << endl;
    return EXIT_FAILURE;
}
