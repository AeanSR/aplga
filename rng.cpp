#include "aplga.h"

std::mt19937& rng(){
    static std::mt19937 gen;
    return gen;
};

double uni_rng(){
    static std::uniform_real_distribution<double> uni(0.0, 1.0);
    return uni(rng());
}

double nor_rng(){
    static std::normal_distribution<double> nor(0.0, 1.0);
    return nor(rng());
}

