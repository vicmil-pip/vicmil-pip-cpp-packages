/*
[vmdoc:description] Includes glm, which is a library that can be used for linear algebra and math [vmdoc:enddescription]
*/

// ============================================================
//                           Include
// ============================================================

#pragma once

#include "glm/glm/gtx/transform.hpp"
#include "glm/glm/gtx/string_cast.hpp"

namespace vicmil
{
    // Helper function: behaves like make_mat4
    glm::mat4 make_mat4(const std::vector<double> &mat)
    {
        glm::mat4 result(1.0f); // identity
        if (mat.size() != 16)
            return result; // safety check

        // glTF uses column-major order
        for (int i = 0; i < 16; ++i)
            result[i % 4][i / 4] = static_cast<float>(mat[i]);

        return result;
    }

    // Helper function: works like make_mat4 but accepts a pointer to 16 doubles
    glm::mat4 make_mat4(const double *mat)
    {
        glm::mat4 result(1.0f); // identity
        if (!mat)
            return result;

        // glTF uses column-major order
        for (int i = 0; i < 16; ++i)
            result[i % 4][i / 4] = static_cast<float>(mat[i]);

        return result;
    }
}