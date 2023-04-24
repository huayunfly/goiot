"""
Python algorithm for NN design.
@author huayunfly at 126.com
@start 2023.04.24
"""

from collections import deque
import math
import numpy as np

"""
Graph vertex definition
"""


class Vertex:
    def __init__(self, id) -> None:
        self.id = id
        self.color = None
        self.discovered = 0
        self.finished = 0
        self.distance = 0
        self.pre = None
        self.op_name = None
        pass


class Graph:
    def __init__(self) -> None:
        self.G = dict()
        self.time = 0
        self.debug = False
        pass

    # Set edges
    def set_edges(self, u, v_list):
        self.G[u] = v_list

    # Append edges
    def add_edges(self, u, v_list):
        adjs = self.G.get(u, None)
        if (adjs is None):
            self.set_edges(u, v_list)
        else:
            adjs.extend(v_list)
            
    # Depth first search
    def dfs(self):
        for u in self.G.keys():
            u.color = 'WHITE'
            u.pre = None
        self.time = 0
        for u in self.G.keys():
            if u.color == 'WHITE':
                self.dfs_visit(u)

    # Depth first search routine
    def dfs_visit(self, u):
        self.time += 1
        u.discovery = self.time
        u.color = 'GRAY'
        for v in self.G[u]:
            if v.color == 'WHITE':
                v.pre = u
                if (self.debug):
                    print(v.id)
                self.dfs_visit(v)
        self.time += 1
        u.finished = self.time
        u.color = 'BLACK'

    # Breadth first search
    def bfs(self, s):
        excluded = self.G.keys().remove(s)
        for u in excluded:
            u.color = 'WHITE'
            u.distance = math.inf
            u.pre = None
        s.color = 'GRAY'
        s.distance = 0
        s.pre = None
        queue = deque()
        queue.append(s)
        while queue:
            u = queue.popleft()
            for v in self.G[u]:
                if v.color == 'WHITE':
                    v.color = 'GRAY'
                    v.distance = u.distance + 1
                    v.pre = u
                    queue.append(v)
            u.color = 'BLACK'


"""
Operator representing arithmetic and logic calculation. 
"""


class Operator:
    def __init__(self) -> None:
        self.op_map = dict()
        self.init_op_map()
        pass

    def init_op_map(self):
        self.op_map['sum'] = lambda x_list: sum(x_list)
        self.op_map['npsum'] = lambda x_list: np.sum(x_list)
        pass

    def calculate(self, name, argv: list):
        try:
            return self.op_map[name](argv)
        except KeyError as e:
            print('* Handling KeyError')
            raise RuntimeError(name) from e


"""TEST"""
op = Operator()
assert (op.calculate('sum', [1, 2]) == 3)
assert (op.calculate('npsum', [1, 2]) == 3)

g = Graph()
g.debug = True
shanghai = Vertex('shanghai')
suzhou = Vertex('suzhou')
hangzhou = Vertex('hangzhou')
wuxi = Vertex('wuxi')
taicang = Vertex('taicang')
shaoxing = Vertex('shaoxing')
jinhua = Vertex('jinhua')
huangshan = Vertex('huangshan')
g.set_edges(shanghai, [suzhou, hangzhou])
g.set_edges(suzhou, [wuxi, taicang])
g.set_edges(hangzhou, [shaoxing, jinhua, huangshan])
g.dfs()

