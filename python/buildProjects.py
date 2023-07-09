import re
from collections import defaultdict
from markdown import Markdown
import yaml
from pathlib import Path

from TreeNode import TreeNode
from Project import Project
from SafeEval import safe_eval
from helpers import readPathIntoString, searchDir, writeStringIntoPath
from compileHTML import createHTML

def createProjectPreview(template: str, yaml: defaultdict[str, str | Path], patterns: dict[str, re.Pattern]):
    if "titlecard-url" not in yaml.keys():
        yaml["titlecard-url"] = yaml['url'].joinpath("titlecard.png")
    return createHTML(yaml, template, patterns)

def writeTemplate(keypairs: defaultdict[str, str], url: Path,  template: str, patterns: dict[str, re.Pattern]):
    writeStringIntoPath(createHTML(keypairs, template, patterns), url)

def yamlToDict(yamlStr: str) -> defaultdict:
    return defaultdict(str, yaml.safe_load(yamlStr.strip()))

def createProjects(source: Path, patterns: dict[str, re.Pattern], templates: dict[str, str]):

    previewTemplate = templates['preview']
    projectTemplate = templates['project']

    yamlPattern = patterns["yaml"]

    projectTree: TreeNode[Project] = TreeNode(None)
    for project in searchDir(source / "projects", "md"):
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
        source: Path = Path("."), destination: Path = Path("_site"), templateFolder: Path = Path("templates")):

    patterns = {pattern: re.compile(
        patternStrings[pattern], flags=re.DOTALL) for pattern in patternStrings}
    templates = {path: readPathIntoString(Path(
        templateFolder, templatePaths[path])) for path in templatePaths}

    projects = createProjects(source, patterns, templates)

    # List is so that it actions the map
    projectList = list(filter(None, projects.mapContent(lambda project:
                                          (project.page, Path(
                                              destination, project.url, "index.html"))
                                          )))

    # Write the Preview Page
    previewPage = createHTML(
        defaultdict(
            str, [('content', convertIntoHTML(projects, patterns, templates))]),
        templates['overview'], patterns)
    previewTuple = (previewPage, Path(
        destination, "projects/index.html"))
    
    return previewTuple, projectList
