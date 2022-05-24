---
name: DirectX
---

# DirectX - Skifree-ish

Why Flee when you can Ski?
Take a 'relaxing' ski down the infinite slopes, but be careful of the dangers that lie on those unassuming slopes.

### Experimentation
Bored of trees hurting your ride? Turn them into boosters inside the resource files and speed as fast as you want!

---
## Background

For the first AT Project, I was tasked with writing a small DirectX 11 game engine and a recreation of Skifree, an infinitely scrolling skiing game, where the player attempts to escape a Yeti.

![A Screenshot of the original Ski Free main menu](/projects/University/AT/Project1/skifree.png)

For this project, I explored more of the customisation side, enabling the user to edit almost all of the game via it's XML files. This also allowed me to explore how to read and utilise text files to create the entire graphics pipeline (Using the chosen XML Parser, [TinyXML2](https://github.com/leethomason/tinyxml2))

The Default Loop of the script defines almost nothing, moving the position of the player forwards by a given speed and acceleration, but without any ability to render more than a blank screen that displays the HUD, which contains the "Press Key to Start", the Health, 3 Lives, and Score.

![Image showing off Some of the XML]()

The XML Defines the Shaders and Meshes that are used, as well as the Objects that the Player can interact with, and their effects, such as Speed Boost/Reduction, and Damaging. These effects are written into the engine, as when originally written, I could not decide how the logic would be defined.