from dataclasses import dataclass


@dataclass
class Project:
    url: str
    preview: str
    page: str
    isHTML: bool

    def __repr__(self) -> str:
        return self.url
