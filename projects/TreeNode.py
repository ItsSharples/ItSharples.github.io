from dataclasses import dataclass, field
from collections import defaultdict
from .Project import Project

def get_n_tabs(n):
    return ' '*n
    
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
    
    @property
    def depth(self) -> int:
        if self.parent == None:
            return 0
        return self.parent.depth + 1;
        
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
