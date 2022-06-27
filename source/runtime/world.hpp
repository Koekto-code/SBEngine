// 27 june 2022
// SB Engine v0.1.0-alpha

// Global variables (are changed in callbacks.cpp and main.cpp)
extern unsigned long g_markerIndex1 , g_markerIndex2;
extern double g_plMass; // kg
extern const double g_plRadius;

extern float g_hardness;
extern float g_visc; // viscosity (0...1)
extern int g_tPrec; // physics frequency multiplier (i.e. precision)
extern double g_tMul; // deltaTime multiplier (0: time stopped)
extern double g_forceMul; // rotation force multiplier 
