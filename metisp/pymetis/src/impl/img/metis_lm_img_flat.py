from prefabricates.flat import MetisBaseImgFlatImpl


class MetisLmImgFlatImpl(MetisBaseImgFlatImpl):
    class InputSet(MetisBaseImgFlatImpl.InputSet):
        band = "LM"
        detector = "2RG"

    class Product(MetisBaseImgFlatImpl.Product):
        band: str = "LM"
