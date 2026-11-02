"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

from pymetis.engine.core.parameter import ParameterList, ParameterEnum
from pymetis.engine.core.functions.dummy import create_dummy_table, create_dummy_header
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.recipes import Recipe
from pymetis.engine.inputs import PipelineInputSet, MultiplePipelineInput, PrimaryInputMixin

from pymetis.instruments.metis.mixins import BandLmMixin, Detector2rgMixin
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis import qc
from pymetis.instruments.metis import dataitems

import cpl, hdrl
from cpl.core import Msg
import copy

class MetisLmImgBackgroundImpl(BandLmMixin, Detector2rgMixin, MetisRecipeImpl):
    class InputSet(PipelineInputSet):
        class BasicReducedInput(PrimaryInputMixin, MultiplePipelineInput):
            Item = dataitems.BasicReduced

        #class SkyBasicReducedInput(MultiplePipelineInput):
        #    Item = dataitems.LmSkyBasicReduced

        basic_reduced: BasicReducedInput
        #sky_basic_reduced: SkyBasicReducedInput

    class ProductSet(PipelineProductSet):
        Bkg = dataitems.Background
        BkgSubtracted = dataitems.BackgroundSubtracted
        ObjectCatalog = dataitems.ObjectCatalog

    class Qc(QcParameterSet):
        Median = qc.background.QcLmImgBkgMedian
        MedianDev = qc.background.QcLmImgBkgMedianDeviation

        
    def process(self) -> set[DataItem]:

        # load data TODO get 3 extensions once BASIC_REDUCE is set up
        #science_images = self.inputset.basic_reduced.load_data('DET1.DATA')


        science_images = self.inputset.basic_reduced.load_data('DET1.DATA')
        Msg.info(self.__class__.__qualname__, f"Loaded {len(science_images)} science images")

        sky_images = self.inputset.basic_reduced.load_data('DET1.DATA')
        Msg.info(self.__class__.__qualname__, f"Loaded {len(sky_images)} sky images")

        # extract primary header
        
        primary_header = cpl.core.PropertyList()
        hh = primary_header.load(self.inputset.basic_reduced.frameset[0].file, 0)
        product_set: set[DataItem] = set()

        # we're doing extraction in pixel coordinates, so WCS should be the same
        # for all frames

        # WCS object for the catalogue generation
        # currently WCS is not in the headers for processed data; we need to append to the
        # header, not replace it. Therefore, passing None 
        # TODO revisit this when WCS of the data is more complete; we may
        # needs a separate WCS for each image.
        # the catalogue routine takes a CPL image

        #wcs = cpl.drs.WCS(cpl.core.PropertyList.load(self.inputset.basic_reduced.item.filename,1))
        wcs = None
        #table = create_dummy_table()
        header_bkg = create_dummy_header()
        header_bkg_subtracted = create_dummy_header()
        header_object_cat = create_dummy_header()

        science_images_hdrl = hdrl.core.ImageList()

        Msg.info(self.__class__.__qualname__, f"Turning CPL images into HDRL images")

        for i,image in enumerate(science_images):
           # temporary kludge: mask the zero masked edge region, otherwise the extraction is wonky
            # TODO: this should be moved somewhere general as a first stage mask
            
            mask = cpl.core.Mask.threshold_image(image, -10, 1, 1)
        
            #update cplImage Mask
            image.bpm = image.bpm | mask   

            
            # TODO proper Enhanced image reading once basic_reduce is merged
            
            noise = cpl.core.Image.zeros_like(image)
            noise.copy_into(image,0,0)
            noise.power(0.5)

            # eventually, this HDRL image list should be creating by a single import
            science_image = hdrl.core.Image(image,noise)
            science_images_hdrl.append(science_image)
        
        sky_list_hdrl = hdrl.core.ImageList()

        Msg.info(self.__class__.__qualname__, f"Processing background images")

        for i,image in enumerate(sky_images):

            Msg.info(self.__class__.__qualname__, f"Processing file {self.inputset.basic_reduced.frameset[i].file}")

            # temporary kludge: mask the zero masked edge region, otherwise the extraction is wonky
            # TODO: this should be moved somewhere general as a first stage mask
            
            mask = cpl.core.Mask.threshold_image(image, -10, 1, 1)
        
            #update cplImage Mask
            image.bpm = image.bpm | mask   


            # TODO proper Enhanced image reading once basic_reduce is merged
            
            noise = cpl.core.Image.zeros_like(image)
            noise.copy_into(image,0,0)
            noise.power(0.5)

            # eventually, this HDRL image list should be creating by a single import
            sky_image = hdrl.core.Image(image,noise)



            cplImage,catalogue  = self.maskPoints(sky_image.image,wcs)

            
            Msg.info(self.__class__.__qualname__, f"Extracted {catalogue.shape[0]} sources from the image")

            # dummy header at the moment; TODO followup
            product = self.ProductSet.ObjectCatalog(
            create_dummy_header(),
            Hdu(header_object_cat, catalogue, name='TABLE'),
            )
            product_set |= {product}

            
            # we recreate an HDRL image with the new masked image and the old error.
            # the underlying data haven't changed at all, so the error values are
            # still valie
            
            sky_list_hdrl.append(hdrl.core.Image(cplImage, sky_image.error))


        # and collapse to create the master background
        (background, contrib) = sky_list_hdrl.collapse_median()

        Msg.info(self.__class__.__qualname__, f"DDD {type(background)} {type(science_images_hdrl)}")

        # subtract from the science images
        
        science_images_hdrl.sub_image(background)

        # now setup the products
        for i,image in enumerate(science_images):

            # load the original headers
            primary_header = cpl.core.PropertyList()
            hh = primary_header.load(self.inputset.basic_reduced.frameset[i].file, 0)

            extension_header = cpl.core.PropertyList()
            hh = extension_header.load(self.inputset.basic_reduced.frameset[i].file, 1)
            
            
            product = self.ProductSet.BkgSubtracted(
                copy.deepcopy(primary_header),
                Hdu(extension_header, image, name='DET1.DATA'),
            )
            product_set |= {product}

        
        product = self.ProductSet.Bkg(
            primary_header,
            Hdu(header_bkg, background.image, name='DET1.DATA'),
        )

        product_set |= {product}

        # FixMe: compute the real QC values; None marks a parameter that is not available yet
        primary_header.append(self.collect_qc_parameters(
            self.Qc.Median(None),
            self.Qc.MedianDev(None),
        ))

        #return {product_bkg, product_bkg_subtracted, product_object_cat}
        return product_set

    def maskPoints(self,sky_image, wcs):

        """ detect point sources in an image and mask them """


        # create a catalogue object
        # TODO;  move the default paramters somewhere configurable
        # TODO: test default parameters against real data
        
        cat = hdrl.func.Catalogue(
            obj_min_pixels=10,
            obj_threshold=10.0,
            obj_deblending=True,
            obj_core_radius=2.0,
            bkg_estimate=True,  #better false, given that we expect smooth background
            bkg_mesh_size=64,
            bkg_smooth_fwhm=2.0,
            det_eff_gain=1.0,   # add mean of real gain?  We don't need values
            det_saturation=24816.0,  #update with real value TODO
            resulttype=7
        )


        # do the point source extraction
        result = cat.compute(sky_image, wcs=None) # hdrl funct catalogue result

        # possibly overly clever here, but use the segmentation map
        # to determine pixels in points sources and mask from here
        # if we want to make the excluded region a bit smaller, there
        # are mask filtering options that can do that
        
        segmap = result.segmentation_map # cpl Image
        catalogue = result.catalogue # cpl Image

        # mask anything that's not zero
        inmask = cpl.core.Mask.threshold_image(segmap, -0.01, 0.01, 0)

        # update cplImage mask
        sky_image.bpm = sky_image.bpm | inmask

        return sky_image, catalogue
        
    def maskOther(self,sky_image,thresh):
        
        """
        detect and mask non-point sources that are above a given threshold.
        This is superfluous now, but might be needed later to filter out things 
        like diffraction spikes that aren't picked up as point sources. 
        TODO - test against sky data 
        
        Threshold is in RMS units, above the main
        """

        imageRms = sky_image.get_stdev()
        imageMean = sky_image.get_mean()
        print(imageMean,imageRms)
        mask = cpl.core.Mask.threshold_image(sky_image, imageRms * thresh + imageMean, 1e10, 1)    

        sky_image.bpm = sky_image.bpm | mask
    
     

class MetisLmImgBackground(Recipe):
    _name = "metis_lm_img_background"
    _version = "0.1"
    _author = "Chi-Hung Yan, A*"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic reduction of raw exposures from the {band} imager"
    _description = "Something"

    parameters = ParameterList([
        ParameterEnum(
            name="metis_lm_img_background.stacking.method",
            context="metis_lm_img_background",
            description="Name of the method used to combine the input images",
            default="median",
            alternatives=("average", "median"),
        )
    ])

    _matched_keywords: frozenset[str] = frozenset({'DRS.FILTER'})
    _algorithm = """For a list of frames (science/std or sky) 
    perform source detection
    mask detected detected sources
    combined frames to create a background image
    subtract this image from all science images
    """

    Impl = MetisLmImgBackgroundImpl
