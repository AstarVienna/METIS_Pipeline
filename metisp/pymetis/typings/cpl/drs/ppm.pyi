"""
Point Pattern Matching Module
"""
from __future__ import annotations
import cpl.core
import typing
__all__: list[str] = ['match_points', 'match_positions']
def match_points(data: cpl.core.Matrix, use_data: typing.SupportsInt | typing.SupportsIndex, err_data: typing.SupportsFloat | typing.SupportsIndex, pattern: cpl.core.Matrix, use_pattern: typing.SupportsInt | typing.SupportsIndex, err_pattern: typing.SupportsFloat | typing.SupportsIndex, tolerance: typing.SupportsFloat | typing.SupportsIndex, radius: typing.SupportsFloat | typing.SupportsIndex) -> typing.Any:
    """
        Match 2-D distributions of points.
    
        Parameters
        ----------
        data : cpl.core.Matrix
            List of data points (e.g., detected stars positions).
        use_data : int
            Number of `data` points used for preliminary match.
        err_data : float
            Error on `data` points positions.
        pattern : cpl.core.Matrix
            List of pattern points (e.g., expected stars positions).
        use_pattern : int
            Number of `pattern` points used for preliminary match.
        err_pattern : float
            Error on `pattern` points positions.
        tolerance :double
            Max relative difference of angles and scales from their median value for match 
            acceptance.
        radius : float
            Search radius applied in final matching (`data` units).
    
        Return
        ------
        NamedTuple(matches: List[int], mdata: cpl.core.Matrix, mpattern: cpl.core.Matrix, lin_scale: float, lin_angle: float)
        
            matches : list of int
                Indexes of identified data points (pattern-to-data).
            mdata : cpl.core.Matrix
                the list of identified `data` points
            mpattern,: cpl.core.Matrix
                the list of matching `pattern` points
            lin_scale : float
                the Linear transformation scale factor
            lin_angle : float
                the Linear transformation rotation angle
    
        Notes
        -----
        A point is described here by its coordinates on a cartesian plane.
        The input matrices `data` and `pattern` must have 2 rows, as
        their column vectors are the points coordinates.
    
        This function attemps to associate points in `data` to points in
        `pattern`, under the assumption that a transformation limited to
        scaling, rotation, and translation, would convert positions in
        `pattern` into positions in `data`. Association between points
        is also indicated in the following as "match", or "identification".
    
        Point identification is performed in two steps. In the first step
        only a subset of the points is identified (preliminary match). In
        the second step the identified points are used to define a first-guess
        transformation from `pattern` points to `data` points, that is
        applied to identify all the remaining points as well. The second
        step would be avoided if a `use_pattern` equal to the number of
        points in `pattern` is given, and exactly `use_pattern` points
        would be identified already in the first step.
    
        First step:
    
        All possible triangles (sub-patterns) are built using the first
        `use_data` points from `data` and the first `use_pattern`
        points from `pattern`. The values of `use_data` and `use_pattern`
        must always be at least 3 (however, see the note at the end),
        and should not be greater than the length of the corresponding
        lists of points. The point-matching algorithm goes as follow:
    
        For every triplet of points:
           Select one point as the reference. The triangle coordinates
           are defined by
    
                       ((Rmin/Rmax)^2, theta_min theta_max)
    
           where Rmin (Rmax) is the shortest (longest) distance from the
           reference point to one of the two other points, and theta_min
           (theta_max) is the view angle in [0; 2pi[ to the nearest
           (farthest) point.
    
           Triangles are computed by using each point in the triplet
           as reference, thereby computing 3 times as many triangles
           as needed.
    
           The accuracy of triangle patterns is robust against distortions
           (i.e., systematic inaccuracies in the points positions) of the
           second order. This is because, if the points positions had
           constant statistical uncertainty, the relative uncertainty in
           the triangle coordinates would be inversely proportional to
           the triangle size, while if second order distortions are
           present the systematic error on points position would be
           directly proportional to the triangle size.
    
        For every triangle derived from the `pattern` points:
           Match with nearest triangle derived from `data` points
           if their distance in the parameter space is less than their
           uncertainties (propagated from the points positions uncertainties
           `err_data` and `err_pattern`). For every matched pair of
           triangles, record their scale ratio, and their orientation
           difference. Note that if both `err_data` and `err_pattern`
           are zero, the tolerance in triangle comparison will also be
           zero, and therefore no match will be found.
    
        Get median scale ratio and median angle of rotation, and reject
        matches with a relative variation greater than `tolerance` from
        the median of either quantities. The estimator of all the rotation
        angles a_i is computed as::
    
                    atan(med sin(a_i) / med cos(a_i))
    
    
        Second step:
    
        From the safely matched triangles, a list of identified points is
        derived, and the best transformation from `pattern` points to
        `data` points (in terms of best rotation angle, best scaling
        factor, and best shift) is applied to attempt the identification of
        all the points that are still without match. This matching is made
        by selecting for each `pattern` point the `data` point which is
        closest to its transformed position, and at a distance less than
        `radius`.
    
        The returned array of integers is as long as the number of points in
        `pattern`, and each element reports the position of the matching
        point in `data` (counted starting from zero), or is invalid if no
        match was found for the `pattern` point. For instance, if element
        N of the array has value M, it means that the Nth point in `pattern`
        matches the Mth point in `data`. 
    
        Two more matrices `mdata` and `mpattern` will be returned with the 
        coordinates of the identified points. These two matrices will both 
        have the same size: 2 rows, and as many columns as successfully 
        identified points. Matching points will be in the same column of both 
        matrices.
    
        If `lin_scale` is returned with a good estimate
        of the scale (distance_in_data = lin_scaledistance_in_pattern).
        This makes sense only in case the transformation between `pattern`
        and `data` is an affine transformation. In case of failure,
        `lin_scale` is set to zero.
    
        If `lin_angle` is returned with a good
        estimate of the rotation angle between `pattern` and `data`
        in degrees (counted counterclockwise, from -180 to +180, and with
        data_orientation = pattern_orientation + lin_angle). This makes
        sense only in case the transformation between `pattern` and
        `data` is an affine transformation. In case of failure,
        `lin_angle` is set to zero.
    
        The returned values for `lin_scale` and `lin_angle` have the only
        purpose of providing a hint on the relation between `pattern` points
        and `data` points. This function doesn't attempt in any way to
        determine or even suggest a possible transformation between `pattern`
        points and `data` points: this function just matches points, and it
        is entriely a responsibility of the caller to fit the appropriate
        transformation between one coordinate system and the other.
        A polynomial transformation of degree 2 from `pattern` to `data`
        may be fit in the following way (assuming that `mpattern` and
        `mdata` are available):
    
        .. code-block:: python
    
            x = cpl.core.Vector(mdata[0,:])
            y = cpl.core.Vector(mdata[1,:])
            x_transform = cpl.core.Polynomial(2)
            y_transform = cpl.core.Polynomial(2)
            x_transform.fit(mpattern, x, False, [transform_x.degree,])
            y_transform.fit(mpattern, y, False, [transform_y.degree,])
    
        The basic requirement for using this function is that the searched
        point pattern (or at least most of it) is contained in the data.
        As an indirect consequence of this, it would generally be appropriate
        to have more points in `data` than in `pattern` (and analogously,
        to have `use_data` greater than `use_pattern`), even if this is
        not strictly necessary.
    
        Also, `pattern` and `data` should not contain too few points
        (say, less than 5 or 4) or the identification may risk to be incorrect:
        more points enable the construction of many more triangles, reducing
        the risk of ambiguity (multiple valid solutions). Special situations,
        involving regularities in patterns (as, for instance, input `data`
        containing just three equidistant points, or the case of a regular
        grid of points) would certainly provide an answer, and this answer
        would very likely be wrong (the human brain would fail as well,
        and for exactly the same reasons).
    
        The reason why a two steps approach is encouraged here is mainly to
        enable an efficient use of this function: in principle, constructing
        all possible triangles using all of the available points is never
        wrong, but it could become very slow: a list of N points implies the
        evaluation of N*(N-1)*(N-2)/2 triangles, and an even greater number
        of comparisons between triangles. The possibility of evaluating
        first a rough transformation based on a limited number of identified
        points, and then using this transformation for recovering all the
        remaining points, may significantly speed up the whole identification
        process. However it should again be ensured that the main requirement
        (i.e., that the searched point pattern must be contained in the data)
        would still be valid for the selected subsets of points: a random
        choice would likely lead to a matching failure (due to too few, or
        no, common points).
    
        A secondary reason for the two steps approach is to limit the effect
        of another class of ambiguities, happening when either or both of
        the input matrices contains a very large number of uniformely
        distributed points. The likelihood to find several triangles that
        are similar by chance, and at all scales and orientations, may
        increase to unacceptable levels.
    
        A real example may clarify a possible way of using this function:
        let `data` contain the positions (in pixel) of detected stars
        on a CCD. Typically hundreds of star positions would be available,
        but only the brightest ones may be used for preliminary identification.
        The input `data` positions will therefore be opportunely ordered
        from the brightest to the dimmest star positions. In order to
        identify stars, a star catalogue is needed. From a rough knowledge
        of the pointing position of the telescope and of the size of the
        field of view, a subset of stars can be selected from the catalogue:
        they will be stored in the `pattern` list, ordered as well by their
        brightness, and with their RA and Dec coordinates converted into
        standard coordinates (a gnomonic coordinate system centered on the
        telescope pointing, i.e., a cartesian coordinate system), no matter
        in what units of arc, and no matter what orientation of the field.
        For the first matching step, the 10 brightest catalogue stars may
        be selected (selecting less stars would perhaps be unsafe, selecting
        more would likely make the program slower without producing any
        better result). Therefore `use_pattern` would be set to 10.
        From the data side, it would generally be appropriate to select
        twice as many stars positions, just to ensure that the searched
        pattern is present. Therefore `use_data` would be set to 20.
        A reasonable value for `tolerance` and for `radius` would be
        respectively 0.1 (a 10% variation of scales and angles) and 20
        (pixels).
    """
