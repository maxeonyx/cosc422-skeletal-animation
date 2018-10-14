#!/usr/bin/fish

g++ -std=c++11 Main.cpp -lglut -lGL -lGLU -lm -lassimp -lIL -o bin/skeletal-animation -Wall
and bin/skeletal-animation