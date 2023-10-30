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
    def __init__(self, id: object) -> None:
        self.id = id
        self.color = None
        self.discovered = 0
        self.finished = 0
        self.distance = 0
        self.pre = None
        self.op_name = None
        pass


class Graph:
    C_WHITE = 'white'
    C_GRAY = 'gray'
    C_BLACK = 'black'

    def __init__(self, debug = False) -> None:
        self.G = dict()
        self.debug = debug
        pass

    # Set edges
    def set_edges(self, u: Vertex, v_list):
        self.G[u] = v_list

    # Append edges
    def add_edges(self, u: Vertex, v_list):
        adjs = self.G.get(u)
        if (adjs is None):
            self.set_edges(u, v_list)
        else:
            adjs.extend(v_list)
            
    # Depth first search
    def dfs(self):
        for u in self.G:
            u.color = Graph.C_WHITE
            u.pre = None
        time = [0]
        for u in self.G:
            if u.color == Graph.C_WHITE:
                self.dfs_visit(u, time)

    # Depth first search routine
    def dfs_visit(self, u: Vertex, time):
        time[0] += 1
        u.discovery = time[0]
        u.color = Graph.C_GRAY
        # debug
        if (self.debug):
            print(u.id)
        # adjacent
        v_list = self.G.get(u)
        if (v_list is not None):         
            for v in v_list:
                if v.color == Graph.C_WHITE:
                    v.pre = u
                    self.dfs_visit(v, time)
        time[0] += 1
        u.finished = time[0]
        u.color = Graph.C_BLACK

    # Breadth first search
    def bfs(self, s: Vertex):
        if s is None:
            raise ValueError(s)
        excluded = set(self.G.keys()) - {s}
        for u in excluded:
            u.color = Graph.C_WHITE
            u.distance = math.inf
            u.pre = None
        s.color = Graph.C_GRAY
        s.distance = 0
        s.pre = None
        queue = deque()
        queue.append(s)
        while queue:
            u = queue.popleft()
            v_list = self.G.get(u)
            if (v_list is not None):
                for v in v_list:
                    if v.color == Graph.C_WHITE:
                        v.color = Graph.C_GRAY
                        v.distance = u.distance + 1
                        v.pre = u
                        queue.append(v)
            u.color = Graph.C_BLACK
            # debug
            if self.debug:
                print(u.id)


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

g = Graph(True)
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
g.set_edges(wuxi, None)
g.set_edges(taicang, None)
g.set_edges(shaoxing, None)
g.set_edges(jinhua, None)
g.set_edges(huangshan, None)
print('* Test dfs()')
g.dfs()
print('* Test bfs()')
g.bfs(shanghai)

