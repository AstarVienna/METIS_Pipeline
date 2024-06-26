from typing import Any, Dict

from cpl import core
from cpl import ui
from cpl import dfs
from cpl.core import Msg


class ScienceDataProcessor(ui.PyRecipe):
    # Fill in recipe information
    _name = "metis_lm_basic_reduction"
    _version = "0.1"
    _author = "Chi-Hung Yan"
    _email = "chyan@asiaa.sinica.edu.tw"
    _copyright = "GPL-3.0-or-later"
    _synopsis = "Basic science image data processing"
    _description = (
        "The recipe combines all science input files in the input set-of-frames using\n"
        + "the given method. For each input science image the master bias is subtracted,\n"
        + "and it is divided by the master flat."
    )

    def __init__(self) -> None:
        super().__init__()

        # The recipe will have a single enumeration type parameter, which allows the
        # user to select the frame combination method.
        self.parameters = ui.ParameterList(
            (
                ui.ParameterEnum(
                    name="basic_reduction.stacking.method",
                    context="basic_reduction",
                    description="Name of the method used to combine the input images",
                    default="add",
                    alternatives=("add", "average", "median"),
                ),
            )
        )

    def run(self, frameset: ui.FrameSet, settings: Dict[str, Any]) -> ui.FrameSet:
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
        bias_frame = None
        flat_frame = None

        output_file = "OBJECT_REDUCED.fits"

        # Go through the list of input frames, check the tag and act accordingly
        for frame in frameset:
            if frame.tag == "LM_IMAGE_SCI_RAW":
                frame.group = ui.Frame.FrameGroup.RAW
                raw_frames.append(frame)
                Msg.debug(self.name, f"Got raw frame: {frame.file}.")
            elif frame.tag == "MASTER_DARK_2RG":
                frame.group = ui.Frame.FrameGroup.CALIB
                bias_frame = frame
                Msg.debug(self.name, f"Got bias frame: {frame.file}.")
            elif frame.tag == "MASTER_FLAT_LAMP":
                frame.group = ui.Frame.FrameGroup.CALIB
                flat_frame = frame
                Msg.debug(self.name, f"Got flat field frame: {frame.file}.")
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
        bias_image = None
        if bias_frame:
            bias_image = core.Image.load(bias_frame.file, extension=0)
            Msg.info(self.name, f"Loaded bias frame {bias_frame.file!r}.")
        else:
            #raise core.DataNotFoundError("No bias frame in frameset.")
            Msg.warning(self.name, "No bias frame in frameset.")

        flat_image = None
        if flat_frame:
            flat_image = core.Image.load(flat_frame.file, extension=0)
            Msg.info(self.name, f"Loaded flat frame {flat_frame.file!r}.")
        else:
            # raise core.DataNotFoundError("No flat frame in frameset.")
            Msg.warning(self.name, "No flat frame in frameset.")

        # Flat field preparation: subtract bias and normalize it to median 1
        Msg.info(self.name, "Preparing flat field")
        if flat_image:
            if bias_image:
                flat_image.subtract(bias_image)
            median = flat_image.get_median()
            flat_image.divide_scalar(median)

        header = None
        processed_images = core.ImageList()
        for idx, frame in enumerate(raw_frames):
            Msg.info(self.name, f"Processing {frame.file!r}...")

            if idx == 0:
                header = core.PropertyList.load(frame.file, 0)

            Msg.debug(self.name, "Loading image.")
            raw_image = core.Image.load(frame.file, extension=1)

            if bias_image:
                Msg.debug(self.name, "Bias subtracting...")
                raw_image.subtract(bias_image)

            if flat_image:
                Msg.debug(self.name, "Flat fielding...")
                raw_image.divide(flat_image)

            # Insert the processed image in an image list. Of course
            # there is also an append() method available.
            processed_images.insert(idx, raw_image)

        # Combine the images in the image list using the image stacking
        # option requested by the user.
        method = self.parameters["basic_reduction.stacking.method"].value
        Msg.info(self.name, f"Combining images using method {method!r}")

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
            )
            # Since we did not create a product we need to return an empty
            # ui.FrameSet object. The result frameset product_frames will do,
            # it is still empty here!
            return product_frames

        # Create property list specifying the product tag of the processed image
        product_properties = core.PropertyList()
        product_properties.append(
            core.Property("ESO PRO CATG", core.Type.STRING, r"OBJECT_REDUCED")
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
                tag="OBJECT_REDUCED",
                group=ui.Frame.FrameGroup.PRODUCT,
                level=ui.Frame.FrameLevel.FINAL,
                frameType=ui.Frame.FrameType.IMAGE,
            )
        )

        return product_frames
