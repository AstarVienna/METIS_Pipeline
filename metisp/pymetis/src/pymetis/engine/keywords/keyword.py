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

FITS header keywords as objects.

A `Keyword` is one card of the DRLD's FITS-keyword appendix: the canonical dotted name
('INS.OPTI10.NAME', 'MJD-OBS') and what the card says about it (type, unit, printf format,
default, range, description). Every other spelling is derived from the name, following the
rule EDPS applies (`edps.generator.fits.long_keyword`): the header card is
'ESO INS OPTI10 NAME' (a bare name stays bare), astropy's form is 'HIERARCH ESO ...', the
workflow package spells it 'ins.opti10.name', which is also the DRLD's hyperlink label.

Keywords are frozen instances, not classes: they carry no value, live in frozensets and dict
keys, and are compared by name alone. The instrument declares them once, in one module
(`pymetis.instruments.metis.keywords`), and everything else refers to those objects, so a
misspelt keyword is an AttributeError at import instead of a silent mismatch.

`cpl` is imported lazily: the module must stay importable without it, because the EDPS
workflow package's strings are cross-checked against the vocabulary from a plain interpreter.
"""
import re
from dataclasses import dataclass, field, replace
from typing import Any, ClassVar

_UNSET = object()

CANONICAL = re.compile(r'[A-Z](?:[A-Z0-9_-]|\{[a-z]\})*(?:\.(?:[A-Z0-9_-]|\{[a-z]\})+)*')   # A-Z0-9_- components, {n} placeholders
PLACEHOLDER = re.compile(r'\{(\w+)\}')


@dataclass(frozen=True)
class Keyword:
    """
    One FITS header keyword of the pipeline's interface.

    `name` is the canonical dotted form; an index placeholder ('SEQ.WCU.LASER{n}.WLEN')
    makes it a template, resolved with `keyword[n]` into an unregistered instance whose
    `.template` points back. The descriptive fields never take part in comparison or
    hashing: `kw[2] == kw[2]`, and a resolved instance equals a registered one of the same
    name.
    """
    name: str
    type: type = str                                    # python type; CPL and DRLD types are derived from it
    description: str = field(default='', compare=False)
    comment: str = field(default='', compare=False)     # the card's "Comment"; may hold newlines
    unit: str | None = field(default=None, compare=False)         # None: dimensionless / not applicable
    default: Any = field(default=None, compare=False)
    range: Any = field(default=None, compare=False)     # free text or (min, max)
    format: str | None = field(default=None, compare=False)       # printf; None: FORMATS[type]
    context: str | None = field(default=None, compare=False)      # None: first dotted part, 'FITS' for bare cards
    index: 'range | None' = field(default=None, compare=False)    # the valid n of a template (quoted: `range` is a field name above)
    labels: tuple[str, ...] = field(default=(), compare=False)    # extra \label targets the DRLD text refers to
    template: 'Keyword | None' = field(default=None, compare=False, repr=False)

    registry: ClassVar[dict[str, 'Keyword']] = {}       # canonical name -> keyword (templates included, resolved excluded)
    FORMATS: ClassVar[dict[type, str]] = {float: '%.3f', int: '%i', str: '%.50s', bool: '%i'}

    def __post_init__(self):
        if not CANONICAL.fullmatch(self.name):
            raise ValueError(f"{self.name!r} is not a canonical dotted keyword name (e.g. 'INS.OPTI10.NAME')")
        if self.index is not None and not self.is_template:
            raise ValueError(f"{self.name}: an index range needs a {{n}} placeholder in the name")
        if self.template is None:
            existing = self.registry.get(self.name)
            if existing is not None and existing is not self and not self._same_as(existing):
                raise TypeError(f"keyword {self.name} is defined twice, differently; every keyword has exactly one declaration")
            self.registry[self.name] = self

    def _same_as(self, other: 'Keyword') -> bool:
        """ An identical redefinition (a module reloaded under test) is not a second declaration. """
        return all(getattr(self, f) == getattr(other, f) for f in
                   ('name', 'type', 'description', 'comment', 'unit', 'default', 'range', 'format', 'context', 'index', 'labels'))

    # --- identity ---

    def __hash__(self) -> int:
        return hash(self.name)

    def __eq__(self, other) -> bool:
        return isinstance(other, Keyword) and other.name == self.name

    def __lt__(self, other: 'Keyword') -> bool:
        return self.name < other.name

    def __str__(self) -> str:
        return self.name

    # --- the spellings ---

    @property
    def dotted(self) -> str:
        """ 'INS.OPTI10.NAME': the canonical form, the DRLD's \\FITS{} argument. """
        return self.name

    @property
    def header(self) -> str:
        """ 'ESO INS OPTI10 NAME': how cpl.core.PropertyList keys the card; a bare name ('MJD-OBS') stays bare. """
        return f"ESO {self.name.replace('.', ' ')}" if '.' in self.name else self.name

    @property
    def hierarch(self) -> str:
        """ 'HIERARCH ESO INS OPTI10 NAME': how astropy keys it. """
        return f"HIERARCH {self.header}" if '.' in self.name else self.name

    @property
    def edps(self) -> str:
        """ 'ins.opti10.name': how the EDPS workflow package spells it (its long form is upper() + dots to spaces). """
        return self.name.lower()

    @property
    def shown(self) -> str:
        """ As the DRLD writes a template: 'SEQ.WCU.LASERn.WLEN'. """
        return PLACEHOLDER.sub(lambda m: m.group(1).lower(), self.name)

    @property
    def label(self) -> str:
        """ The DRLD hyperlink label of the card: 'fits:' + this. """
        return self.shown.lower()

    @property
    def group(self) -> str:
        """ The card's Context: the first dotted component ('DET', 'INS', 'DPR'), 'FITS' for a bare name. """
        return self.context or (self.name.split('.')[0] if '.' in self.name else 'FITS')

    @property
    def printf(self) -> str:
        return self.format or self.FORMATS.get(self.type, '%s')

    # --- indexed keywords ---

    @property
    def is_template(self) -> bool:
        return '{' in self.name

    def __getitem__(self, n: int) -> 'Keyword':
        if not self.is_template:
            raise TypeError(f"{self.name} is not an indexed keyword")
        if self.index is not None and n not in self.index:
            raise IndexError(f"{self.name}: index {n} is outside {self.index}")
        return replace(self, name=PLACEHOLDER.sub(str(n), self.name), index=None, template=self)

    # --- lookups ---

    @classmethod
    def find(cls, name: str) -> 'Keyword | None':
        """ 'DET.DIT' -> the keyword; 'SEQ.WCU.LASER2.WLEN' -> the template resolved with n=2; None when unknown. """
        if (hit := cls.registry.get(name)) is not None:
            return hit
        for keyword in cls.registry.values():
            if keyword.is_template:
                pattern = PLACEHOLDER.sub(lambda _: r'(\d+)', re.escape(keyword.name).replace(r'\{', '{').replace(r'\}', '}'))
                if (m := re.fullmatch(pattern, name)) is not None:
                    # one placeholder resolves; a matrix keyword (CD{n}_{m}) is answered with its template
                    return keyword[int(m.group(1))] if len(m.groups()) == 1 else keyword
        return None

    @classmethod
    def from_edps(cls, spelling: str) -> 'Keyword | None':
        """ 'ins.opti10.name' -> the keyword. """
        return cls.find(spelling.upper())

    @classmethod
    def from_header(cls, card: str) -> 'Keyword | None':
        """ 'ESO INS OPTI10 NAME' or 'HIERARCH ESO INS OPTI10 NAME' or 'MJD-OBS' -> the keyword. """
        return cls.find(re.sub(r'^(HIERARCH )?ESO ', '', card).replace(' ', '.'))

    # --- header access ---

    def present(self, header) -> bool:
        return self.header in header

    def get(self, header, default: Any = _UNSET) -> Any:
        """ The card's value coerced to the declared type; KeyError naming both spellings when absent and no default. """
        if self.header not in header:
            if default is _UNSET:
                raise KeyError(f"{self.header} ({self.name}) is not in the header")
            return default
        return self.type(header[self.header].value)

    def as_property(self, value: Any):
        import cpl
        from pymetis.engine.core.functions.property import python_to_cpl_type
        return cpl.core.Property(self.header, python_to_cpl_type(self.type), self.type(value), self.description)

    def set(self, header, value: Any) -> None:
        """ Write the card, replacing an existing one. """
        if self.header in header:
            header[self.header].value = self.type(value)
        else:
            header.append(self.as_property(value))


