---
name: SSG
tags: 
- Web
skills:
- Python
---

# Custom Static Site Generator (SSG)

Written originally to implement the 'projects' page for this site, this SSG enables users (me) to more generically write HTML templates and Markdown documents.

## Why not use x?
While I am aware of many SSGs (Jekyll, Hugo, etc), I decided that I wanted to try out making my own!
It only really supports what I need it to, such as putting named content in HTML, if/foreach, and calculating basic sums.

## Can I use it?
Currently? No.
It is a goal to disconnect it from this website structure, so that others can use it.
But first I want to remove more of the magic words, and place them into the config.yml

## What languages does it use?
This project uses yaml, regex, and Markdown inside Python 3.10.
The Files are Non-Standard Markdown (with a YAML header), and HTML (with SSG syntax), and Standard YAML in the Config File.
The Markdown files should mostly work without SSG Processing ([Obsidian](https://obsidian.md) uses a similar system), but HTML will be visibly artifacted.