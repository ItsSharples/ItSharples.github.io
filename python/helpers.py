import os
from pathlib import Path


def readPathIntoString(url: Path):
    with url.open("r") as file:
        return file.read()


def writeStringIntoPath(string: str, url: Path):
    with url.open("w") as out:
        out.write(string)


def normalise(string: str):
    return string.strip().lower()


def searchDir(path: Path, forExtension: str = "html") -> list[Path]:
    '''Takes a Path and Returns all files that have the given extension (including subdirectories)'''
    out: list[Path] = []
    if path.is_file():
        out.append(path)
    for project in path.iterdir():
        if project.name == "data":
            continue
        if forExtension in project.suffixes:
            out.append(project)
            continue
        if project.is_dir():
            out += (searchDir((path / project.name), forExtension))
            continue
    return out


def getFileExtensionOfPath(path: str):
    return os.path.splitext(path)[1][1:]