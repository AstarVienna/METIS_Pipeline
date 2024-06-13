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

            Msg.debug(self.name, "Loading image.")
            raw_image = core.Image.load(frame.file, extension=extension)


            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            raw_images.insert(idx, raw_image)
            
        return raw_images, header
        
    def doSingleDark(self,frameset, tag, extension, method):


        """ given a frame set and extension, calculate the dark frame. can be called multiple times
            for IFU """

        raw_frames = self.extractFrames(frameset, tag, extension)

        print(raw_frames)
        raw_images, header = self.extractImages(raw_frames, extension)


        processed_images, combined_image = self.stackImages(raw_images, method)

        return processed_images, combined_image, method, raw_frames, header
    
    
        
    def run(self, frameset: ui.FrameSet, settings: Dict[str, Any]) -> ui.FrameSet:

        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(
                    self.name,
                    f"Settings includes {key}:{value} but {self} has no parameter named {key}.",
                )
    
        method = self.parameters["metis_det_dark.stacking.method"].value
        #tag = self.parameters['tag'].value
        tag = "DARK_IFU_RAW"
        output_file = "MASTER_DARK_IFU.fits"
        
        # create the frameset(s)
        product_frames = ui.FrameSet()

        availableTags = ["DARK_LM_RAW","DARK_N_RAW","DARK_IFU_RAW"]
        
        if tag not in availableTags:
            Msg.debug(self.name, f"Tag value of {tag} incorrect")
            # TODO RETURN EMPTY VALUES



    
        if(tag == "DARK_LM_RAW" or tag == "N_DARK_RAW"):
            processed_images, combined_image, method, raw_frames, header = self.doSingleDark(frameset, tag, 1, method)

        # make this bit more elegant once I figure out how I want to do it.
        
        elif(tag == "DARK_IFU_RAW"):
            processed_images, combined_image1, method, raw_frames, header = self.doSingleDark(frameset, tag, 1, method)
            processed_images, combined_image2, method, raw_frames, header = self.doSingleDark(frameset, tag, 2, method)
            processed_images, combined_image3, method, raw_frames, header = self.doSingleDark(frameset, tag, 3, method)
            processed_images, combined_image4, method, raw_frames, header = self.doSingleDark(frameset, tag, 4, method)
            

        product_properties = core.PropertyList()

        if(tag == "DARK_LM_RAW"):
            product_properties.append(
                core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_DARK_2RG")
                )
        elif(tag == "N_DARK_RAW"):
            product_properties.append(
                core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_DARK_GEO")
            )
        elif(tag == "DARK_IFU_RAW"):
            product_properties.append(
                core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_DARK_IFU")
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
            header=header,
        )

        ### then save the extensions

        if(tag == "DARK_LM_RAW" or tag == "DARK_N_RAW"):
            combined_image.save(output_file, core.PropertyList(), core.io.EXTEND)
        elif(tag == "DARK_IFU_RAW"):
            combined_image1.save(output_file, core.PropertyList(), core.io.EXTEND)
            combined_image2.save(output_file, core.PropertyList(), core.io.EXTEND)
            combined_image3.save(output_file, core.PropertyList(), core.io.EXTEND)
            combined_image4.save(output_file, core.PropertyList(), core.io.EXTEND)

        # Register the created product
        product_frames.append(
            ui.Frame(
                file=output_file,
                tag="MASTER_DARK_2RG",
                group=ui.Frame.FrameGroup.PRODUCT,
                level=ui.Frame.FrameLevel.FINAL,
                frameType=ui.Frame.FrameType.IMAGE,
            )
        )


        return product_frames
