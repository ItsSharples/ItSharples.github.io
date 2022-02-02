from inspect import Attribute
import itertools
import re, os
from collections import defaultdict
from xml.etree.ElementTree import TreeBuilder
from markdown import Markdown
from dataclasses import dataclass, field

@dataclass
class Project:
    url: str
    preview: str
    page: str
    isHTML: bool

    def __repr__(self) -> str:
        return self.url



def get_n_tabs(n):
    return ' '*n
    # return '\t'*n
@dataclass
class TreeNode:
    parent: 'TreeNode' = field(default= None, repr=False)
    name: str = field(default= "Base", repr=False)
    childNodes: dict[str, 'TreeNode'] = field(default_factory= defaultdict, repr=False)
    content: list[Project] = field(default_factory= list)
    
    @property
    def fullName(self) -> list[str]:
        if self.parent == None:
            return [self.name]
        return self.parent.fullName + [self.name]
        
    def __getitem__(self, key):
        if key not in self.childNodes.keys():
            self.childNodes[key] = TreeNode(self, key)
        return self.childNodes[key]

    def __setitem__(self, key, item):
        self[key].content.append(item);

    def append(self, item):
        self.content.append(item)

    def __iter__(self):
        currNode = self
        if len(self.childNodes) != 0:
            for item in currNode.childNodes.values():
                for value in iter(item):
                    yield value
        else:
            for item in self.content:
                yield (self.fullName, item)

    def __child_repr__(self, depth=0):
        return ''.join([f"\n{get_n_tabs(depth)}{key}: {self.childNodes[key].__repr__(depth+1)}" for key in self.childNodes]);
    def __repr__(self, depth=0):
        if self.childNodes.__len__() != 0:
            return f"Has Children: {self.__child_repr__(depth+1)}"
        else:
            return f"Has Contents: {self.content}"

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