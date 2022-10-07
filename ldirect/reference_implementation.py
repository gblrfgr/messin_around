import numpy as np
import scipy as sp

from dataclasses import dataclass
from collections.abc import Callable


@dataclass
class OptimizationTarget:
    function: Callable[[np.array], float]
    domain: np.array

    def evaluate(self, point: np.array):
        if point.size != self.domain.shape[0]:
            raise ValueError(
                f"point has incorrect number of dimensions (expected {self.domain.shape[0]}, got {point.size})")
        for dimension in range(point.size):
            if not (self.domain[dimension][0] <= point[dimension] and self.domain[dimension][1] >= point[dimension]):
                print(self.domain[dimension])
                raise ValueError(f"point {point} not within expected domain")
        return self.function(point)


@dataclass
class DirectSubdivision:
    bounds: np.array

    def subdivide(self):
        # subdivides self into three parts, returns 'left' and 'right' subdivisions as a list
        axis = np.argmax(np.abs(self.bounds.T[0] - self.bounds.T[1]))
        gap = (self.bounds[axis][1] - self.bounds[axis][0]) / 3
        left = DirectSubdivision(np.copy(self.bounds))
        right = DirectSubdivision(np.copy(self.bounds))
        self.bounds[axis][0] += gap
        self.bounds[axis][1] -= gap
        left.bounds[axis][1] = self.bounds[axis][0]
        right.bounds[axis][0] = self.bounds[axis][1]
        return [left, right]

    def radius(self):
        # euclidean distance from centroid to any corner
        return np.sqrt(np.sum(((self.bounds.T[0] - self.bounds.T[1]) / 2)**2))

    def center(self):
        # centroid
        return ((self.bounds.T[0] + self.bounds.T[1]) / 2).T

    def __eq__(self, other):
        return np.array_equal(self.bounds, other.bounds)


def get_potentially_optimal(func: OptimizationTarget, subdivisions: list[DirectSubdivision]):
    # use andrew's monotone chain to get the lower convex hull
    def ccw(a, b, c):
        return (b.radius() - a.radius()) * (func.evaluate(c.center()) - func.evaluate(a.center())) - \
               (func.evaluate(b.center()) - func.evaluate(a.center()) * (c.radius() - a.radius()))
    stack = []
    subdivisions.sort(key=lambda s: func.evaluate(s.center()))
    subdivisions.sort(key=lambda s: s.radius())
    for i in range(len(subdivisions)):
        while len(stack) > 1 and ccw(subdivisions[stack[-2]], subdivisions[stack[-1]], subdivisions[i]) <= 0:
            stack.pop()
        stack.append(i)

    # chop off the downhill bit at the beginning
    while len(stack) > 1 and func.evaluate(subdivisions[stack[0]].center()) > func.evaluate(subdivisions[stack[1]].center()):
        stack.pop(0)

    return stack


rastrigin = OptimizationTarget(lambda x: 40 + np.sum(x**2 - 10*np.cos(2 * np.pi * x)),
                               np.array([[-5.12, 5.12] for i in range(4)]))

subdivisions = [DirectSubdivision(np.copy(rastrigin.domain))]
for i in range(100):
    optimals = get_potentially_optimal(rastrigin, subdivisions)
    for ind in optimals:
        subdivisions += subdivisions[ind].subdivide()