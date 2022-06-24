// 24 june 2022
// SB Engine v0.1.0-alpha

#include <runtime/world.hpp>

// Global variables are changed only in callbacks.cpp and main.cpp
unsigned long g_markerIndex1 = 0, g_markerIndex2 = 0;
double g_plMass = 3.7e5; // kg
const double g_plRadius = 125.0; // metres

float g_hardness = 120000.f;
float g_visc = 0.63f; // viscosity (0...1)
int g_tPrec = 177; // physics frequency multiplier (i.e. precision)
double g_tMul = 0.1; // deltaTime multiplier (0: time stopped)
double g_forceMul = 50.0; // rotation force multiplier 
