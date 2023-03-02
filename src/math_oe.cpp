#include <OE/math_oe.h>

#include <pmmintrin.h>
#include <xmmintrin.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>



using namespace std;

// basically translation from the GLM library

// overloading operators

OE_Vec4 OE_Vec4::operator+(const OE_Vec4& other) {
    auto temp = static_cast<glm::vec4>(*this) + static_cast<glm::vec4>(other);
    return OE_Vec4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Vec4 OE_Vec4::operator-(const OE_Vec4& other) {
    auto temp = static_cast<glm::vec4>(*this) - static_cast<glm::vec4>(other);
    return OE_Vec4(temp[0], temp[1], temp[2], temp[3]);
}


OE_Quat::OE_Quat(glm::quat q) {
    this->w = q.w;
    this->x = q.x;
    this->y = q.y;
    this->z = q.z;
}

OE_Vec3 OE_Mat3x3::operator*(const OE_Vec3& other) {
    auto temp = static_cast<glm::mat3>(*this) * static_cast<glm::vec3>(other);
    return OE_Vec3(temp[0], temp[1], temp[2]);
}

OE_Mat4x4 OE_Mat4x4::operator*(const OE_Mat4x4& other) {
    auto temp = static_cast<glm::mat4>(*this) * static_cast<glm::mat4>(other);
    return OE_Mat4x4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Vec4 OE_Mat4x4::operator*(const OE_Vec4& other) {
    auto temp = static_cast<glm::mat4>(*this) * static_cast<glm::vec4>(other);
    return OE_Vec4(temp[0], temp[1], temp[2], temp[3]);
}


OE_Quat OE_Quat::operator*(const OE_Quat& other) {
    auto temp = static_cast<glm::quat>(*this) * static_cast<glm::quat>(other);
    return OE_Quat(temp);
}


// math library functions
OE_Mat4x4 OE_Transpose(OE_Mat4x4 mat4) {
    auto temp = glm::transpose(static_cast<glm::mat4>(mat4));
    return OE_Mat4x4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Mat4x4 OE_Translate(OE_Mat4x4 mat4, OE_Vec3 vec3) {
    auto temp = glm::translate(static_cast<glm::mat4>(mat4), static_cast<glm::vec3>(vec3));
    return OE_Mat4x4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Mat4x4 OE_Scale(OE_Mat4x4 mat4, OE_Vec3 vec3) {
    auto temp = glm::scale(static_cast<glm::mat4>(mat4), static_cast<glm::vec3>(vec3));
    return OE_Mat4x4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Mat4x4 OE_Quat2Mat4x4(OE_Quat quat) {

    auto temp = glm::toMat4(static_cast<glm::quat>(quat));
    return OE_Mat4x4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Vec4 OE_Normalize(OE_Vec4 vec4) {
    auto temp = glm::normalize(static_cast<glm::vec4>(vec4));
    return OE_Vec4(temp[0], temp[1], temp[2], temp[3]);
}

OE_Quat OE_Normalize(OE_Quat quat) {
    auto temp = glm::normalize(static_cast<glm::quat>(quat));
    return OE_Quat(temp);
}

float OE_Length(OE_Vec3 vec3) {
    return std::sqrt(std::pow(vec3[0], 2) + std::pow(vec3[1], 2) + std::pow(vec3[2], 2));
}

float OE_Length(OE_Vec4 vec4) {
    return std::sqrt(std::pow(vec4[0], 2) + std::pow(vec4[1], 2) + std::pow(vec4[2], 2) + std::pow(vec4[3], 2));
}


std::vector<float> OE_Mat4x4ToSTDVector(OE_Mat4x4 matrix) {
    vector<float> output;
    output.reserve(16);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            output.push_back(matrix[i][j]);
        }
    }
    return output;
}

std::vector<float> OE_Vec4ToSTDVector(OE_Vec4 vector_in) {
    vector<float> output;
    output.reserve(4);
    for (int i = 0; i < 4; i++) {
        output.push_back(vector_in[i]);
    }
    return output;
}


OE_Mat4x4 OE_Perspective(float fov, float aspect, float near, float far) {

    // float fov_rad = FE_Math::radians(fov);
    float fov_rad = fov;
    float range   = std::tan(fov_rad / 2.0f) * near;
    // std::cout << "range: " << range << std::endl;
    float     sx     = (2.0f * near) / (range * aspect + range * aspect);
    float     sy     = near / range;
    float     sz     = -(far + near) / (far - near);
    float     pz     = -(2.0f * far * near) / (far - near);
    OE_Mat4x4 output = OE_Mat4x4(0.0f);
    output[0][0]     = sx;
    output[1][1]     = sy;
    output[2][2]     = sz;
    output[3][2]     = pz;
    output[2][3]     = -1.0f;
    // std::cout << output.print(false) << std::endl;
    return output;
}

OE_Vec4 OE_GetClipCoords(OE_Mat4x4 perspective, OE_Vec4 view_coords) {
    OE_Vec4 temp  = perspective * view_coords;
    OE_Vec4 temp2 = OE_Vec4(temp[0] / temp.w, temp[1] / temp.w, temp[2] / temp.w, temp[3] / temp.w);
    return temp2;
}


OE_Quat OE_QuatFromAxisAngle(float a, float xx, float yy, float zz) {
    float factor = std::sin(a / 2.0f);
    return OE_Normalize(OE_Quat(std::cos(a / 2.0f), xx * factor, yy * factor, zz * factor));
}

std::vector<float> OE_GetBoundingBoxVertexBuffer(float max_x, float min_x, float max_y, float min_y, float max_z, float min_z) {

    std::vector<float> output;
    output.reserve(216);

    // max_x
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);

    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);

    // min_x
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);

    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(0.0f);

    // max_y
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);

    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(0.0f);

    // min_y
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);

    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(0.0f);

    // max_z
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);

    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(max_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(1.0f);

    // min_z
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(min_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);

    output.push_back(min_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(max_x);
    output.push_back(max_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);
    output.push_back(max_x);
    output.push_back(min_y);
    output.push_back(min_z);
    output.push_back(0.0f);
    output.push_back(0.0f);
    output.push_back(-1.0f);

    return output;
}

std::vector<float> OE_GetBoundingSphereVertexBuffer(float r1, float r2, size_t n) {
    std::vector<float> vbo;

    assert(n % 2 == 0);

    // push top vertex
    vbo.push_back(0.0f);
    vbo.push_back(0.0f);
    vbo.push_back(r1);

    // push top vertex normal
    vbo.push_back(0.0f);
    vbo.push_back(0.0f);
    vbo.push_back(r1);

    for (uint32_t k = 0; k < 2 * n; k++) {
        for (uint32_t m = 1; m < n; m++) {

            // push once for position
            vbo.push_back(r2 * std::cos(oe::PI / n * k) * std::sin(oe::PI / n * m));
            vbo.push_back(r2 * std::sin(oe::PI / n * k) * std::sin(oe::PI / n * m));
            vbo.push_back(r1 * std::cos(oe::PI / n * m));
            // and once for normal
            vbo.push_back(r2 * std::cos(oe::PI / n * k) * std::sin(oe::PI / n * m));
            vbo.push_back(r2 * std::sin(oe::PI / n * k) * std::sin(oe::PI / n * m));
            vbo.push_back(r1 * std::cos(oe::PI / n * m));
        }
    }

    // push bottom vertex
    vbo.push_back(0.0f);
    vbo.push_back(0.0f);
    vbo.push_back(-r1);

    // push bottom vertex normal
    vbo.push_back(0.0f);
    vbo.push_back(0.0f);
    vbo.push_back(-r1);

    return vbo;
}

std::vector<uint32_t> OE_GetBoundingSphereIndexBuffer(float r1, float r2, size_t n) {
    std::vector<uint32_t> ibo;

    assert(n % 2 == 0);

    for (uint32_t k = 0; k < 2 * n; k++) {

        ibo.push_back(0);
        ibo.push_back(1 + (n - 1) * k);
        ibo.push_back(1 + (n - 1) * ((k + 1) % (2 * n)));

        for (uint32_t m = 1; m < n - 1; m++) {
            // one square
            ibo.push_back(m + (n - 1) * k);
            ibo.push_back(m + 1 + (n - 1) * k);
            ibo.push_back(m + (n - 1) * ((k + 1) % (2 * n)));

            ibo.push_back(m + (n - 1) * ((k + 1) % (2 * n)));
            ibo.push_back(m + 1 + (n - 1) * k);
            ibo.push_back(m + 1 + (n - 1) * ((k + 1) % (2 * n)));
        }
        ibo.push_back(n - 1 + (n - 1) * k);
        ibo.push_back((n - 1) * (2 * n) + 1);
        ibo.push_back(n - 1 + (n - 1) * ((k + 1) % (2 * n)));
    }
    return ibo;
}

std::vector<float> oe::math::vertex_shader_regular_sw(const std::vector<float>& vbo, OE_Mat4x4 model_matrix_in,
                                                      OE_Mat4x4 mvp_matrix_in, int num_of_uvs) {

    auto         bufsize = vbo.size();
    const float* bufptr  = &vbo[0];

    auto model_matrix = OE_Mat4x4ToSTDVector(model_matrix_in);
    auto mvp_matrix   = OE_Mat4x4ToSTDVector(mvp_matrix_in);

    __m128 model_mat_1 = _mm_setr_ps(model_matrix[0], model_matrix[1], model_matrix[2], model_matrix[3]);
    __m128 model_mat_2 = _mm_setr_ps(model_matrix[4], model_matrix[5], model_matrix[6], model_matrix[7]);
    __m128 model_mat_3 = _mm_setr_ps(model_matrix[8], model_matrix[9], model_matrix[10], model_matrix[11]);
    __m128 model_mat_4 = _mm_setr_ps(model_matrix[12], model_matrix[13], model_matrix[14], model_matrix[15]);

    __m128 mvp_mat_1 = _mm_setr_ps(mvp_matrix[0], mvp_matrix[1], mvp_matrix[2], mvp_matrix[3]);
    __m128 mvp_mat_2 = _mm_setr_ps(mvp_matrix[4], mvp_matrix[5], mvp_matrix[6], mvp_matrix[7]);
    __m128 mvp_mat_3 = _mm_setr_ps(mvp_matrix[8], mvp_matrix[9], mvp_matrix[10], mvp_matrix[11]);
    __m128 mvp_mat_4 = _mm_setr_ps(mvp_matrix[12], mvp_matrix[13], mvp_matrix[14], mvp_matrix[15]);

    __m128 empty_buf = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);

    long int offset          = 6 + num_of_uvs * 2;
    long int num_of_vertices = bufsize / offset;

    std::vector<float> output;
    output.reserve(num_of_vertices * (offset + 4));
    output.resize(num_of_vertices * (offset + 4));

    for (long int i = 0; i < num_of_vertices; i++) {

        alignas(16) float vertices[4];
        alignas(16) float normals[4];

        vertices[3] = 1.0f;
        normals[3]  = 0.0f;

        std::memcpy(&vertices[0], &bufptr[i * offset], 12);
        std::memcpy(&normals[0], &bufptr[i * offset + 3], 12);

        __m128 vertex_in  = _mm_load_ps(&vertices[0]);
        __m128 normals_in = _mm_load_ps(&normals[0]);

        __m128 mm1  = _mm_mul_ps(vertex_in, model_mat_1);
        __m128 mmn1 = _mm_mul_ps(normals_in, model_mat_1);
        __m128 mvp1 = _mm_mul_ps(vertex_in, mvp_mat_1);

        __m128 mm2  = _mm_mul_ps(vertex_in, model_mat_2);
        __m128 mmn2 = _mm_mul_ps(normals_in, model_mat_2);
        __m128 mvp2 = _mm_mul_ps(vertex_in, mvp_mat_2);

        __m128 mm3  = _mm_mul_ps(vertex_in, model_mat_3);
        __m128 mmn3 = _mm_mul_ps(normals_in, model_mat_3);
        __m128 mvp3 = _mm_mul_ps(vertex_in, mvp_mat_3);

        __m128 mm4  = _mm_mul_ps(vertex_in, model_mat_4);
        __m128 mvp4 = _mm_mul_ps(vertex_in, mvp_mat_4);

        __m128 mm12  = _mm_hadd_ps(mm1, mm2);
        __m128 mvp12 = _mm_hadd_ps(mvp1, mvp2);
        __m128 mmn12 = _mm_hadd_ps(mmn1, mmn2);

        __m128 mm34  = _mm_hadd_ps(mm3, mm4);
        __m128 mvp34 = _mm_hadd_ps(mvp3, mvp4);
        __m128 mmn34 = _mm_hadd_ps(mmn3, empty_buf);

        __m128 mmn1234 = _mm_hadd_ps(mmn12, mmn34);
        __m128 mm1234  = _mm_hadd_ps(mm12, mm34);
        __m128 mvp1234 = _mm_hadd_ps(mvp12, mvp34);

        _mm_store_ps(&normals[0], mmn1234);
        _mm_store_ps(&vertices[0], mm1234);
        _mm_storeu_ps(&output[i * (offset + 4) + 6], mvp1234);

        std::memcpy(&output[i * (offset + 4)], &vertices[0], 12);
        std::memcpy(&output[i * (offset + 4) + 3], &normals[0], 12);

        for (int uv = 0; uv < num_of_uvs; uv++) {
            std::memcpy(&output[i * (offset + 4) + 10 + uv * 2], &bufptr[i * offset + 6 + uv * 2], 8);
        }
    }

    return output;
}
