# Life is short; you need Python.
import z3
import numpy as np
import sympy as sp
import matplotlib.pyplot as plt

x = sp.symbols('x')

def plot(f, points=[], draw_label=True, draw_points=True):
    """Plot a sympy symbolic polynomial f."""

    xmin = min([x_ for x_, _ in points], default=-1) - 0.1
    xmax = max([x_ for x_, _ in points], default=1) + 0.1

    xs = np.arange(xmin, xmax, (xmax - xmin) / 100)
    ys = [f.subs(x, x_) for x_ in xs]

    plt.grid(True)
    plt.plot(xs, ys)
    if draw_points:
        plt.scatter(
            [x_ for x_, y_ in points],
            [y_ for x_, y_ in points],
        )
    if draw_label:
        for x_, y_ in points:
            plt.text(x_, y_, f'$({x_},{y_})$', va='bottom', ha='center')
        plt.title(f'$y = {sp.latex(f)}$')

def interpolate(n=0, xs=[], ys=[]):
    """Return a polynomial that passes through all given points."""
    n = max(n, len(xs), len(ys))
    if len(xs) == 0: xs = [sp.symbols(f'x{i}') for i in range(n)]
    if len(ys) == 0: ys = [sp.symbols(f'y{i}') for i in range(n)]
    vs = [sp.symbols(f'a{i}') for i in range(n)]
    power = list(range(n))

    cons = [
        sum(
            v * (x_ ** k) for v, k in zip(vs, power)
        ) - y
            for x_, y in zip(xs, ys)
    ]

    sol = list(sp.linsolve(cons, vs))[0]

    f = (sum(
        v * (x ** k) for v, k in zip(sol, power)
    ))
    
    return f

# demo1
# plot(x + 1, draw_label=False)
# plot(x**2 + 1, draw_label=False)
# plot(x**3 + 1, draw_label=False)

# demo2
# xs = [-1, 0, 1, 2, 3]
# ys = [-1, 2, 1, 4, 5]
# f = interpolate(xs=xs, ys=ys)
# plot(f, list(zip(xs, ys)), True)

# demo3
n = 10
xs = np.arange(-1, 1, 1 / n)
ys = np.sin(xs * n)
f = interpolate(xs=xs, ys=ys)
plot(f, list(zip(xs, ys)), draw_points=True, draw_label=False)
f

plt.savefig("math_explore.jpg")