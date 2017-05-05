#include "CubicSpline.h"
using namespace glm;

CubicSpline::CubicSpline(vec3 a, vec3 b, vec3 c, vec3 d)
{
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
}

vec3 CubicSpline::getPointOnSpline(float s) const
{
    return (((d * s) + c) * s + b) * s + a;
}

std::vector<CubicSpline> CubicSpline::CalculateCubicSpline(std::vector<vec3> points)
{
    auto n = points.size() - 1;
    auto& v = points;
    std::vector<vec3> gamma(n + 1);
    std::vector<vec3> delta(n + 1);
    std::vector<vec3> D(n + 1);

    int i;
    /* We need to solve the equation
    * taken from: http://mathworld.wolfram.com/CubicSpline.html
    [2 1       ] [D[0]]   [3(v[1] - v[0])  ]
    |1 4 1     | |D[1]|   |3(v[2] - v[0])  |
    |  1 4 1   | | .  | = |      .         |
    |    ..... | | .  |   |      .         |
    |     1 4 1| | .  |   |3(v[n] - v[n-2])|
    [       1 2] [D[n]]   [3(v[n] - v[n-1])]

    by converting the matrix to upper triangular.
    The D[i] are the derivatives at the control points.
    */

    //this builds the coefficients of the left matrix
    gamma[0] = vec3(0);
    gamma[0].x = 1.0f / 2.0f;
    gamma[0].y = 1.0f / 2.0f;
    gamma[0].z = 1.0f / 2.0f;

    for (i = 1; i < n; i++)
    {
        gamma[i] = vec3(1) / ((4.0f * vec3(1)) - gamma[i - 1]);
    }

    gamma[n] = vec3(1) / ((2.0f * vec3(1)) - gamma[n - 1]);

    delta[0] = 3.0f * (v[1] - v[0]) * gamma[0];

    for (i = 1; i < n; i++)
    {
        delta[i] = (3.0f * (v[i + 1] - v[i - 1]) - delta[i - 1]) * gamma[i];
    }

    delta[n] = (3.0f * (v[n] - v[n - 1]) - delta[n - 1]) * gamma[n];

    D[n] = delta[n];

    for (i = n - 1; i >= 0; i--)
    {
        D[i] = delta[i] - gamma[i] * D[i + 1];
    }

    // now compute the coefficients of the cubics 
    std::vector<CubicSpline> C(n);

    for (i = 0; i < n; i++)
    {
        C[i] = CubicSpline(v[i], D[i], 3.0f * (v[i + 1] - v[i]) - 2.0f * D[i] - D[i + 1], 2.0f * (v[i] - v[i + 1]) + D[i] + D[i + 1]);
    }

    return C;
}