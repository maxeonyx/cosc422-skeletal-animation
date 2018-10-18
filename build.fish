#!/usr/bin/fish

g++ -std=c++11 src/Main.cpp -lglut -lGL -lGLU -lm -lassimp -lIL -o bin/skeletal-animation -Wall
and bin/skeletal-animation