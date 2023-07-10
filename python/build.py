import argparse
import os
from pathlib import Path
import re
import shutil
import yaml
from collections import defaultdict
from dataclasses import dataclass, field
from buildProjects import buildProjects
from compileHTML import createHTML
from helpers import readPathIntoString, writeStringIntoPath, getFileExtensionOfPath, \
    searchDir, searchDirForExtensions, IncludeExclude
from parsing import setupParserAndParse

def main():
    args, yamlConfig = setupParserAndParse()

    try:
        patternStrings = yamlConfig["patterns"]
        templatePaths = yamlConfig["templates"]

        templateFolder = args.templatePath

        patterns = {
                pattern: re.compile(patternStrings[pattern], flags=re.DOTALL)
                for pattern in patternStrings
            }
        templates = {
                path: readPathIntoString(Path(templateFolder, templatePaths[path]))
                for path in templatePaths
            }

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

    sourcePath = Path(".", args.sourcePath)
    buildPath = Path(".", args.buildPath)

    print(f"Building from {sourcePath} into {buildPath}")

    dirs, includedFiles, excludedFiles, unusedExtensions = searchDirForExtensions(
        sourcePath, fileConfig, folderConfig)
    
    noncompiledHTML: list[tuple[str, Path]] = []
    toCopy: list[str] = []
    for copy in includedFiles:
        fromPath = Path(sourcePath, copy)
        to = Path(buildPath, copy)
        if getFileExtensionOfPath(copy) == "html":
            contents = readPathIntoString(fromPath)
            noncompiledHTML.append((contents, copy))
        else:
            toCopy.append(copy)

    ### TODO: Diff the content?
    # Delete the old contents at Destination
    shutil.rmtree(buildPath, ignore_errors=True)
    os.mkdir(buildPath)

    # Rebuild the Tree
    for dir in set(dirs):
        try:
            os.makedirs(Path(buildPath, dir))
        except FileExistsError:
            continue

    # Add the leaves
    for copy in toCopy:
        fromPath = Path(sourcePath, copy)
        toPath = Path(buildPath, copy)
        print(f"Copying {fromPath} to {toPath}")
        shutil.copyfile(fromPath, toPath)

    # Build the projects
    preview, pages = buildProjects(patterns, templates, sourcePath)

    for (page, path) in noncompiledHTML:
        compiledPage = createHTML(defaultdict(str), page, patterns)
        print(f"Compiling: {path}")
        pages.append((compiledPage, path))
    
    unusedFilePatterns = {ext: re.compile(
        fr"\.({ext})", flags=re.IGNORECASE) for ext in unusedExtensions}
    foundPatterns = set()
    for page, path in pages:
        toPath = Path(buildPath, path)
        print(f"Writing to {path}")
        # Search for values that they may depend on that the build won't copy
        for (ext, pattern) in unusedFilePatterns.items():
            found = re.search(pattern, page)
            if found: foundPatterns.add(ext)

        writeStringIntoPath(page, toPath)
    print(f"WARNING: These patterns were found in the HTML of your project, but were not included in the build: {foundPatterns}")

    # Write the preview file
    writeStringIntoPath(preview[0], Path(buildPath, preview[1]))


if __name__ == "__main__":
    main()