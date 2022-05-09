from dataclasses import dataclass
from pathlib import Path


@dataclass
class Project:
    url: Path
    preview: str
    page: str
    isHTML: bool

    def __repr__(self) -> str:
        return self.preview
