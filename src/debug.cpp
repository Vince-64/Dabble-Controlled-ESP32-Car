#include <iostream>

struct MapaQuadrantes {
    int q1;
    int q2;
    int q3;
    int q4;
    int q5;
};

int main() {
    struct MapaQuadrantes mapaQuadrantes = {10, 0, 8, 0, 245};
    int *p = (int*)&mapaQuadrantes;
    unsigned int i;

    for(i = 0; i < sizeof(mapaQuadrantes)/sizeof(int); i++) {
        printf("Membro %d: %d\n", i+1, p[i]);
    }
    int velocidade = 150, esterc = -60;

    std::cout << velocidade + esterc;

    return 0;
}