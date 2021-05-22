import numpy as np
import arcticpy.wrapper as w
from arcticpy.src.ccd import CCDPhase, CCD
from arcticpy.src.roe import ROE
from arcticpy.src.traps import Trap, TrapInstantCapture


def add_cti(
    image,
    # Parallel
    parallel_ccd=None,
    parallel_roe=None,
    parallel_traps=None,
    parallel_express=0,
    parallel_offset=0,
    parallel_window_start=0,
    parallel_window_stop=-1,
    # Serial
    serial_ccd=None,
    serial_roe=None,
    serial_traps=None,
    serial_express=0,
    serial_offset=0,
    serial_window_start=0,
    serial_window_stop=-1,
    # Output
    verbosity=1,
):
    """
    Wrapper for arctic's add_cti() in src/cti.cpp, see its documentation.

    Add CTI trails to an image by trapping, releasing, and moving electrons
    along their independent columns, for parallel and/or serial clocking.

    This wrapper extracts individual numbers and arrays from the user-input
    objects to pass to the C++ via Cython. See cy_add_cti() in
    arcticpy/wrapper.pyx and add_cti() in arcticpy/interface.cpp.
    """
    image = np.copy(image).astype(np.double)

    # Extract inputs to pass to the wrapper

    # Parallel
    if parallel_traps is not None:
        # Extract trap inputs
        parallel_traps_standard = [
            trap for trap in parallel_traps if type(trap) == Trap
        ]
        parallel_traps_instant_capture = [
            trap for trap in parallel_traps if type(trap) == TrapInstantCapture
        ]
        parallel_n_traps_standard = len(parallel_traps_standard)
        parallel_n_traps_instant_capture = len(parallel_traps_instant_capture)
        if parallel_n_traps_standard + parallel_n_traps_instant_capture != len(
            parallel_traps
        ):
            raise Exception(
                "Not all traps extracted successfully (%d standard, %d instant capture, %d total)"
                % (
                    parallel_n_traps_standard,
                    parallel_n_traps_instant_capture,
                    len(parallel_traps),
                )
            )
        # Make sure the order is correct
        parallel_traps = parallel_traps_standard + parallel_traps_instant_capture
        parallel_trap_densities = np.array(
            [trap.density for trap in parallel_traps], dtype=np.double
        )
        parallel_trap_release_timescales = np.array(
            [trap.release_timescale for trap in parallel_traps], dtype=np.double
        )
        parallel_trap_capture_timescales = np.array(
            [trap.capture_timescale for trap in parallel_traps], dtype=np.double
        )
    else:
        # No parallel clocking, set dummy variables instead
        parallel_roe = ROE()
        parallel_ccd = CCD([CCDPhase(0.0, 0.0, 0.0)], [0.0])
        parallel_trap_densities = np.array([0.0], dtype=np.double)
        parallel_trap_release_timescales = np.array([0.0], dtype=np.double)
        parallel_trap_capture_timescales = np.array([0.0], dtype=np.double)
        parallel_n_traps_standard = 0
        parallel_n_traps_instant_capture = 0
        parallel_express = 0
        parallel_offset = 0
        parallel_window_start = 0
        parallel_window_stop = 0

    # Serial
    if serial_traps is not None:
        # Extract trap inputs
        serial_traps_standard = [trap for trap in serial_traps if type(trap) == Trap]
        serial_traps_instant_capture = [
            trap for trap in serial_traps if type(trap) == TrapInstantCapture
        ]
        serial_n_traps_standard = len(serial_traps_standard)
        serial_n_traps_instant_capture = len(serial_traps_instant_capture)
        if serial_n_traps_standard + serial_n_traps_instant_capture != len(
            serial_traps
        ):
            raise Exception(
                "Not all traps extracted successfully (%d standard, %d instant capture, %d total)"
                % (
                    serial_n_traps_standard,
                    serial_n_traps_instant_capture,
                    len(serial_traps),
                )
            )
        # Make sure the order is correct
        serial_traps = serial_traps_standard + serial_traps_instant_capture
        serial_trap_densities = np.array(
            [trap.density for trap in serial_traps], dtype=np.double
        )
        serial_trap_release_timescales = np.array(
            [trap.release_timescale for trap in serial_traps], dtype=np.double
        )
        serial_trap_capture_timescales = np.array(
            [trap.capture_timescale for trap in serial_traps], dtype=np.double
        )
    else:
        # No serial clocking, set dummy variables instead
        serial_roe = ROE()
        serial_ccd = CCD([CCDPhase(0.0, 0.0, 0.0)], [0.0])
        serial_trap_densities = np.array([0.0], dtype=np.double)
        serial_trap_release_timescales = np.array([0.0], dtype=np.double)
        serial_trap_capture_timescales = np.array([0.0], dtype=np.double)
        serial_n_traps_standard = 0
        serial_n_traps_instant_capture = 0
        serial_express = 0
        serial_offset = 0
        serial_window_start = 0
        serial_window_stop = 0

    # ========
    # Add CTI
    # ========
    # Pass the extracted inputs to C++ via the cython wrapper
    return w.cy_add_cti(
        image,
        # ========
        # Parallel
        # ========
        # ROE
        parallel_roe.dwell_times,
        parallel_roe.empty_traps_between_columns,
        parallel_roe.empty_traps_for_first_transfers,
        parallel_roe.force_release_away_from_readout,
        parallel_roe.use_integer_express_matrix,
        # CCD
        parallel_ccd.fraction_of_traps_per_phase,
        parallel_ccd.full_well_depths,
        parallel_ccd.well_notch_depths,
        parallel_ccd.well_fill_powers,
        # Traps
        parallel_trap_densities,
        parallel_trap_release_timescales,
        parallel_trap_capture_timescales,
        parallel_n_traps_standard,
        parallel_n_traps_instant_capture,
        # Misc
        parallel_express,
        parallel_offset,
        parallel_window_start,
        parallel_window_stop,
        # ========
        # Serial
        # ========
        # ROE
        serial_roe.dwell_times,
        serial_roe.empty_traps_between_columns,
        serial_roe.empty_traps_for_first_transfers,
        serial_roe.force_release_away_from_readout,
        serial_roe.use_integer_express_matrix,
        # CCD
        serial_ccd.fraction_of_traps_per_phase,
        serial_ccd.full_well_depths,
        serial_ccd.well_notch_depths,
        serial_ccd.well_fill_powers,
        # Traps
        serial_trap_densities,
        serial_trap_release_timescales,
        serial_trap_capture_timescales,
        serial_n_traps_standard,
        serial_n_traps_instant_capture,
        # Misc
        serial_express,
        serial_offset,
        serial_window_start,
        serial_window_stop,
        # Output
        verbosity,
    )


