from math import floor
from random import randint, random, choice

layersAndRules = {
    ".stars1":"animation: space 2400s linear infinite;",
    ".stars2":"animation: space 3000s linear infinite;",
    ".stars3":"animation: space 3600s linear infinite;",
    ".stars4":"animation: space 4500s linear infinite;"
}

colours = ['163, 193, 255',
           '179, 204, 255',
           '204, 219, 255',
           '255, 243, 239',
           '255, 209, 163',
           '255, 180, 107']

width = 400
height = 220
numchunks = 2
numstars = 4

dim = max(width, height)

xchunk = floor((dim+width) / (numchunks*2))
ychunk = floor((dim+height) / (numchunks*2))


outComponents = []
# Take each Layer and Convert into appropriate css
for i, (layerName, layerRules) in enumerate(layersAndRules.items()):
    layerOut = layerName + " { " + layerRules + " "
    layerInt = i-1

    # Add Basic Rules
    layerOut += f"background-size: {dim}px {dim}px; "
    layerOut += f"transform: rotate({layerInt * 4}turn); "

    # Write the stars out as the background 'image'
    starRule = "background-image: "
    for star in range(numstars):
        x = randint(0, xchunk) * numchunks
        y = randint(0, ychunk) * numchunks

        val = random()
        if val > 0.7:
            if val > 0.9: w = 2.5
            else: w = 1.5
        else:
            w = 1

        colour = choice(colours)
        starRule += f"radial-gradient({w}px {w}px at {x}px {y}px, rgba({colour}, 0.8), rgba({colour}, 0)),"

    starRule = starRule[:-1] + "; "

    # Finish the Rule
    layerOut += starRule;
    layerOut += "}"

    # Add to the final components
    outComponents.append(layerOut)

# Add Reduced Motion Config
reducedMotion = f"@media (prefers-reduced-motion) {{ {', '.join(layersAndRules.keys())} {{ animation-play-state: paused !important; }} }}\n"

outComponents.append(reducedMotion)

with open("./src/css/stars-config.css", "w") as config:
    config.write("\n".join(outComponents))
