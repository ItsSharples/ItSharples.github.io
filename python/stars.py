from random import randint, random

layers = [
    ".stars1 { animation: space 240s linear infinite; background-image: ",
    ".stars2 { animation: space 300s linear infinite; background-image:",
    ".stars3 { animation: space 360s linear infinite; background-image:",
    ".stars4 { animation: space 450s linear infinite; background-image:"
]


out = ""
for layer in layers:
    out += layer
    for star in range(20):
        x = randint(0, 80) * 5;
        y = randint(0, 45) * 5;

        val = random()
        if val > 0.7:
            if val > 0.9:
                w = 2.5
            else:
                w = 1.5
        else:
            w = 1
        out += f"radial-gradient({w}px {w}px at {x}px {y}px, white, rgba(255, 255, 255, 0)),"

    out = out[:-1] + ";}\n"

print(out)