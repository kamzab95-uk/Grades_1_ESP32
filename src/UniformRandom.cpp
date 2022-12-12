#include <iostream>
#include <random>

using namespace std;

class UniformRandom {
    mt19937 *gen;
    uniform_int_distribution<> *distr;

    public:
    UniformRandom(int form, int to) {
        random_device rd;
        gen = new mt19937(rd());
        distr = new uniform_int_distribution<>(10, 40);
    }

    int generate() {
        return distr->operator()(*gen);
    }
};
