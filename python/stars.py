from math import floor
from random import randint, random, choice

layers = [
    ".stars1 { animation: space 2400s linear infinite; background-image:",
    ".stars2 { animation: space 3000s linear infinite; background-image:",
    ".stars3 { animation: space 3600s linear infinite; background-image:",
    ".stars4 { animation: space 4500s linear infinite; background-image:"
]

colours = ['163, 193, 255',
           '179, 204, 255',
           '204, 219, 255',
           '255, 243, 239',
           '255, 209, 163',
           '255, 180, 107']

width = 400
height = 220
numchunks = 5
numstars = 10

dim = max(width, height)

xchunk = floor((dim+width) / (numchunks*2))
ychunk = floor((dim+height) / (numchunks*2))


out = ""
for i, layer in enumerate(layers):
    layerInt = i-1
    out += layer
    for star in range(numstars):
        x = randint(0, xchunk) * numchunks
        y = randint(0, ychunk) * numchunks

        val = random()
        if val > 0.7:
            if val > 0.9:
                w = 2.5
            else:
                w = 1.5
        else:
            w = 1

        colour = choice(colours)
        out += f"radial-gradient({w}px {w}px at {x}px {y}px, rgba({colour}, 0.8), rgba({colour}, 0)),"

    out = out[:-1] + ";"

    out += f"background-size: {dim}px {dim}px;"
    out += f"transform: rotate({layerInt * 4}turn);"

    out += "}\n"

with open("./css/stars-config.css", "w") as config:
    config.write(out)
