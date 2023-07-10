import argparse
import yaml


def setupParserAndParse():
    # Default Arguments
    defaultConfigPath = "_config.yml"
    defaultSourcePath = "."
    defaultBuildPath = "_site"
    defaultTemplatesPath = "templates"
    defaultComponentsPath = "components"
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
    parser.add_argument('-p', '--components', metavar='path', type=str, dest='componentPath', const=None,
                        nargs='?', help=f"The components path relative to '.'. Default: {defaultComponentsPath}")
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

    args.patterns = yamlConfig['patterns']
    args.templates = yamlConfig['templates']

    # Read from yaml if the command line does not indicate a preference
    if args.sourcePath == None:
        args.sourcePath = yamlConfig.get("sourceDir", defaultSourcePath)
    if args.buildPath == None:
        args.buildPath = yamlConfig.get("destinationDir", defaultBuildPath)
    if args.templatePath == None:
        args.templatePath = yamlConfig.get("templatePath", defaultTemplatesPath)
    if args.componentPath == None:
        args.componentPath = yamlConfig.get("componentPath", defaultComponentsPath)

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
