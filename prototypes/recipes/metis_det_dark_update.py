from typing import Any, Dict

from cpl import core
from cpl import ui
from cpl import dfs
from cpl.core import Msg


class MetisDetDark(ui.PyRecipe):
    # Fill in recipe information
    _name = "metis_det_dark"
    _version = "0.1"
    _author = "Kieran Chi-Hung Hugo"
    _email = "hugo@buddelmeijer.nl"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Create master dark"
    _description = (
        "Prototype to create a METIS Masterdark."
    )

    def __init__(self) -> None:
        super().__init__()

        # The recipe will have a single enumeration type parameter, which allows the
        # user to select the frame combination method.
        self.parameters = ui.ParameterList(
            (
                ui.ParameterEnum(
                   name="metis_det_dark.stacking.method",
                   context="metis_det_dark",
                   description="Name of the method used to combine the input images",
                   default="average",
                    alternatives=("add", "average", "median"),
                ),
                ui.ParameterEnum(
                   name="metis_det_dark.do.catg",
                   context="metis_det_dark",
                   description="DO.CATG type to process",
                   default="DARK_LM_RAW",
                   alternatives=("DARK_LM_RAW", "DARK_N_RAW", "DARK_IFU_RAW"),
                    
                ),
            )
        )

    def extractFrames(self, frameset: ui.FrameSet, tag, extension):

        """given a frameset, a tag (DO.CATG) and extension number, extract the relevant frames and make a new frameset"""
        
        raw_frames = ui.FrameSet()

        for frame in frameset:
            # TODO: N and GEO
            print(frame.tag)
            if frame.tag == tag :
                frame.group = ui.Frame.FrameGroup.RAW
                raw_frames.append(frame)
                Msg.debug(self.name, f"Got raw frame: {frame.file}.")
            else:
                Msg.warning(
                    self.name,
                    f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.",
                )

                
        if len(raw_frames) == 0:
            raise core.DataNotFoundError("No raw frames in frameset.")

        return raw_frames

    def stackImages(self,raw_images, method):

        """given an imageset, stach using the specified method"""

        Msg.info(self.name, f"Combining images using method {method!r}")

        processed_images = raw_images
        combined_image = None

        if method == "add":
            for idx, image in enumerate(processed_images):
                if idx == 0:
                    combined_image = image
                else:
                    combined_image.add(image)
        elif method == "average":
            combined_image = processed_images.collapse_create()
        elif method == "median":
            combined_image = processed_images.collapse_median_create()
        else:
            Msg.error(
                self.name,
                f"Got unknown stacking method {method!r}. Stopping right here!",
                ## TODO exit methods
            )
        return processed_images, combined_image

    def extractImages(self, raw_frames, extension):

        """ given a frameset and extension number, extract the images from the files and make a fileset """

        header = None
        raw_images = core.ImageList()
        
        for idx, frame in enumerate(raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                header = core.PropertyList.load(frame.file, 0)
                extheader = core.PropertyList.load(frame.file, extension)

            Msg.debug(self.name, "Loading image.")
            raw_image = core.Image.load(frame.file, extension=extension)


            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            raw_images.insert(idx, raw_image)
            
        return raw_images, header, extheader
        
    def doSingleDark(self,frameset, tag, extension, method):


        """ given a frame set and extension, calculate the dark frame. can be called multiple times
            for IFU """

        raw_frames = self.extractFrames(frameset, tag, extension)
            

        print(raw_frames)
        raw_images, header, extheader = self.extractImages(raw_frames, extension)


        processed_images, combined_image = self.stackImages(raw_images, method)

        out_struct = {}
        out_struct['data'] = combined_image
        out_struct['error'] = combined_image
        out_struct['quality'] = combined_image
        out_struct['header'] = header
        out_struct['raw_frames'] = raw_frames
        out_struct['extheader'] = extheader
        
        return out_struct
    
        
    def run(self, frameset: ui.FrameSet, settings: Dict[str, Any]) -> ui.FrameSet:

        # create the frameset that will be returned. It is currently empty. 
        product_frames = ui.FrameSet()

        # check for user specified parameters
            
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(
                    self.name,
                    f"Settings includes {key}:{value} but {self} has no parameter named {key}.",
                )

        # set the user defined variables
            
        method = self.parameters["metis_det_dark.stacking.method"].value

        # this check is probably unnecessary and will be done by the parameterEnum check
        availableTags = ["DARK_LM_RAW","DARK_N_RAW_IFU_RAW"]
            
        tag = self.parameters['metis_det_dark.do.catg'].value
        if tag not in availableTags:
            Msg.debug(self.name, f"Tag value of {tag} incorrect")
            # TODO RETURN EMPTY VALUES

            
        # set the output file name
        if(tag == "DARK_LM_RAW"):
            output_file = "MASTER_DARK_LM.fits"
            catg = r"MASTER_DARK_2RG"
            next = 1
        elif(tag == "DARK_N_RAW"):
            output_file = "MASTER_DARK_N.fits"
            catg = r"MASTER_DARK_GEO"
            next = 1
        elif(tag == "DARK_IFU_RAW"):
            output_file = "MASTER_DARK_IFU.fits"
            catg = r"MASTER_DARK_IFU"
            next = 4

        print(tag, output_file, catg, next)
        # put the output into a list, then we can loop over them to handle 1 or 4 detectors
        output_list = []

        # for each detector, calculate the dark frame
        for i in range(1,next+1):
            output_list.append(self.doSingleDark(frameset, tag, i, method))

        # set the PRO.CATG in the propertylist
        product_properties = core.PropertyList()        
        product_properties.append(
                core.Property("ESO PRO CATG", core.Type.STRING, catg)
        )
        
        
        ### initialize the file by saving the propertylist ot create the primary header
        
        Msg.info(self.name, f"Saving product file as {output_file!r}.")
        dfs.save_propertylist(
            frameset,
            self.parameters,
            frameset,
            self.name,
            product_properties,
            f"demo/{self.version!r}",
            output_file,
            header=output_list[0]['header'],
        )
        
        ### then save the extensions

        for out_struct in output_list:
            extension_properties = out_struct['extheader']
            extension_properties.append(core.Property("HDUCLAS1",core.Type.STRING, r"IMAGE"))
            extension_properties.append(core.Property("HDUCLAS2",core.Type.STRING, r"DATA"))
            out_struct['data'].to_type(core.Type.INT).save(output_file, extension_properties, core.io.EXTEND)

            extension_properties = out_struct['extheader']
            extension_properties.append(core.Property("HDUCLAS1",core.Type.STRING, r"IMAGE"))
            extension_properties.append(core.Property("HDUCLAS2",core.Type.STRING, r"ERROR"))
            extension_properties.append(core.Property("HDUCLAS3",core.Type.STRING, r"RSME"))
            out_struct['error'].to_type(core.Type.INT).save(output_file, extension_properties, core.io.EXTEND)

            extension_properties = out_struct['extheader']
            extension_properties.append(core.Property("HDUCLAS1",core.Type.STRING, r"IMAGE"))
            extension_properties.append(core.Property("HDUCLAS2",core.Type.STRING, r"QUALITY"))
            extension_properties.append(core.Property("HDUCLAS3",core.Type.STRING, r"FLAG32BIT"))
            out_struct['quality'].to_type(core.Type.INT).save(output_file, extension_properties, core.io.EXTEND)

            

        # Register the created product
        product_frames.append(
            ui.Frame(
                file=output_file,
                tag=catg,
                group=ui.Frame.FrameGroup.PRODUCT,
                level=ui.Frame.FrameLevel.FINAL,
                frameType=ui.Frame.FrameType.IMAGE,
            )
        )

        return product_frames
