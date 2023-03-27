#include <stdio.h>
#include <string.h>
#include "calculator.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Invalid arguments, usage: ./<executable> \"<expression>\"\n");
        return 1;
    }

    Calculator calc;
    double res = calc.parse(argv[1]);
    printf("%.3lf\n", res);
}