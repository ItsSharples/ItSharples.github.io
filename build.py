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


# Build the projects
projects.build.buildProjects()
# Move all files to the build path
print(args.buildPath)
print(args.types)

dots = ['.venv', '.jekyll-cache', '.git', '.github']
userDefined = [args.buildPath, 'scratch', 'data']
exclude = set(dots + userDefined)
def searchDirForExtensions(path, forExtensions: list[str] = defaultTypes):
    out: list[tuple[list, str]] = []
    if os.path.isfile(path):
        out.append(path.name)

    usedDirs = []
    for root, dirs, files in os.walk(path):
        for file in files:
            if os.path.splitext(file)[1][1:] in forExtensions:
                dirs[:] = set(dirs) - exclude
                out.append(os.path.join(root, file));

        usedDirs.append(root)
    return usedDirs, out

dirs, toMove = searchDirForExtensions(".")

# for move in toMove:
#     print(move)
print(dirs)


shutil.rmtree(args.buildPath, ignore_errors=True)
os.mkdir(args.buildPath)
# Rebuild the Tree
for dir in set(dirs):
    try:
        os.makedirs(os.path.join('.', args.buildPath, dir[2:]))
    except FileExistsError:
        print(f"Ignoring {dir}. Reason: Already Exists")
# Add the leaves
for move in toMove:
    # print(move)
    shutil.copyfile(move, os.path.join('.', args.buildPath, move[2:]))