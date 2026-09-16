"""
HDRL Debug submodule

  This module provides utilities for testing PyCPL to PyHDRL type converters.
  Not intended for use by pipeline developers.
  
"""
from __future__ import annotations
import cpl.core
import cpl.drs
__all__: list[str] = ['Types']
class Types:
    """
    
          A hdrl.debug.Types is a helper class to run tests for the custom type converters.
          The custom type caster handles the conversion dynamically. 
    
          It is not intended to be used by pipeline developers. 
    
          It is only intended for use by PyHDRL developers.
          
    """
    @staticmethod
    def image(image: cpl.core.Image) -> cpl.core.Image:
        """
              Utility function to help test the custom type caster for cpl.core.Image type.
        
              Parameters
              ----------
              image : cpl.core.Image
                    Image to be converted
        
              Returns
              -------
              cpl.core.Image
                    A newly allocated Image.
        """
    @staticmethod
    def imagelist(imagelist: cpl.core.ImageList) -> cpl.core.ImageList:
        """
              Utility function to help test the custom type caster for cpl.core.ImageList type.
        
              Parameters
              ----------
              imagelist : cpl.core.ImageList
                    ImageList to be converted
        
              Returns
              -------
              cpl.core.ImageList
                    A newly allocated ImageList.
        """
    @staticmethod
    def mask(m: cpl.core.Mask) -> cpl.core.Mask:
        """
              Utility function to help test the custom type caster for cpl.core.Mask type.
        
              Parameters
              ----------
              m : cpl.core.Mask
                    Mask to be converted
        
              Returns
              -------
              cpl.core.Mask
                    A newly allocated mask.
        """
    @staticmethod
    def propertylist(plist: cpl.core.PropertyList) -> cpl.core.PropertyList:
        """
              Utility function to help test the custom type caster for cpl.core.PropertyList type.
        
              Parameters
              ----------
              plist : cpl.core.PropertyList
                    PropertyList to be converted
        
              Returns
              -------
              cpl.core.PropertyList
                    A newly allocated PropertyList.
        """
    @staticmethod
    def table(tab: cpl.core.Table) -> cpl.core.Table:
        """
              Utility function to help test the custom type caster for cpl.core.Table type.
        
              Parameters
              ----------
              tab : cpl.core.Table
                    Table to be converted
        
              Returns
              -------
              cpl.core.Table
                    A newly allocated Table.
        """
    @staticmethod
    def vector(v: cpl.core.Vector) -> cpl.core.Vector:
        """
              Utility function to help test the custom type caster for cpl.core.Vector type.
        
              Parameters
              ----------
              v : cpl.core.Vector
                    Vector to be converted
        
              Returns
              -------
              cpl.core.Vector
                    A newly allocated Vector.
        """
    @staticmethod
    def wcs(w: cpl.drs.WCS) -> cpl.drs.WCS:
        """
              Utility function to help test the custom type caster for cpl.drs.WCS type.
        
              Parameters
              ----------
              w : cpl.drs.WCS
                    WCS to be converted
              Returns
              -------
              cpl.drs.WCS
                    A newly allocated WCS.
        """
    def __buffer__(self, flags):
        """
        Return a buffer object that exposes the underlying memory of the object.
        """
    def __init__(self) -> None:
        ...
    def __release_buffer__(self, buffer):
        """
        Release the buffer object that exposes the underlying memory of the object.
        """
