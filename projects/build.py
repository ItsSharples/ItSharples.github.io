import re, os
from collections import defaultdict
from markdown import Markdown

from .TreeNode import TreeNode
from .Project  import Project



def get_n_tabs(n):
    return ' '*n
    # return '\t'*n


def readIntoString(url: str): 
    with open(url, "r") as file: return file.read()
def writeStringInto(string: str, url: str):
    with open(url, "w") as out: out.write(string)

def createHTML(keypairs: dict, template: str, commandPattern: re.Pattern):
    # Is there a better way to reconstruct the template?
    match = re.search(commandPattern, template)
    while match:
        new_value = keypairs[match[1].strip().lower()]
        template = template[:match.start(0)] + new_value + template[match.end(0):]
        match = re.search(commandPattern, template)
    return template

def searchDir(path: str, forExtension: str = ".html"):
    out: list[tuple[list, str]] = []
    if os.path.isfile(path):
        out.append(path.name)
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

def createProjectPreview(template: str, yaml: defaultdict, commandPattern: re.Pattern):
    if "titlecard-url" not in yaml.keys():
        yaml["titlecard-url"] = f"{yaml['url']}/titlecard.png"
    return createHTML(yaml, template, commandPattern)

def writeTemplate(keypairs: defaultdict(str), url: str,  template: str, commandPattern: re.Pattern):
    writeStringInto(createHTML(keypairs, template, commandPattern), url)

def createProjects( yamlPattern: re.Pattern, commandPattern: re.Pattern,
                    projectTemplate: str, previewTemplate: str):
    projectTree = TreeNode(None)
    projects = searchDir("projects", ".md")
    for project in projects:
        with open(project, "r") as file:
            a = re.split(yamlPattern, file.read())
        yamlString, markdownString = "", ""
        # Just Markdown
        match len(a):
            case 1:
                markdown = a[0]
            case 2:
                _, markdown = a
            case 3:
                _, yamlString, markdownString = a

        yaml = defaultdict(str, [tuple(row.split(':')) for row in yamlString.strip().splitlines()])
        yaml["url"] = os.path.dirname(project)

        md = Markdown()
        md.build_parser()

        groups: list[str] = os.path.normpath(yaml["url"]).split(os.sep)
        currNode = projectTree
        for group in groups:
            currNode = currNode.__getitem__(group)
        
        preview = createProjectPreview(previewTemplate, yaml, commandPattern)
        page = createHTML(defaultdict(str, [('content', md.convert(markdownString))]), projectTemplate, commandPattern)

        project = Project(url= yaml['url'], preview= preview, page= page, isHTML= True)
        currNode.append(project)

    return projectTree



def buildProjects(into: str = ""):
    patternStrings = {
        'command': r"{{([^}}]+)}}",
        'yaml': r"---([^---]+)---"
    }
    templatePaths = {
        'preview': "templates/project_listitem_template.html",
        'project': "templates/project_template.html",
        'overview': "templates/projects_template.html",
        'group': "templates/project_group_template.html"
    }

    patterns = {pattern: re.compile(patternStrings[pattern]) for pattern in patternStrings}
    templates = {path: readIntoString(templatePaths[path]) for path in templatePaths}


    previewTemplate = templates['preview']
    projectTemplate = templates['project']
    overviewTemplate = templates['overview']
    groupTemplate = templates['group']

    commandPattern = patterns['command']
    yamlPattern = patterns["yaml"]

    projects: TreeNode = createProjects(yamlPattern, commandPattern, projectTemplate, previewTemplate)

    projectList: list[Project] = []
    groupDict: defaultdict[str, list[Project]] = defaultdict(list)
    for name, project in projects:
        # Ignore past 'projects', as this is where we expect them to be (Past 'Base' and 'projects')
        # And then ignore the last one, because that's the final folder
        group = os.path.join(*name[2:-1])
        groupDict[group].append(project)
        projectList.append(project)


    previewContent = '\n'.join([createHTML(defaultdict(str,
        [('content', '\n'.join([project.preview for project in groupDict[group]])),
         ('name', group)
        ]), groupTemplate, commandPattern)
        for group in groupDict])

    # Write the Preview Page
    previewPage = createHTML(
        defaultdict(str, [('content', previewContent)]),
        overviewTemplate, commandPattern)

    for project in projectList:
        writeStringInto(project.page, os.path.join(into, project.url, "index.html"))
    writeStringInto(previewPage, os.path.join(into, "projects/index.html"))