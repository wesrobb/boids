#include "collision.h"

#include "cglm/cglm.h"
#undef bool // Thanks cglm

static void farthestPoint(vec3 verts, u32 count, vec3 direction, vec3 dest)
{
    f32 maxDot = glm_vec3_dot(&verts[0], direction);
    u32 maxIndex = 0;

    for (u32 i = 1; i < count; i++)
    {
        f32 dot = glm_vec3_dot(&verts[i], direction);
        if (dot > maxDot)
        {
            maxDot = dot;
            maxIndex = i;
        }
    }

    glm_vec3_copy(&verts[maxIndex], dest);
}

// Returns true if the support is past the origin, false otherwise.
static bool support(vec3 aVerts, u32 aCount, vec3 bVerts, u32 bCount, vec3 dir, vec3 dest)
{
    vec3 aFarthest;
    farthestPoint(aVerts, aCount, dir, aFarthest);

    vec3 negative_dir;
    glm_vec3_negate_to(dir, negative_dir);

    vec3 bFarthest;
    farthestPoint(bVerts, bCount, negative_dir, bFarthest);

    glm_vec3_sub(aFarthest, bFarthest, dest);

    return glm_vec3_dot(dir, dest) > 0;
}

bool gjk_collide(vec3 aVerts, u32 aCount, vec3 bVerts, u32 bCount)
{
    // a = last point added to simplex
    // b = second last
    // c = third last etc etc

    vec3 simplex[4] = {0};
    i32 simplexIndex = 0;
    bool pastOrigin;
    vec3 dir = {0.0f, 1.0f, 0.0f};

    // Simplex 0
    {
        pastOrigin = support(aVerts, aCount, bVerts, bCount, dir, simplex[simplexIndex++]);
        if (!pastOrigin) return false;
    }

    // Simplex 1
    {
        // Second simplex point aims at the origin from the first simplex point.
        glm_vec3_negate_to(simplex[simplexIndex], dir);

        pastOrigin = support(aVerts, aCount, bVerts, bCount, dir, simplex[simplexIndex++]);
        if (!pastOrigin) return false;
    }

    // Simplex 2
    {
        vec3 a0; // a = last point added so ao = a -> origin = -a
        glm_vec3_negate_to(simplex[simplexIndex], a0);

        vec3 ab; // b = 2nd last point added
        glm_vec3_sub(simplex[simplexIndex - 1], simplex[simplexIndex], ab);

        // Get the vector pointing toward the origin from line segment ab
        // This is done using the "Vector Triple Product"
        glm_vec3_cross(a0, ab, dir);
        glm_vec3_cross(ab, dir, dir);

        pastOrigin = support(aVerts, aCount, bVerts, bCount, dir, simplex[simplexIndex++]);
        if (!pastOrigin) return false;
    }

    // Simplex 3
    {
        vec3 ab; // b = 2nd last point added
        glm_vec3_sub(simplex[simplexIndex - 1], simplex[simplexIndex], ab);

        vec3 ac; // c = 3rd last point added
        glm_vec3_sub(simplex[simplexIndex - 2], simplex[simplexIndex], ac);

        // Set the direction vector perpendicular to ab & ac
        glm_vec3_cross(ac, ab, dir);

        // Make sure it points toward the origin
        vec3 a0;
        glm_vec3_negate_to(simplex[simplexIndex], a0);
        if (glm_vec3_dot(a0, dir) < 0)
        {
            glm_vec3_negate(dir);
        }

        pastOrigin = support(aVerts, aCount, bVerts, bCount, dir, simplex[simplexIndex++]);
        if (!pastOrigin) return false;
    }

    // Simplex 4
    {
        /* Made with https://textik.com
                                         D
                                        /|\
                                      /- \ -\
                                    /-    |  -\
                                   /      \    -\
                                 /-        |     \
                               /-          \      -\
                              /             |       -\
                            /-              |      ----C
                          /-                \  ---/   |
                        /-                ----/       |
                       /              ---/   \        |
                     /-          ----/        |      /
                   /-        ---/             \      |
                  /     ----/                  |     |
                /-  ---/                       \     |
              /- --/                            |    |
           A ---\                               |    |
                 ----\                          \    |
                      ----\                      |   |
                           -----\                \  /
                                 ----\            | |
                                      ----\       \ |
                                           ----\   ||
                                                ---||
                                                   B
         */

        while (true)
        {
            // Calculate tetrahedron edges that make up the faces we are interested in
            // Note that we don't care about the abc triangle since we know we are "above" it
            vec3 da, db, dc;
            glm_vec3_sub(simplex[simplexIndex], simplex[simplexIndex - 3], da);
            glm_vec3_sub(simplex[simplexIndex - 1], simplex[simplexIndex - 3], db);
            glm_vec3_sub(simplex[simplexIndex - 2], simplex[simplexIndex - 3], dc);

            // Calculate the normals for each face
            vec3 abdNorm, bcdNorm, cadNorm;
            glm_vec3_cross(da, db, abdNorm);
            glm_vec3_cross(db, dc, bcdNorm);
            glm_vec3_cross(dc, da, cadNorm);

            // Calculate the direction to origin from d
            vec3 d0;
            glm_vec3_negate_to(simplex[simplexIndex - 3], d0);

            u32 simplexIndexToOverwrite = 0;
            if (glm_vec3_dot(abdNorm, d0) > 0)
            {
                // Origin is outside abd face (counter clockwise winding order)
                // so discard C and update simplex in abdNorm direction
                simplexIndexToOverwrite = simplexIndex - 2;
                glm_vec3_copy(abdNorm, dir);
            }
            else if (glm_vec3_dot(bcdNorm, d0) > 0)
            {
                // Origin is outside bcd face (counter clockwise winding order)
                // so discard A and update simplex in bcdNorm direction
                simplexIndexToOverwrite = simplexIndex;
                glm_vec3_copy(bcdNorm, dir);
            }
            else if (glm_vec3_dot(cadNorm, d0) > 0)
            {
                // Origin is outside cad face (counter clockwise winding order)
                // so discard B and update simplex in cadNorm direction
                simplexIndexToOverwrite = simplexIndex - 1;
                glm_vec3_copy(cadNorm, dir);
            }
            else
            {
                // We have a collision.
                return true;
            }

            pastOrigin = support(aVerts, aCount, bVerts, bCount, dir, simplex[simplexIndexToOverwrite]);
            if (!pastOrigin) return false;
        }
    }

    return false;
}
