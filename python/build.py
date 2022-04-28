import argparse
import os
import shutil
import yaml
from dataclasses import dataclass, field
from buildProjects import buildProjects


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

    def searchDirForExtensions(path: str,
                               fileConfig: IncludeExclude = fileConfig, folderConfig: IncludeExclude = folderConfig):
        out: list[str] = []
        if os.path.isfile(path):
            out.append(path.name)

        usedDirs = []
        for root, dirs, files in os.walk(path):
            reinclude = set(dirs) & folderConfig.include
            # Exclude from dirs, all subdirs starting with . or in the exclude set,
            #   and then reinclude any that were excluded
            dirs[:] = (set(dirs)
                       - set([dir for dir in dirs if dir.startswith('.')])
                       - folderConfig.exclude) \
                | reinclude

            for file in files:
                if file in fileConfig.exclude and not file in fileConfig.include:
                    continue

                if os.path.splitext(file)[1][1:] in fileConfig.include:
                    out.append(os.path.join(root, file))

            usedDirs.append(root)
        return usedDirs, out

    print(f"Building from {args.sourcePath} into {args.buildPath}")

    dirs, toCopy = searchDirForExtensions(
        args.sourcePath, fileConfig, folderConfig)

    # Delete the old contents at Destination
    shutil.rmtree(args.buildPath, ignore_errors=True)
    os.mkdir(args.buildPath)

    # Rebuild the Tree
    for dir in set(dirs):
        try:
            os.makedirs(os.path.join('.', args.buildPath, dir[2:]))
        except FileExistsError:
            continue

    # Add the leaves
    for copy in toCopy:
        to = os.path.join('.', args.buildPath, copy[2:])
        print(f"Copying {copy} to {to}")
        shutil.copyfile(copy, to)

    # Build the projects
    buildProjects(patternStrings, templatePaths, args.sourcePath,
                  args.buildPath, args.templatePath)


if __name__ == "__main__":
    main()
