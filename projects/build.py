import re, os
from collections import defaultdict
from markdown import Markdown


def replaceCommands(template: str, dict: dict, commandPattern: re.Pattern):
    # Is there a better way to reconstruct the template?
    match = re.search(commandPattern, template)
    while match:
        new_value = dict[match[1].strip().lower()]
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
    return replaceCommands(template, yaml, commandPattern)

def writeProjectsOverview(projects: str, overviewTemplate: str, commandPattern: re.Pattern):
    keypairs = defaultdict(str)
    keypairs['projects'] = projects

    newFile = replaceCommands(overviewTemplate, keypairs, commandPattern)

    with open("projects/index.html", "w") as out:
        out.write(newFile)

def writeProject(project: str, url: str, projectTemplate: str, commandPattern: re.Pattern):
    keypairs = defaultdict(str)
    keypairs['content'] = project

    newFile = replaceCommands(projectTemplate, keypairs, commandPattern)
    with open(url, "w") as out:
        out.write(newFile)

def writeProjects(
    yamlPattern: re.Pattern, commandPattern: re.Pattern,
    projectTemplate: str, previewTemplate: str):
    projectList: list[str] = []
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

        projectList.append(createProjectPreview(previewTemplate, yaml, commandPattern))

        md = Markdown()
        md.build_parser()
        markdown = md.convert(markdownString);
        
        writeProject(markdown, f"{yaml['url']}/index.html", projectTemplate, commandPattern)
    return projectList


def buildProjects():
    commandPattern = re.compile(r"{{([^}}]+)}}")
    yamlPattern = re.compile(r"---([^---]+)---")

    b = defaultdict(lambda: "Unknown", {'name': 'world', 'food': 'cheese'})

    with open("templates/project_listitem_template.html", "r") as file:
        previewTemplate = file.read()
    with open("templates/project_template.html", "r") as file:
        projectTemplate = file.read()
    with open("templates/projects_template.html", "r") as file:
        overviewTemplate = file.read()

    projectList = writeProjects(yamlPattern, commandPattern, projectTemplate, previewTemplate)
    writeProjectsOverview('\n'.join(projectList), overviewTemplate, commandPattern)


def testPath():
    print(buildPath)