@dataclass(frozen=True)
class Alias(Keyword):
    """
    A DRS.* keyword of the DRLD: not a card the instrument writes but a name for the
    instrument keyword(s) a recipe matches on, since LM and N put their filter on different
    wheels. `combine='any'`: the value is that of whichever of `resolves_to` is in the header;
    `combine='all'`: the tuple of the values of every one present (a slit or a coronagraphic
    setup is several wheels at once), which is the DRLD's "value of A or B" against
    "combination of A, B, C". Simulated data writes the alias as a physical card; `get()`
    honours such a card first.
    """
    resolves_to: tuple[Keyword, ...] = field(default=(), compare=False)
    combine: str = field(default='any', compare=False)

    def __post_init__(self):
        if not self.name.startswith('DRS.'):
            raise ValueError(f"an alias is a DRS.* keyword, not {self.name}")
        if self.template is None:
            if not self.resolves_to:
                raise ValueError(f"{self.name} resolves to nothing")
            if any(isinstance(k, Alias) or k.is_template for k in self.resolves_to):
                raise ValueError(f"{self.name} must resolve to concrete keywords that are no aliases")
            if self.combine not in ('any', 'all'):
                raise ValueError(f"{self.name}: combine is 'any' or 'all', not {self.combine!r}")
        super().__post_init__()

    @property
    def edps_alternatives(self) -> tuple[str, ...]:
        """ What a workflow has to read instead of the alias, which EDPS cannot resolve. """
        return tuple(k.edps for k in self.resolves_to)

    def get(self, header, default: Any = _UNSET) -> Any:
        if self.header in header:
            return self.type(header[self.header].value)
        values = [k.get(header) for k in self.resolves_to if k.present(header)]
        if not values:
            if default is _UNSET:
                raise KeyError(f"{self.name}: none of {[k.header for k in self.resolves_to]} is in the header")
            return default
        return values[0] if self.combine == 'any' else tuple(values)
