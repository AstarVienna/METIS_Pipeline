from collections import defaultdict

from edps import JobParameters, get_parameter, Job


########################################################################################################################
###         Functions that define conditions depending on the values of the the workflow parameters                  ###
########################################################################################################################

#def use_telluric_standard(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'false'


#def use_molecfit(params: JobParameters):
#    return get_parameter(params, 'molecfit') != 'false'


#def molecfit_on_standard(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'standard'


#def molecfit_on_science(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'science'

def on_science (params : JobParameters) -> bool:
    return get_parameter(params, "molecfit") == "science"

def on_standard (params: JobParameters) -> bool:
    return get_parameter(params, "molecfit") == "standard"

def instrument_to_linlimit(job : Job):
    linlimit = f'{job.command}.linlimit'
    subinstrument = job.input_files[0].get_keyword_value("dpr.tech", None)
    if "LM" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 22100 # this will also need to be adapted based on readoutmode, as that will change the saturation limit and gain
    elif "N" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 13000
    elif "IFU" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 44000
    else:
        print("do not recognize instrument, using default value")


def _classify_lingain_frames(files):
    """Return (on_indices, off_indices, dits) for a list of DETLIN raw files.

    Classification uses the ESO DRS FILTER keyword: 'open' marks a
    closed-shutter dark (OFF), any other filter value is illuminated (ON).
    Note: 'open' currently represents the closed position because of the
    Shutter hack in METIS_Simulations
    """
    dits, on_idx, off_idx = [], [], []
    for i, f in enumerate(files):
        dits.append(f.get_keyword_value("det.dit", None))
        fw = (f.get_keyword_value("drs.filter", "") or "").strip()
        if fw == "open":
            off_idx.append(i)
        elif fw:
            on_idx.append(i)
    return on_idx, off_idx, dits


def prefilter_lingain_inputs(job: Job) -> None:
    """Drop DETLIN raws that cannot participate in a valid (ON, OFF) pair.

    The lingain recipe needs, for every DIT used, at least 2 ON frames and
    2 OFF frames at that DIT.
    """
    files = list(job.input_files)
    if not files:
        return

    on_idx, off_idx, dits = _classify_lingain_frames(files)

    # If the FILTER keyword leaves the ON or OFF array empty, don't trim anything.
    # Otherwise it would require checking pixel data, which is already done in the recipe for IFU inputs (working for LM/N) and would introduce overhead here.
    if not on_idx or not off_idx:
        return

    by_dit_on = defaultdict(list)
    by_dit_off = defaultdict(list)
    for i in on_idx:
        if dits[i] is not None:
            by_dit_on[dits[i]].append(i)
    for i in off_idx:
        if dits[i] is not None:
            by_dit_off[dits[i]].append(i)

    keep_indices = set()
    for dit, on_at_dit in by_dit_on.items():
        off_at_dit = by_dit_off.get(dit, [])
        if len(on_at_dit) >= 2 and len(off_at_dit) >= 2:
            keep_indices.update(on_at_dit)
            keep_indices.update(off_at_dit)

    job.input_files = [files[i] for i in sorted(keep_indices)]


def setup_lingain_job(job: Job) -> None:
    """Combined job-processing for lingain tasks: set linlimit, then prefilter inputs."""
    instrument_to_linlimit(job)
    prefilter_lingain_inputs(job)

