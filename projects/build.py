import re, os
from collections import defaultdict
from markdown import Markdown

from .TreeNode import TreeNode
from .Project  import Project
from .SafeEval import safe_eval

def readIntoString(url: str): 
    with open(url, "r") as file: return file.read()
def writeStringInto(string: str, url: str):
    with open(url, "w") as out: out.write(string)

def normalise(string: str):
    return string.strip().lower()
def createHTML(keypairs: dict[str, str], template: str, patterns: dict[str, re.Pattern]):
    # Is there a better way to reconstruct the template?
    commandPattern = patterns['command']
    calculatePattern = patterns['calculate']
    operatorPattern = patterns['operations']

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

                value = value[:operators.start()] + str(safe_eval(computation)) + value[operators.end():]

        else:
            value = keypairs[normalise(match[1])]

        template = template[:match.start(0)] + str(value) + template[match.end(0):]
        match = commandPattern.search(template)
    return template

def searchDir(path: str, forExtension: str = ".html"):
    out: list[str] = []
    if os.path.isfile(path):
        out.append(path)
    for project in os.scandir(path):
        if project.name == "data":
            continue
        if forExtension in project.name:
            out.append(project.path);
            continue
        if project.is_dir():
            out += (searchDir(os.path.join(path, project.name), forExtension));
            continue
    return out

def createProjectPreview(template: str, yaml: defaultdict[str, str], patterns: dict[str, re.Pattern]):
    if "titlecard-url" not in yaml.keys():
        yaml["titlecard-url"] = f"{yaml['url']}/titlecard.png"
    return createHTML(yaml, template, patterns)

def writeTemplate(keypairs: defaultdict[str, str], url: str,  template: str, patterns: dict[str, re.Pattern]):
    writeStringInto(createHTML(keypairs, template, patterns), url)

def createProjects( patterns: dict[str, re.Pattern],
                    templates: dict[str, str]):

    previewTemplate = templates['preview']
    projectTemplate = templates['project']

    yamlPattern = patterns["yaml"]

    projectTree: TreeNode[Project] = TreeNode(None)
    projects = searchDir("projects", ".md")
    for project in projects:
        with open(project, "r") as file:
            a = re.split(yamlPattern, file.read())
        yamlString, markdownString = "", ""
        # Just Markdown
        match len(a):
            case 1:
                markdownString = a[0]
            case 2:
                _, markdownString = a
            case 3:
                _, yamlString, markdownString = a

        yaml = defaultdict(str, [tuple(row.split(':')) for row in yamlString.strip().splitlines()])
        yaml["url"] = os.path.dirname(project)

        md = Markdown()
        md.build_parser()

        groups: list[str] = os.path.normpath(yaml["url"]).split(os.sep)
        currNode = projectTree
        for group in groups:
            currNode = currNode[group]
        
        preview = createProjectPreview(previewTemplate, yaml, patterns)
        page = createHTML(defaultdict(str, [('content', md.convert(markdownString))]), projectTemplate, patterns)

        project = Project(url= yaml['url'], preview= preview, page= page, isHTML= True)
        currNode.append(project)

    # We can ignore 'Base' node, and pretend we start at 'projects'
    return projectTree.getBranch('projects').rename('Projects')



def buildProjects(into: str = ""):
    patternStrings = {
        'command': r"{{([^}}]+)}}",
        'calculate': r"calc\((.+)\)",
        'yaml': r"---([^---]+)---",
        'operations': r"(?P<left>.+)(?P<operator>[/*-+])(?P<right>.+)"
    }
    templatePaths = {
        'preview': "templates/project_preview_template.html",
        'project': "templates/project_template.html",
        'overview': "templates/projects_template.html",
        'group': "templates/project_group_template.html"
    }

    patterns = {pattern: re.compile(patternStrings[pattern]) for pattern in patternStrings}
    templates = {path: readIntoString(templatePaths[path]) for path in templatePaths}

    projects: TreeNode[Project] = createProjects(patterns, templates)

    projectList: list[Project] = []
    groupDict: defaultdict[str, list[tuple[TreeNode, str, list[Project]]]] = defaultdict(list)
    groupPreviews: list[str] = []

    overviewTemplate = templates['overview']
    groupTemplate = templates['group']

    # Convert the projects Tree into groups of groups of data
    def convertIntoHTML(projects: TreeNode[Project]):
        htmlOut = ""
        if projects.childNodes:
            # Get Preview of childNode
            groupHtml = ''.join([convertIntoHTML(childNode) for childNode in projects.childNodes.values()])
            
            if projects.parent == None: 
                pass # We cover the title elsewhere in the html
            else:
                groupHtml = createHTML(defaultdict(str,
                    [
                    ('content', groupHtml),
                    ('name', projects.name),
                    ('depth', str(projects.depth))
                    ]), groupTemplate, patterns)

            htmlOut += groupHtml

        if projects.content:
            # Get Preview of contents
            projectsHtml = ''.join([project.preview for project in projects.content])

            # If you want to add the title to the project list
            # projectsHtml= createHTML(defaultdict(str,
            #     [
            #     ('content', projectsHtml),
            #     ('name', projects.name),
            #     ('depth', str(projects.depth))
            #     ]), groupTemplate, patterns)

            htmlOut += projectsHtml

        return htmlOut

    # List is so that it actions the map
    list(filter(None, projects.mapContent(lambda project: 
        writeStringInto(project.page, os.path.join(into, project.url, "index.html"))
    )))

    # Write the Preview Page
    previewPage = createHTML(
        defaultdict(str, [('content', convertIntoHTML(projects))]),
        overviewTemplate, patterns)
    writeStringInto(previewPage, os.path.join(into, "projects/index.html"))

    
    