def match_positions(peaks: cpl.core.Vector, lines: cpl.core.Vector, min_disp: typing.SupportsFloat | typing.SupportsIndex, max_disp: typing.SupportsFloat | typing.SupportsIndex, tolerance: typing.SupportsFloat | typing.SupportsIndex) -> cpl.core.Bivector:
    """
        Match 1-D patterns.
    
        Parameters
        ----------
        peaks : cpl.core.Vector
            List of observed positions (e.g., of emission peaks)
        lines : cpl.core.Vector
            List of positions in searched pattern (e.g., wavelengths)
        min_disp : float
            Min expected scale (e.g., spectral dispersion in A/pixel)
        max_disp : float
            Max expected scale (e.g., spectral dispersion in A/pixel)
        tolerance : float
            Tolerance for interval ratio comparison
    
        Return
        -------
        List of all matching points positions
    
        Notes
        -----
        This function attempts to find the reference pattern `lines` in a
        list of observed positions `peaks`. In the following documentation
        a terminology drawn from the context of arc lamp spectra calibration
        is used for simplicity: the reference pattern is then a list of
        wavelengths corresponding to a set of reference arc lamp emission
        lines the so-called line catalog; while the observed positions are
        the positions (in pixel) on the CCD, measured along the dispersion
        direction, of any significant peak of the signal. To identify the
        observed peaks means to associate them with the right reference
        wavelength. This is attempted here with a point-pattern matching
        technique, where the pattern is contained in the vector `lines`,
        and is searched in the vector `peak`.
    
        In order to work, this method just requires a rough expectation
        value of the spectral dispersion (in Angstrom/pixel), and a line
        catalog. The line catalog `lines` should just include lines that
        are expected somewhere in the CCD exposure of the calibration lamp
        (note, however, that a catalog including extra lines at its blue
        and/or red ends is still allowed).
    
        Typically, the arc lamp lines candidates `peak` will include
        light contaminations, hot pixels, and other unwanted signal,
        but only in extreme cases does this prevent the pattern-recognition
        algorithm from identifying all the spectral lines. The pattern
        is detected even in the case `peak` contained more arc lamp
        lines than actually listed in the input line catalog.
    
        This method is based on the assumption that the relation between
        wavelengths and CCD positions is with good approximation `locally`
        linear (this is always true, for any modern spectrograph).
    
        The ratio between consecutive intervals pairs in wavelength and in
        pixel is invariant to linear transformations, and therefore this
        quantity can be used in the recognition of local portions of the
        searched pattern. All the examined sub-patterns will overlap, leading
        to the final identification of the whole pattern, notwithstanding the
        overall non-linearity of the relation between pixels and wavelengths.
    
        Ambiguous cases, caused by exceptional regularities in the pattern,
        or by a number of undetected (but expected) peaks that disrupt the
        pattern on the data, are recovered by linear interpolation and
        extrapolation of the safely identified peaks.
    
        More details about the applied algorithm can be found in the comments
        to the function code.
    """
