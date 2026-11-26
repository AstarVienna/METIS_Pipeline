"""
Numerical tests of `pymetis.drl.combine`: the stacking that every recipe relies on.
"""
import numpy as np
import pytest
from cpl.core import Image as CplImage, ImageList as CplImageList
from hdrl.core import Image as HdrlImage

from pymetis.drl.combine import combine_images, combine_images_cpl, combine_images_hdrl
from pymetis.drl.noise import estimate_noise_list


def stack(*values: float, shape=(4, 6)) -> CplImageList:
    """ One constant frame per value. """
    return CplImageList([CplImage(np.full(shape, value, dtype=np.float64)) for value in values])

def hdrl_stack(*values: float, shape=(4, 6)):
    """ One constant HDRL frame per value, with Poisson errors sqrt(value). """
    return estimate_noise_list(stack(*values, shape=shape), read_noise=0.0)

def sigclip_kwargs(**overrides) -> dict:
    """ A complete, valid set of sigma-clipping kwargs, with selected values overridden. """
    return {"kappalow": 3.0, "kappahigh": 3.0, "keepfrac": 0.5, "mode": "median"} | overrides

def hdrl_sigclip_kwargs(**overrides) -> dict:
    """ A complete, valid set of HDRL sigma-clipping kwargs, with selected values overridden. """
    return {"kappa_low": 3.0, "kappa_high": 3.0, "niter": 5} | overrides

class TestCombineCpl:
    @pytest.mark.parametrize("method, expected", [
        ("add", 13.0),
        ("average", 13.0 / 3),
        ("median", 2.0),
    ])
    def test_constant_frames_combine_to_the_expected_constant(self, method, expected):
        combined = combine_images(stack(1.0, 2.0, 10.0), method)
        assert isinstance(combined, CplImage)
        np.testing.assert_allclose(combined.as_array(), np.full((4, 6), expected))

    def test_median_ignores_a_single_outlier_frame(self):
        combined = combine_images(stack(5.0, 5.0, 5.0, 5000.0), "median")
        np.testing.assert_allclose(combined.as_array(), np.full((4, 6), 5.0))

    def test_unknown_method_is_rejected(self):
        with pytest.raises(ValueError):
            combine_images(stack(1.0, 2.0), "geometric-mean")


class TestCombineHdrl:
    def test_average_of_hdrl_frames_keeps_data_and_error(self):
        frames = estimate_noise_list(stack(4.0, 16.0), read_noise=0.0)
        combined = combine_images(frames, "average")
        assert isinstance(combined, HdrlImage)
        np.testing.assert_allclose(combined.image.as_array(), np.full((4, 6), 10.0))
        # errors are sqrt(4)=2 and sqrt(16)=4, propagated through the mean of two frames
        np.testing.assert_allclose(combined.error.as_array(),
                                   np.full((4, 6), np.hypot(2.0, 4.0) / 2), rtol=1e-6)

class TestCombineHdrlSigclip:
    @pytest.mark.parametrize("missing", ["kappa_low", "kappa_high", "niter"])
    def test_missing_kwarg_is_rejected(self, missing):
        kwargs = hdrl_sigclip_kwargs()
        del kwargs[missing]
        with pytest.raises(TypeError, match="sigclip needs exactly"):
            combine_images_hdrl(hdrl_stack(1.0, 2.0, 3.0), "sigclip", **kwargs)

    def test_misspelled_kwarg_is_rejected(self):
        # The CPL spelling must not be accepted by the HDRL path.
        kwargs = hdrl_sigclip_kwargs()
        kwargs["kappalow"] = kwargs.pop("kappa_low")
        with pytest.raises(TypeError, match="sigclip needs exactly"):
            combine_images_hdrl(hdrl_stack(1.0, 2.0, 3.0), "sigclip", **kwargs)

    def test_no_kwargs_is_rejected(self):
        with pytest.raises(TypeError):
            combine_images_hdrl(hdrl_stack(1.0, 2.0, 3.0), "sigclip")

    @pytest.mark.parametrize("method", ["average", "median"])
    def test_kwargs_with_other_method_are_rejected(self, method):
        with pytest.raises(TypeError, match="only accepted by method 'sigclip'"):
            combine_images_hdrl(hdrl_stack(1.0, 2.0, 3.0), method, **hdrl_sigclip_kwargs())

    def test_combine_images_forwards_sigclip_kwargs(self):
        combined = combine_images(hdrl_stack(*[5.0] * 20, 1000.0), "sigclip", **hdrl_sigclip_kwargs())
        np.testing.assert_allclose(combined.image.as_array(), np.full((4, 6), 5.0))

class TestCombineCplSigclip:
    @pytest.mark.parametrize("mode", ["median", "MEDIAN", "Median"])
    def test_mode_is_case_insensitive(self, mode):
        combined = combine_images_cpl(stack(5.0, 5.0, 5.0), "sigclip", **sigclip_kwargs(mode=mode))
        np.testing.assert_allclose(combined.as_array(), np.full((4, 6), 5.0))

    @pytest.mark.parametrize("missing", ["kappalow", "kappahigh", "keepfrac", "mode"])
    def test_missing_kwarg_is_rejected(self, missing):
        kwargs = sigclip_kwargs()
        del kwargs[missing]
        with pytest.raises(TypeError, match="sigclip needs exactly"):
            combine_images_cpl(stack(1.0, 2.0, 3.0), "sigclip", **kwargs)

    def test_misspelled_kwarg_is_rejected(self):
        kwargs = sigclip_kwargs()
        kwargs["kappa_low"] = kwargs.pop("kappalow")
        with pytest.raises(TypeError, match="sigclip needs exactly"):
            combine_images_cpl(stack(1.0, 2.0, 3.0), "sigclip", **kwargs)

    def test_no_kwargs_is_rejected(self):
        with pytest.raises(TypeError):
            combine_images_cpl(stack(1.0, 2.0, 3.0), "sigclip")

    @pytest.mark.parametrize("method", ["average", "median"])
    def test_kwargs_with_other_method_are_rejected(self, method):
        with pytest.raises(TypeError, match="only accepted by method 'sigclip'"):
            combine_images_cpl(stack(1.0, 2.0, 3.0), method, **sigclip_kwargs())


class TestAddDoesNotAliasInputs:
    def test_cpl_inputs_are_unchanged(self):
        from cpl.core import Image as CplImage, ImageList as CplImageList
        frames = CplImageList([CplImage(np.full((3, 3), 1.0)), CplImage(np.full((3, 3), 2.0))])
        combined = combine_images(frames, 'add')
        np.testing.assert_array_equal(combined.as_array(), 3.0)
        np.testing.assert_array_equal(frames[0].as_array(), 1.0)

    def test_hdrl_inputs_are_unchanged(self):
        from cpl.core import Image as CplImage, ImageList as CplImageList
        from pymetis.drl.noise import estimate_noise_list
        frames = estimate_noise_list(CplImageList([CplImage(np.full((3, 3), 1.0)), CplImage(np.full((3, 3), 2.0))]), 0.0)
        combined = combine_images(frames, 'add')
        np.testing.assert_array_equal(combined.image.as_array(), 3.0)
        np.testing.assert_array_equal(frames[0].image.as_array(), 1.0)
