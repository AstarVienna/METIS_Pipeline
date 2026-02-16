from edps.test.base import BaseIT
from edps.test.json_meta_test import JSONMetaTest


class JSONTests(BaseIT, metaclass=JSONMetaTest):
    pass
