import argparse
import os
from pathlib import Path
import re
import shutil
import yaml
from dataclasses import dataclass, field
from buildProjects import buildProjects
from helpers import readPathIntoString, writeStringIntoPath, getFileExtensionOfPath


@dataclass
class IncludeExclude():
    include: frozenset[str] = field(init=True)
    exclude: frozenset[str] = field(init=True)


def setupParserAndParse():
    # Default Arguments
    defaultConfigPath = "_config.yml"
    defaultSourcePath = "."
    defaultBuildPath = "_site"
    defaultTemplatesPath = "templates"
    defaultTypes = ["png", "html", "js", "css"]
    # Process Arguments
    parser = argparse.ArgumentParser(
        description='Build this Website, and Move it to an output path.')
    parser.add_argument('-c', '--config', metavar='config', type=str, dest='config', default=defaultConfigPath,
                        nargs='?', help=f"The location of the config file, if used. Default: {defaultConfigPath}")
    parser.add_argument('-o', '--output', metavar='path', type=str, dest='buildPath', const=None,
                        nargs='?', help=f"The output destination relative to '.'. Default: {defaultBuildPath}")
    parser.add_argument('-s', '--source', metavar='path', type=str, dest='sourcePath', const=None,
                        nargs='?', help=f"The source path relative to '.'. Default: {defaultSourcePath}")
    parser.add_argument('-t', '--templates', metavar='path', type=str, dest='templatePath', const=None,
                        nargs='?', help=f"The templates path relative to '.'. Default: {defaultTemplatesPath}")
    fileGroup = parser.add_argument_group('Files')
    fileGroup.add_argument('-i', '--include-types', metavar='type', type=str, dest='includeFiles', const=None,
                           nargs='*', help=f"Types to include in the output path. Default: {defaultTypes}")
    fileGroup.add_argument('--exclude-types', metavar='type', type=str, dest='excludeFiles', const=None,
                           nargs='*', help=f"Types to exclude in the output path. Default: {None}")
    folderGroup = parser.add_argument_group('Folders')
    folderGroup.add_argument('--include-folders', metavar='folder', type=str, dest='includeFolders', const=None,
                             nargs='*', help=f"Folders to include in the output path. Default: {None}")
    folderGroup.add_argument('-e', '--exclude-folders', metavar='folder', type=str, dest='excludeFolders', const=None,
                             nargs='*', help=f"Folders to exclude in the output path. Default: {'Files starting with .'}")

    parser.add_help = True
    args = parser.parse_args()

    with open(args.config, 'r') as config:
        yamlConfig: dict = yaml.safe_load(config.read())

    # Read from yaml if the command line does not indicate a preference
    if args.sourcePath == None:
        args.sourcePath = yamlConfig.get("sourceDir", defaultSourcePath)
    if args.buildPath == None:
        args.buildPath = yamlConfig.get("destinationDir", defaultBuildPath)
    if args.templatePath == None:
        args.templatePath = yamlConfig.get(
            "templatePath", defaultTemplatesPath)

    if args.includeFiles == None:
        args.includeFiles = yamlConfig.get("includeFiles", defaultTypes)
    if args.excludeFiles == None:
        args.excludeFiles = yamlConfig.get("excludeFiles", [])

    if args.includeFolders == None:
        args.includeFolders = yamlConfig.get("includeFolders", [])
    if args.excludeFolders == None:
        args.excludeFolders = yamlConfig.get("excludeFolders", [])

    args.excludeFolders = set(args.excludeFolders) | set(
        ['__pycache__', 'python', args.buildPath])

    return args, yamlConfig


def main():
    args, yamlConfig = setupParserAndParse()

    try:
        patternStrings = yamlConfig["patterns"]
        templatePaths = yamlConfig["templates"]
    except KeyError as exception:
        print(f"KeyError: Missing {exception} from the config")
        raise

    folderConfig = IncludeExclude(
        include=frozenset(args.includeFolders),
        exclude=frozenset(args.excludeFolders)
    )

    fileConfig = IncludeExclude(
        include=frozenset(args.includeFiles),
        exclude=frozenset(args.excludeFiles)
    )

    def searchDirForExtensions(path: Path,
                               fileConfig: IncludeExclude = fileConfig, 
                               folderConfig: IncludeExclude = folderConfig):
        foundFiles: list[str] = []
        excludedFiles: list[str] = []
        foundExtensions: set[str] = set()

        if os.path.isfile(path):
            foundFiles.append(path.name)

        usedDirs = []
        for root, dirs, files in os.walk(path):
            relativeRoot = root[len(path)+1:]
            reinclude = set(dirs) & folderConfig.include
            # Exclude from dirs, all subdirs starting with . or in the exclude set,
            #   and then reinclude any that were excluded. Include overrides Exclude
            dirs[:] = (set(dirs)
                       - set([dir for dir in dirs if dir.startswith('.')])
                       - folderConfig.exclude) | reinclude

            for file in files:
                fileExtension = getFileExtensionOfPath(file)
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

    print(f"Building from {args.sourcePath} into {args.buildPath}")

    dirs, includedFiles, excludedFiles, unusedExtensions = searchDirForExtensions(
        args.sourcePath, fileConfig, folderConfig)
    
    noncompiledHTML: list[tuple[str, Path]] = []
    toCopy: list[str] = []
    for copy in includedFiles:
        to = Path(args.buildPath, copy)
        if getFileExtensionOfPath(to) == "html":
            contents = readPathIntoString(Path(args.sourcePath, copy))
            noncompiledHTML.append((contents, to))
        else:
            toCopy.append(copy)

    ### TODO: Diff the content?
    # Delete the old contents at Destination
    shutil.rmtree(args.buildPath, ignore_errors=True)
    os.mkdir(args.buildPath)

    # Rebuild the Tree
    for dir in set(dirs):
        try:
            os.makedirs(Path(args.buildPath, dir))
        except FileExistsError:
            continue

    # Add the leaves
    for copy in toCopy:
        fromPath = Path(args.sourcePath, copy)
        toPath = Path(args.buildPath, copy)
        print(f"Copying {fromPath} to {toPath}")
        shutil.copyfile(fromPath, toPath)

    # Build the projects
    preview, pages = buildProjects(patternStrings, templatePaths, Path(args.sourcePath),
                  Path(args.buildPath), args.templatePath)
    
    unusedFilePatterns = {ext: re.compile(
        fr"\.({ext})", flags=re.IGNORECASE) for ext in unusedExtensions}
    foundPatterns = set()
    for page, path in pages + noncompiledHTML:
        # Search for values that they may depend on that the build won't copy
        for (ext, pattern) in unusedFilePatterns.items():
            found = re.search(pattern, page)
            if found: foundPatterns.add(ext)

        writeStringIntoPath(page, path)
    print(f"WARNING: These patterns were found in the HTML of your project, but were not included in the build: {foundPatterns}")

    # Write the preview file
    writeStringIntoPath(*preview)


if __name__ == "__main__":
    main()