def remove_cti(
    image,
    n_iterations,
    # Parallel
    parallel_ccd=None,
    parallel_roe=None,
    parallel_traps=None,
    parallel_express=0,
    parallel_offset=0,
    parallel_window_start=0,
    parallel_window_stop=-1,
    # Serial
    serial_ccd=None,
    serial_roe=None,
    serial_traps=None,
    serial_express=0,
    serial_offset=0,
    serial_window_start=0,
    serial_window_stop=-1,
    # Output
    verbosity=1,
):
    """
    Wrapper for arctic's remove_cti() in src/cti.cpp, see its documentation.

    Remove CTI trails from an image by first modelling the addition of CTI, for
    parallel and/or serial clocking.

    This wrapper extracts individual numbers and arrays from the user-input
    objects to pass to the C++ via Cython. See cy_remove_cti() in
    arcticpy/wrapper.pyx and remove_cti() in arcticpy/interface.cpp.
    """
    image = np.copy(image).astype(np.double)

    # Extract inputs to pass to the wrapper

    # Parallel
    if parallel_traps is not None:
        # Extract trap inputs
        parallel_traps_standard = [
            trap for trap in parallel_traps if type(trap) == Trap
        ]
        parallel_traps_instant_capture = [
            trap for trap in parallel_traps if type(trap) == TrapInstantCapture
        ]
        parallel_n_traps_standard = len(parallel_traps_standard)
        parallel_n_traps_instant_capture = len(parallel_traps_instant_capture)
        if parallel_n_traps_standard + parallel_n_traps_instant_capture != len(
            parallel_traps
        ):
            raise Exception(
                "Not all traps extracted successfully (%d standard, %d instant capture, %d total)"
                % (
                    parallel_n_traps_standard,
                    parallel_n_traps_instant_capture,
                    len(parallel_traps),
                )
            )
        # Make sure the order is correct
        parallel_traps = parallel_traps_standard + parallel_traps_instant_capture
        parallel_trap_densities = np.array(
            [trap.density for trap in parallel_traps], dtype=np.double
        )
        parallel_trap_release_timescales = np.array(
            [trap.release_timescale for trap in parallel_traps], dtype=np.double
        )
        parallel_trap_capture_timescales = np.array(
            [trap.capture_timescale for trap in parallel_traps], dtype=np.double
        )
    else:
        # No parallel clocking, set dummy variables instead
        parallel_roe = ROE()
        parallel_ccd = CCD([CCDPhase(0.0, 0.0, 0.0)], [0.0])
        parallel_trap_densities = np.array([0.0], dtype=np.double)
        parallel_trap_release_timescales = np.array([0.0], dtype=np.double)
        parallel_trap_capture_timescales = np.array([0.0], dtype=np.double)
        parallel_n_traps_standard = 0
        parallel_n_traps_instant_capture = 0
        parallel_express = 0
        parallel_offset = 0
        parallel_window_start = 0
        parallel_window_stop = 0

    # Serial
    if serial_traps is not None:
        # Extract trap inputs
        serial_traps_standard = [trap for trap in serial_traps if type(trap) == Trap]
        serial_traps_instant_capture = [
            trap for trap in serial_traps if type(trap) == TrapInstantCapture
        ]
        serial_n_traps_standard = len(serial_traps_standard)
        serial_n_traps_instant_capture = len(serial_traps_instant_capture)
        if serial_n_traps_standard + serial_n_traps_instant_capture != len(
            serial_traps
        ):
            raise Exception(
                "Not all traps extracted successfully (%d standard, %d instant capture, %d total)"
                % (
                    serial_n_traps_standard,
                    serial_n_traps_instant_capture,
                    len(serial_traps),
                )
            )
        # Make sure the order is correct
        serial_traps = serial_traps_standard + serial_traps_instant_capture
        serial_trap_densities = np.array(
            [trap.density for trap in serial_traps], dtype=np.double
        )
        serial_trap_release_timescales = np.array(
            [trap.release_timescale for trap in serial_traps], dtype=np.double
        )
        serial_trap_capture_timescales = np.array(
            [trap.capture_timescale for trap in serial_traps], dtype=np.double
        )
    else:
        # No serial clocking, set dummy variables instead
        serial_roe = ROE()
        serial_ccd = CCD([CCDPhase(0.0, 0.0, 0.0)], [0.0])
        serial_trap_densities = np.array([0.0], dtype=np.double)
        serial_trap_release_timescales = np.array([0.0], dtype=np.double)
        serial_trap_capture_timescales = np.array([0.0], dtype=np.double)
        serial_n_traps_standard = 0
        serial_n_traps_instant_capture = 0
        serial_express = 0
        serial_offset = 0
        serial_window_start = 0
        serial_window_stop = 0

    # ========
    # Remove CTI
    # ========
    # Pass the extracted inputs to C++ via the cython wrapper
    return w.cy_remove_cti(
        image,
        n_iterations,
        # ========
        # Parallel
        # ========
        # ROE
        parallel_roe.dwell_times,
        parallel_roe.empty_traps_between_columns,
        parallel_roe.empty_traps_for_first_transfers,
        parallel_roe.force_release_away_from_readout,
        parallel_roe.use_integer_express_matrix,
        # CCD
        parallel_ccd.fraction_of_traps_per_phase,
        parallel_ccd.full_well_depths,
        parallel_ccd.well_notch_depths,
        parallel_ccd.well_fill_powers,
        # Traps
        parallel_trap_densities,
        parallel_trap_release_timescales,
        parallel_trap_capture_timescales,
        parallel_n_traps_standard,
        parallel_n_traps_instant_capture,
        # Misc
        parallel_express,
        parallel_offset,
        parallel_window_start,
        parallel_window_stop,
        # ========
        # Serial
        # ========
        # ROE
        serial_roe.dwell_times,
        serial_roe.empty_traps_between_columns,
        serial_roe.empty_traps_for_first_transfers,
        serial_roe.force_release_away_from_readout,
        serial_roe.use_integer_express_matrix,
        # CCD
        serial_ccd.fraction_of_traps_per_phase,
        serial_ccd.full_well_depths,
        serial_ccd.well_notch_depths,
        serial_ccd.well_fill_powers,
        # Traps
        serial_trap_densities,
        serial_trap_release_timescales,
        serial_trap_capture_timescales,
        serial_n_traps_standard,
        serial_n_traps_instant_capture,
        # Misc
        serial_express,
        serial_offset,
        serial_window_start,
        serial_window_stop,
        # Output
        verbosity,
    )