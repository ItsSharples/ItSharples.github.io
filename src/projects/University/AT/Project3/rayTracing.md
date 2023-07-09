---
name: Ray Tracing
---

# Ray Tracing

For the third and final project, I was tasked with created a Ray Tracer, using no framework for the rendering (Was able to use frameworks for windowing and displaying the output)

Note. There is a weird bug in the original, where the output only renders reflections on one side, and the first bounce on the other.


## World Representation
The World is made up of Bounding boxes, which contain either child boxes, or a list of triangles. Each Bounding box would attempt to roughly split itself into 2, to improve the ability to Search through this Tree to find where the Ray finally intersected in.