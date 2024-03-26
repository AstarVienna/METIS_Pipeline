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

    def run(self, frameset: ui.FrameSet, settings: Dict[str, Any]) -> ui.FrameSet:
        print(42)
        # Update the recipe paramters with the values requested by the user through the
        # settings argument
        for key, value in settings.items():
            try:
                self.parameters[key].value = value
            except KeyError:
                Msg.warning(
                    self.name,
                    f"Settings includes {key}:{value} but {self} has no parameter named {key}.",
                )

        raw_frames = ui.FrameSet()
        product_frames = ui.FrameSet()

        # TODO: Detect detector
        output_file = "MASTER_DARK_2RG.fits"

        # Go through the list of input frames, check the tag and act accordingly
        for frame in frameset:
            # TODO: N and GEO
            if frame.tag == "DARK_LM_RAW":
                frame.group = ui.Frame.FrameGroup.RAW
                raw_frames.append(frame)
                Msg.debug(self.name, f"Got raw frame: {frame.file}.")
            else:
                Msg.warning(
                    self.name,
                    f"Got frame {frame.file!r} with unexpected tag {frame.tag!r}, ignoring.",
                )

        # For demonstration purposes we raise an exception here. Real world
        # recipes should rather print a message (also to have it in the log file)
        # and exit gracefully.
        if len(raw_frames) == 0:
            raise core.DataNotFoundError("No raw frames in frameset.")

        # By default images are loaded as Python float data. Raw image
        # data which is usually represented as 2-byte integer data in a
        # FITS file is converted on the fly when an image is loaded from
        # a file. It is however also possible to load images without
        # performing this conversion.


        # Flat field preparation: subtract bias and normalize it to median 1
        # Msg.info(self.name, "Preparing flat field")
        # if flat_image:
        #     if bias_image:
        #         flat_image.subtract(bias_image)
        #     median = flat_image.get_median()
        #     flat_image.divide_scalar(median)



        header = None
        raw_images = core.ImageList()

        for idx, frame in enumerate(raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                header = core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = core.Image.load(frame.file, extension=1)


            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            raw_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["metis_det_dark.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

        combined_image = None
        # TODO: preprocessing steps like persistence correction / nonlinearity (or not)
        processed_images = raw_images
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
            )
            # Since we did not create a product we need to return an empty
            # ui.FrameSet object. The result frameset product_frames will do,
            # it is still empty here!
            return product_frames

        # Create property list specifying the product tag of the processed image
        product_properties = core.PropertyList()
        product_properties.append(
            # TODO: Other detectors
                core.Property("ESO PRO CATG", core.Type.STRING, r"MASTER_DARK_2RG")
        )

        # Save the result image as a standard pipeline product file
        Msg.info(self.name, f"Saving product file as {output_file!r}.")
        dfs.save_image(
            frameset,
            self.parameters,
            frameset,
            combined_image,
            self.name,
            product_properties,
            f"demo/{self.version!r}",
            output_file,
            header=header,
        )

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
