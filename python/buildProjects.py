import re
import os
from collections import defaultdict
from markdown import Markdown
import yaml
import pathlib
from pathlib import Path

from TreeNode import TreeNode
from Project import Project
from SafeEval import safe_eval


def readIntoString(url: Path):
    with open(url, "r") as file:
        return file.read()


def writeStringInto(string: str, url: Path):
    with open(url, "w") as out:
        out.write(string)


def normalise(string: str):
    return string.strip().lower()


def createHTML(keypairs: dict[str, str], template: str, patterns: dict[str, re.Pattern]):
    # Is there a better way to reconstruct the template?
    commandPattern = patterns['command']
    calculatePattern = patterns['calculate']
    computePattern = patterns['compute']
    operatorPattern = patterns['operations']

    # First rebuild the template with any computations there may be
    todoCompute = computePattern.search(normalise(template))
    while todoCompute:
        value: str
        query: str
        result: str
        query, value, result = todoCompute['query'], todoCompute['value'], todoCompute['result']
        span = todoCompute.span()
        editedTemplate = False
        match(query):
            case "if":
                if(keypairs[value]):
                    template = template[:span[0]] + result + template[span[1]:]
                    editedTemplate = True

            case "foreach":
                name, _, lst = value.split()
                if(keypairs[lst]):
                    out = [createHTML(defaultdict(str, [tuple(
                        [name.strip(), item.strip()])]), result, patterns) for item in keypairs[lst]]
                    template = template[:span[0]] + \
                        ",\n".join(out) + template[span[1]:]
                    editedTemplate = True
            case _:
                print(query)
                pass

        if not editedTemplate:
            # Remove the failed case
            template = template[:span[0]] + template[span[1]:]

        todoCompute = computePattern.search(normalise(template))

    # Then build the template with the values
    match = commandPattern.search(template)
    while match:
        todoCalculate = calculatePattern.search(normalise(match[1]))
        if todoCalculate:
            # Inside Calculations, the values change meaning from just strings to potential values
            value = todoCalculate[1]
            # Safety Check, only evaluate if exists in a very limited subset of python
            operators = operatorPattern.search(value)

            if operators:
                normalLeft = normalise(operators['left'])
                normalRight = normalise(operators['right'])
                # Convert left and right to values, if possible
                left = keypairs[normalLeft] if normalLeft in keypairs else normalLeft
                right = keypairs[normalRight] if normalRight in keypairs else normalRight

                left = int(left) if left.isnumeric() else left
                right = int(right) if right.isnumeric() else right

                operator = normalise(operators['operator'])

                computation = ''.join([str(left), str(operator), str(right)])

                value = value[:operators.start()] + \
                    str(safe_eval(computation)) + value[operators.end():]

        else:
            value = keypairs[normalise(match[1])]

        template = template[:match.start(
            0)] + str(value) + template[match.end(0):]
        match = commandPattern.search(template)
    return template.strip()


def searchDir(path: Path, forExtension: str = ".html"):
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

def createProjectPreview(template: str, yaml: defaultdict[str, str | Path], patterns: dict[str, re.Pattern]):
    if "titlecard-url" not in yaml.keys():
        yaml["titlecard-url"] = yaml['url'].joinpath("titlecard.png")
    return createHTML(yaml, template, patterns)


def writeTemplate(keypairs: defaultdict[str, str], url: Path,  template: str, patterns: dict[str, re.Pattern]):
    writeStringInto(createHTML(keypairs, template, patterns), url)


def yamlToDict(yamlStr: str) -> defaultdict:
    return defaultdict(str, yaml.safe_load(yamlStr.strip()))


def createProjects(source: pathlib.Path, patterns: dict[str, re.Pattern], templates: dict[str, str]):

    previewTemplate = templates['preview']
    projectTemplate = templates['project']

    yamlPattern = patterns["yaml"]

    projectTree: TreeNode[Project] = TreeNode(None)
    for project in searchDir(source / "projects", ".md"):
        with open(project, "r") as file:
            a = re.match(yamlPattern, file.read())

        md = Markdown()
        md.build_parser()

        yaml, markdown = yamlToDict(a['yaml']), md.convert(a['markdown'])
        yaml["url"] = project.parent
        yaml["ignore"] = bool(yaml['ignore'])

        if yaml["ignore"]:
            continue

        groups: list[Path] = yaml["url"].parts
        currNode = projectTree
        for group in groups:
            currNode = currNode[group]

        preview = createProjectPreview(previewTemplate, yaml, patterns)
        page = createHTML(defaultdict(
            str, [('content', markdown)]), projectTemplate, patterns)

        project = Project(
            url= yaml['url'], preview=preview, page=page, isHTML=True)
        currNode.append(project)
    # We can ignore 'Base' node, and pretend we start at 'projects'
    return projectTree.getBranch('projects').rename('Projects')

# Convert the projects Tree into groups of groups of data


def convertIntoHTML(projects: TreeNode[Project], patterns: dict[str, re.Pattern], templates: dict[str, str]):
    htmlOut = ""
    if projects.childNodes:
        # Get Preview of childNode
        groupHtml = ''.join([convertIntoHTML(childNode, patterns, templates)
                            for childNode in projects.childNodes.values()])

        if projects.parent == None:
            pass  # We cover the title elsewhere in the html
        else:
            groupHtml = createHTML(defaultdict(str,
                                               [
                                                   ('content', groupHtml),
                                                   ('name', projects.name),
                                                   ('depth', str(projects.depth))
                                               ]), templates['group'], patterns)

        htmlOut += groupHtml

    if projects.content:
        # Get Preview of contents
        projectsHtml = ''.join(
            [project.preview for project in projects.content])

        # If you want to add the title to the project list
        # projectsHtml= createHTML(defaultdict(str,
        #     [
        #     ('content', projectsHtml),
        #     ('name', projects.name),
        #     ('depth', str(projects.depth))
        #     ]), groupTemplate, patterns)

        htmlOut += projectsHtml

    return htmlOut


def buildProjects(
        patternStrings: dict[str, str], templatePaths: dict[str, str],
        source: pathlib.Path = Path("."), destination: Path = Path("_site"), templateFolder: Path = Path("templates")):

    patterns = {pattern: re.compile(
        patternStrings[pattern], flags=re.DOTALL) for pattern in patternStrings}
    templates = {path: readIntoString(Path(
        templateFolder, templatePaths[path])) for path in templatePaths}

    projects = createProjects(source, patterns, templates)

    # List is so that it actions the map
    list(filter(None, projects.mapContent(lambda project:
                                          writeStringInto(project.page, Path(
                                              destination, project.url, "index.html"))
                                          )))

    # Write the Preview Page
    previewPage = createHTML(
        defaultdict(
            str, [('content', convertIntoHTML(projects, patterns, templates))]),
        templates['overview'], patterns)
    writeStringInto(previewPage, Path(
        destination, "projects/index.html"))
