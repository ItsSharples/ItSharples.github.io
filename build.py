import argparse, os, shutil

import projects.build

# Default Arguments
buildPath = "_site"
defaultTypes = ["png", "html", "js", "css"]
# Process Arguments
parser = argparse.ArgumentParser(description='Build this Website, and Move it to an output path.')
parser.add_argument('-o', '--output', metavar='dest', type=str, dest='buildPath', default=buildPath,
                    nargs='?', help=f"The output destination inside '.'. Default: {buildPath}")
parser.add_argument('-t', '--types', metavar='type', type=str, dest='types', default=defaultTypes,
                    nargs='*', help=f"Types to move to the output path. Default: {defaultTypes}")
parser.add_help = True
args = parser.parse_args()

dots = ['.venv', '.jekyll-cache', '.git', '.github', '__pycache__']
userDefined: list[str] = [args.buildPath, 'scratch', 'data', 'templates']
exclude = set(dots)
exclude = exclude.union(userDefined)

def searchDirForExtensions(path, forExtensions: set[str] = defaultTypes, exclude: set[str] = exclude):
    out: list[tuple[list, str]] = []
    if os.path.isfile(path):
        out.append(path.name)

    usedDirs = []
    for root, dirs, files in os.walk(path):
        dirs[:] = set(dirs) - exclude
        for file in files:
            if os.path.splitext(file)[1][1:] in forExtensions:
                out.append(os.path.join(root, file));

        usedDirs.append(root)
    return usedDirs, out

dirs, toMove = searchDirForExtensions(".", set(args.types), exclude)

shutil.rmtree(args.buildPath, ignore_errors=True)
os.mkdir(args.buildPath)

# Rebuild the Tree
for dir in set(dirs):
    try:
        os.makedirs(os.path.join('.', args.buildPath, dir[2:]))
    except FileExistsError:
        continue

# Add the leaves
for move in toMove:
    print(f"Moving {move}")
    shutil.copyfile(move, os.path.join('.', args.buildPath, move[2:]))

# Build the projects
projects.build.buildProjects(args.buildPath)