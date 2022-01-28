import re, os
from collections import defaultdict
from markdown import Markdown

commandPattern = re.compile(r"{{([^}}]+)}}")
yamlPattern = re.compile(r"---([^---]+)---")

b = defaultdict(lambda: "Unknown", {'name': 'world', 'food': 'cheese'})

with open("templates/project_listitem_template.html", "r") as file:
    previewTemplate = file.read()
with open("templates/project_template.html", "r") as file:
    projectTemplate = file.read()
with open("templates/projects_template.html", "r") as file:
    overviewTemplate = file.read()


def replaceCommands(template, dict):

    # Is there a better way to reconstruct the template?
    match = re.search(commandPattern, template)
    while match:
        new_value = dict[match[1].strip().lower()]
        template = template[:match.start(0)] + new_value + template[match.end(0):]
        match = re.search(commandPattern, template)
    return template

def searchDir(path, forExtension= ".html"):
    out = []
    if os.path.isfile(path):
        out.append(path.name)
    for project in os.scandir(path):
        if project.name == "data": continue
        if forExtension in project.name :
            out.append(project.path); continue
        if project.is_dir():
            out += ((searchDir(os.path.join(path, project.name), forExtension))); continue
    return out


def createProjectPreview(template, yaml: defaultdict):
    if "titlecard-url" not in yaml.keys():
        yaml["titlecard-url"] = f"{yaml['url']}/titlecard.png"
    return replaceCommands(template, yaml)

def writeProjectsOverview(projects):
    keypairs = defaultdict(str)
    keypairs['projects'] = projects

    newFile = replaceCommands(overviewTemplate, keypairs)

    with open("projects/index.html", "w") as out:
        out.write(newFile)

def writeProject(project, url):
    keypairs = defaultdict(str)
    keypairs['content'] = project

    newFile = replaceCommands(projectTemplate, keypairs)
    with open(url, "w") as out:
        out.write(newFile)


projectList = []
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

    projectList.append(createProjectPreview(previewTemplate, yaml))

    md = Markdown()
    md.build_parser()
    markdown = md.convert(markdownString);
    
    writeProject(markdown, f"{yaml['url']}/index.html")

writeProjectsOverview('\n'.join(projectList))