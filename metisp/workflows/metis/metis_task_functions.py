from collections import defaultdict

import numpy as np
from astropy.io import fits

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

    Tries ESO DRS FILTER first ('open' = closed-shutter dark; anything else
    = illuminated). If every frame reports 'open', fall back to a median-pixel-flux split using the first
    image HDU of each file
    """
    dits, fws = [], []
    for f in files:
        dits.append(f.get_keyword_value("det.dit", None))
        fws.append((f.get_keyword_value("drs.filter", "") or "").strip())

    use_median = (not any(fw and fw != "open" for fw in fws))

    on_idx, off_idx = [], []
    if not use_median:
        for i, fw in enumerate(fws):
            if fw == "open":
                off_idx.append(i)
            elif fw:
                on_idx.append(i)
        return on_idx, off_idx, dits

    # Median-flux fallback: read the first 2D image HDU of each file.
    for i, f in enumerate(files):
        median = None
        try:
            with fits.open(f.file_path, memmap=True) as hdul:
                for hdu in hdul[1:]:
                    if hdu.data is not None and getattr(hdu.data, "ndim", 0) == 2:
                        median = float(np.median(hdu.data))
                        break
        except Exception:
            median = None
        if median is None:
            continue
        if median > 2000:
            on_idx.append(i)
        elif median < 2000:
            off_idx.append(i)
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

