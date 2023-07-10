from dataclasses import dataclass
from pathlib import Path
import textwrap


@dataclass
class Project:
    url: Path
    preview: str
    page: str
    isHTML: bool

    def __repr__(self) -> str:
        return textwrap.shorten(self.preview, 20)
    
    def __str__(self) -> str:
        return self.preview
