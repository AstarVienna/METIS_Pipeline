from typing import List

from edps import Job

from . import aspresso_keywords as kwd

# detmon requires different recipe parameters depending on the binning of the detector linearity data.

# regions for detector binning 1:
REG1 = [[25, 1540, 1176, 4616],
        [1265, 1540, 2416, 4616],
        [2505, 1540, 3656, 4616],
        [3745, 1540, 4896, 4616],
        [5025, 1540, 6176, 4616],
        [6265, 1540, 7416, 4616],
        [7505, 1540, 8656, 4616],
        [8745, 1540, 9896, 4616],
        [25, 4681, 1176, 7757],
        [1265, 4681, 2416, 7757],
        [2505, 4681, 3656, 7757],
        [3745, 4681, 4896, 7757],
        [5025, 4681, 6176, 7757],
        [6265, 4681, 7416, 7757],
        [7505, 4681, 8656, 7757],
        [8745, 4681, 9896, 7757]]

# regions for detector binning 2:
REG2 = [[13, 1540, 588, 4616],
        [633, 1540, 1208, 4616],
        [1253, 1540, 1828, 4616],
        [1873, 1540, 2448, 4616],
        [2513, 1540, 3088, 4616],
        [3133, 1540, 3708, 4616],
        [3753, 1540, 4328, 4616],
        [4373, 1540, 4948, 4616],
        [13, 4681, 588, 7757],
        [633, 4681, 1208, 7757],
        [1253, 4681, 1828, 7757],
        [1873, 4681, 2448, 7757],
        [2513, 4681, 3088, 7757],
        [3133, 4681, 3708, 7757],
        [3753, 4681, 4328, 7757],
        [4373, 4681, 4948, 7757]]

# regions for detector binning 4
REG4 = [[7, 770, 294, 2308],
        [317, 770, 604, 2308],
        [627, 770, 914, 2308],
        [937, 770, 1224, 2308],
        [1257, 770, 1544, 2308],
        [1567, 770, 1854, 2308],
        [1877, 770, 2164, 2308],
        [2187, 770, 2474, 2308],
        [7, 2341, 294, 3878],
        [317, 2341, 604, 3878],
        [627, 2341, 914, 3878],
        [937, 2341, 1224, 3878],
        [1257, 2341, 1544, 3878],
        [1567, 2341, 1854, 3878],
        [1877, 2341, 2164, 3878],
        [2187, 2341, 2474, 3878]]

# regions for detector binning 8
REG8 = [[4, 385, 147, 1154],
        [159, 385, 302, 1154],
        [314, 385, 457, 1154],
        [469, 385, 612, 1154],
        [629, 385, 772, 1154],
        [784, 385, 927, 1154],
        [939, 385, 1082, 1154],
        [1094, 385, 1237, 1154],
        [4, 1171, 147, 1940],
        [159, 1171, 302, 1940],
        [314, 1171, 457, 1940],
        [469, 1171, 612, 1940],
        [629, 1171, 772, 1940],
        [784, 1171, 927, 1940],
        [939, 1171, 1082, 1940],
        [1094, 1171, 1237, 1940]]


def reg_to_string(regions: List[List[int]]) -> str:
    return ':'.join([','.join([str(element) for element in row]) for row in regions])


def select_detmon_parameters(job: Job):
    regions = f'detmon.{job.command}.regions'
    saturation_limit = f'detmon.{job.command}.saturation_limit'
    gain_threshold = f'detmon.{job.command}.gain_threshold'
    binx = job.input_files[0].get_keyword_value(kwd.det_binx, None)

    if binx == 1:
        reg = reg_to_string(REG1)
        gain = 5000
        sat = 40000
    elif binx == 2:
        reg = reg_to_string(REG2)
        gain = 20000
        sat = 40000
    elif binx == 4:
        reg = reg_to_string(REG4)
        gain = 20000
        sat = 40000
    elif binx == 8:
        reg = reg_to_string(REG8)
        gain = 45000
        sat = 45000
    else:
        reg = reg_to_string(REG1)
        gain = 5000
        sat = 40000

    job.parameters.recipe_parameters[regions] = reg
    job.parameters.recipe_parameters[saturation_limit] = sat
    job.parameters.recipe_parameters[gain_threshold] = gain
