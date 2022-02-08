from dataclasses import dataclass, field
from collections import defaultdict
from typing import Callable, Iterator, TypeVar, Generic
import copy

def get_n_tabs(n):
    return ' '*n
    

TreeNodeContent = TypeVar('TreeNodeContent')
@dataclass
class TreeNode(Generic[TreeNodeContent]):
    parent: 'TreeNode | None' = field(default= None, repr=False)
    name: str = field(default= "Base", repr=False)
    childNodes: dict[str, 'TreeNode'] = field(default_factory= defaultdict, repr=False)
    content: list[TreeNodeContent] = field(default_factory= list)
    
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

    def rename(self, newName: str) -> 'TreeNode[TreeNodeContent]':
        self.name = newName
        return self

    def getBranch(self, key: str) -> 'TreeNode[TreeNodeContent]':
        branch: TreeNode[TreeNodeContent] = copy.deepcopy(self[key])
        branch.parent = None
        return branch

    def __contains__(self, key: str):
        return key in self.childNodes.keys()

    def __getitem__(self, key: str):
        if key not in self.childNodes.keys():
            self.childNodes[key] = TreeNode(self, key)
        return self.childNodes[key]

    def __setitem__(self, key: str, item: TreeNodeContent):
        self[key].content.append(item);

    def append(self, item: TreeNodeContent):
        self.content.append(item)

    def __iter__(self) -> Iterator[TreeNodeContent]:
        return self.mapContent(lambda content: content)
        # if self.childNodes:
        #     for node in self.childNodes.values():
        #         for value in iter(node):
        #             yield value
        # for item in self.content:
        #     yield (self.fullName, item)

    Return = TypeVar('Return')
    def mapContent(self, function: Callable[[TreeNodeContent], Return]) -> Iterator[Return]:
        if self.childNodes:
            for node in self.childNodes.values():
                for value in node.mapContent(function):
                    yield value
        for item in self.content:
            yield function(item)

    def mapNodes(self, function: 'Callable[[TreeNode], Return]') -> Iterator[Return]:
        if self.childNodes:
            for node in self.childNodes.values():
                for value in node.mapNodes(function):
                    yield value
        yield function(self)

    def __child_repr__(self, depth=0):
        return ''.join([f"\n{get_n_tabs(depth)}{key}: {self.childNodes[key].__repr__(depth+1)}" for key in self.childNodes]);
    def __repr__(self, depth=0):
        if self.childNodes.__len__() != 0:
            return f"Has Children: {self.__child_repr__(depth+1)}"
        else:
            return f"Has Contents: {self.content}"
