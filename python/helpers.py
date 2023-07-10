from dataclasses import dataclass, field
import os
from pathlib import Path


@dataclass
class IncludeExclude():
    include: frozenset[str] = field(init=True)
    exclude: frozenset[str] = field(init=True)


def readPathIntoString(url: Path):
    with url.open("r") as file:
        return file.read()


def writeStringIntoPath(string: str, url: Path):
    with url.open("w") as out:
        out.write(string)


def normalise(string: str):
    return string.strip().lower()


def searchDirForExtensions(path: Path,
                               fileConfig: IncludeExclude, 
                               folderConfig: IncludeExclude):
        foundFiles: list[str] = []
        excludedFiles: list[str] = []
        foundExtensions: set[str] = set()

        if os.path.isfile(path):
            foundFiles.append(path.name)

        # subfiles: dict[str, Path | list[Path]] = {'root': path,'dirs': [], 'files': []}
        # for child in searchDir(path, "*"):
        #     if child.is_dir():
        #         subfiles['dirs'].append(child)
        #         continue
        #     if child.is_file():
        #         print(child)

        #         lastParent = Path(".")
        #         for parent in reversed(child.parents):
        #             if lastParent / "assets" == parent:
        #                 print("Assets Folder!")
        #             print(parent)
        #         subfiles['files'].append(child)
        #         continue
            

        usedDirs = []
        for root, dirs, files in os.walk(path):
            rootPath = Path(root)
            relativeRoot = getPathRelativeToRootChild(rootPath)
            reinclude = set(dirs) & folderConfig.include
            # Exclude from dirs, all subdirs starting with . or in the exclude set,
            #   and then reinclude any that were excluded. Include overrides Exclude
            dirs[:] = (set(dirs)
                       - set([dir for dir in dirs if dir.startswith('.')])
                       - folderConfig.exclude) | reinclude

            for file in files:
                fileExtension = getFileExtensionOfString(file)
                filePath = Path(os.path.join(relativeRoot, file))
                foundExtensions.add(fileExtension)

                if fileExtension in fileConfig.include:
                    foundFiles.append(filePath)
                    continue

                if fileExtension in fileConfig.exclude:
                    excludedFiles.append(filePath)
                    continue
                
                # File wasn't referenced, so is disused
                excludedFiles.append(filePath)

            usedDirs.append(relativeRoot)
        unusedExtensions = foundExtensions ^ (fileConfig.include | fileConfig.exclude)
        print(f"WARNING: Found Extensions: {unusedExtensions} that were not explicitly included or excluded from the project")

        return usedDirs, foundFiles, excludedFiles, unusedExtensions


def searchDir(path: Path, forExtension: str = "*", recursive: bool = True):
    '''Takes a Path and Returns all files that have the given extension (including subdirectories)'''
    if forExtension == "*":
        extension = ""
    else:
        extension = forExtension if forExtension[0] == '.' else f".{forExtension}"
    
    if recursive:
        return list(path.rglob(f"*{extension}"))
    else:
        return list(path.glob(f"*{extension}"))
    
    
    out: list[Path] = []
    if path.is_file():
        out.append(path)
    for project in path.iterdir():
        if project.name == "data":
            continue
        if extension in project.suffixes:
            out.append(project)
            continue
        if project.is_dir():
            out += (searchDir((path / project.name), forExtension))
            continue
    return out


def getFileExtensionOfPath(path: Path):
    return path.suffix[1:]
def getFileExtensionOfString(path: str):
    return os.path.splitext(path)[1][1:]

def getPathRelativeToRootChild(path: Path):
    return path.relative_to(*path.parts[:1])