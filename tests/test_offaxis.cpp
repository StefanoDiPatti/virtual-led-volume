/*
Test of the off-axis projection
- centred eye: same matrices of a standard symmetric perspective
- any eye position: the wall corners are projected on the corners of the NDC square
- eye moved to the right: asymmetric frustum
*/

// Std. Includes
#include <iostream>
#include <cmath>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// off-axis projection
#include <render/offaxis.h>

// comparison between floats with a tolerance
bool NearlyEqual(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }

// comparison of all the 16 elements of two matrices
bool MatricesEqual(const glm::mat4& A, const glm::mat4& B)
{
    for(int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (!NearlyEqual(A[i][j], B[i][j]))
                return false;
    
    return true;
}

// we transform the point in clip space, and then we apply the perspective divide
glm::vec3 ProjectToNDC(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& p)
{
    glm::vec4 clip = projection * view * glm::vec4(p, 1.0f);
    return glm::vec3(clip) / clip.w;
}

// it prints the result of a test, and it counts the failures
void Check(const char* name, bool passed, int& failures)
{
    if(passed)
        std::cout << "[PASS] " << name << std::endl;
    else
    {
        std::cout << "[FAIL]" << name << std::endl;
        failures++;
    }
}

/////////////////// MAIN function ///////////////////////
int main()
{
    int failures = 0;

    // the same wall of the application: 6x3 m at z = -3
    glm::vec3 pa(-3.0f, 0.0f, -3.0f);
    glm::vec3 pb(3.0f, 0.0f, -3.0f);
    glm::vec3 pc(-3.0f, 3.0f, -3.0f);
    glm::vec3 pd = pb + pc - pa;

    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // (a) eye in front of the wall centre, at distance 6: same as glm::perspective
    glm::vec3 pe(0.0f, 1.5f, 3.0f);
    OffAxisMatrices m = OffAxisProjection(pa, pb, pc, pe, nearPlane, farPlane);

    // vertical FOV of the wall seen from the eye (half height 1.5 m, distance 6 m), aspect ratio 6/3 = 2
    float fovy = 2 * std::atan(1.5f / 6.0f);

    glm::mat4 expectedProjection = glm::perspective(fovy, 2.0f, nearPlane, farPlane);
    glm::mat4 expectedView = glm::lookAt(pe, pe + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    Check("centered eye: projection", MatricesEqual(m.projection, expectedProjection), failures);
    Check("centered eye: view", MatricesEqual(m.view, expectedView), failures);

    // (b) eye out of centre: the wall corners must be projected on the NDC corners
    pe = glm::vec3(2.5f, 0.5f, 1.0f);
    m = OffAxisProjection(pa, pb, pc, pe, nearPlane, farPlane);

    glm::vec3 ndcA = ProjectToNDC(m.projection, m.view, pa);
    glm::vec3 ndcB = ProjectToNDC(m.projection, m.view, pb);
    glm::vec3 ndcC = ProjectToNDC(m.projection, m.view, pc);
    glm::vec3 ndcD = ProjectToNDC(m.projection, m.view, pd);

    Check("pa -> (-1,-1)", NearlyEqual(ndcA.x, -1.0f) && NearlyEqual(ndcA.y, -1.0f), failures);
    Check("pb -> (+1,-1)", NearlyEqual(ndcB.x, 1.0f) && NearlyEqual(ndcB.y, -1.0f), failures);
    Check("pc -> (-1,+1)", NearlyEqual(ndcC.x, -1.0f) && NearlyEqual(ndcC.y, 1.0f), failures);
    Check("pd -> (+1,+1)", NearlyEqual(ndcD.x, 1.0f) && NearlyEqual(ndcD.y, 1.0f), failures);

    // (c) eye moved to the right: the frustum is not symmetric anymore
    pe = glm::vec3(1.0f, 1.5f, 3.0f);
    m = OffAxisProjection(pa, pb, pc, pe, nearPlane, farPlane);

    Check("eye on the right: asymmetric", m.projection[2][0] < 0 && NearlyEqual(m.projection[2][1], 0), failures);

    if(failures == 0)
        return 0;
    else
        return 1;
}