#include <math.h>

double fmax(double x, double y) {
    if (x >= y) {
        return x;
    }
    if (x < y) {
        return y;
    }
    if (isnan(x)) {
        return y;
    }
    return x;
}

double fmin(double x, double y) {
    if (x >= y) {
        return y;
    }
    if (x < y) {
        return x;
    }
    if (isnan(x)) {
        return y;
    }
    return x;
}

double fdim(double x, double y) {
    if (x >= y) {
        return x - y;
    }
    if (x < y) {
        return 0;
    }
    if (isnan(x)) {
        return x;
    }
    return y;
}
