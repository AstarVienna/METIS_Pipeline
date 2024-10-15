from prefabricates.flat import MetisBaseImgFlatImpl


class MetisNImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band = "N"
        detector = "GEO"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "N"
