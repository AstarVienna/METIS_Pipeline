"""
High-level functions that are photometry related
"""
from __future__ import annotations
import cpl.core
import typing
__all__: list[str] = ['Unit', 'fill_blackbody']
class Unit:
    """
    Members:
    
      PHOTONRADIANCE
    
      ENERGYRADIANCE
    
      LESS
    
      LENGTH
    
      FREQUENCY
    """
    ENERGYRADIANCE: typing.ClassVar[Unit]  # value = <Unit.ENERGYRADIANCE: 106828722>
    FREQUENCY: typing.ClassVar[Unit]  # value = <Unit.FREQUENCY: 11>
    LENGTH: typing.ClassVar[Unit]  # value = <Unit.LENGTH: 3>
    LESS: typing.ClassVar[Unit]  # value = <Unit.LESS: 1>
    PHOTONRADIANCE: typing.ClassVar[Unit]  # value = <Unit.PHOTONRADIANCE: 7546>
    __members__: typing.ClassVar[dict[str, Unit]]  # value = {'PHOTONRADIANCE': <Unit.PHOTONRADIANCE: 7546>, 'ENERGYRADIANCE': <Unit.ENERGYRADIANCE: 106828722>, 'LESS': <Unit.LESS: 1>, 'LENGTH': <Unit.LENGTH: 3>, 'FREQUENCY': <Unit.FREQUENCY: 11>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
def fill_blackbody(out_unit: Unit, evalpoints: cpl.core.Vector, in_unit: Unit, temp: typing.SupportsFloat | typing.SupportsIndex) -> cpl.core.Vector:
    """
        The Planck radiance from a black-body
    
        Parameters
        ----------
        out_unit: cpl.drs.photom.Unit
            cpl.drs.photom.Unit.PHOTONRADIANCE, cpl.drs.photom.Unit.ENERGYRADIANCE or cpl.drs.photom.Unit.LESS
        evalpoints: cpl.core.Vector
            The evaluation points (wavelengths or frequencies)
        in_unit: cpl.drs.photom.Unit
            cpl.drs.photom.Unit.LENGTH or cpl.drs.photom.Unit.FREQUENCY
        temp: float
            The black body temperature [K]
    
        Return
        ------
        cpl.core.Vector
            The computed radiance
    
        Raises
        ------
        cpl.core.IncompatibleInputError
            if the size of evalpoints is different from the size of spectrum
        cpl.core.UnsupportedModeError
            if in_unit and out_unit are not as requested
        cpl.core.IllegalInputError
            if temp or a wavelength is non-positive
    
        Notes
        -----
        The Planck black-body radiance can be computed in 5 different ways:
        As a radiance of either energy [J*radian/s/m^3] or photons [radian/s/m^3],
        and in terms of either wavelength [m] or frequency [1/s]. The fifth way is
        as a unit-less radiance in terms of wavelength, in which case the area under
        the planck curve is 1.
        The dimension of the spectrum (energy or photons or unit-less, cpl.drs.photom.Unit.LESS)
        is controlled by out_unit, and the dimension of the input (length or
        frequency) is controlled by in_unit.
    
        evalpoints and spectrum must be of equal, positive length.
    
        The input wavelengths/frequencies and the temperature must be positive.
    
        The four different radiance formulas are:
        
        .. math::
            Rph1(\\lambda,T) = 2 \\pi \\frac{c}{\\lambda^4} (\\exp(hc/kT\\lambda)-1)^{-1}
    
        .. math::
            Rph2(\\nu,T) = 2 \\pi \\frac{\\nu^2}{c^4} (\\exp(h\\nu/kT)-1)^{-1}
    
        .. math::
            Re1(\\lambda,T) = 2 \\pi \\frac{hc^2}{\\lambda^5} (\\exp(hc/kT\\lambda)-1)^{-1} =
            \\frac{hc}{\\lambda} Rph1(\\lambda,T)
    
        .. math::
            Re2(\\nu,T) = 2 \\pi \\frac{h\\nu^3}{c^2} (\\exp(h\\nu/kT)-1)^{-1} = h\\nu Rph2(\\nu,T)
    
        .. math::
            R1(\\lambda,T) = \\frac{15h^5c^5}{\\pi^4k^5\\lambda^5T^5}
            (\\exp(hc/kT\\lambda)-1)^{-1} = \\frac{h^4c^3}{2\\pi^5k^5T^5} Rph1(\\lambda,T)
    
        where :math:`\\lambda` is the wavelength, :math:`\\nu` is the frequency,
        :math:`T` is the temperature, h is the Planck constant, k is the Boltzmann
        constant and c is the speed of light in vacuum.
    
        When the radiance is computed in terms of wavelength, the radiance peaks
        at :math:`\\lambda_{max} = 2.897771955\\times 10^{-3}/T` [m]. When the radiance
        is unit-less this maximum, :math:`R1(\\lambda_{max},T)`, is approximately 3.2648.
        :math:`R1(\\lambda,T)` integrated over l from 0 to infinity is 1.
    """
