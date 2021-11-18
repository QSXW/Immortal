#include "impch.h"
#include "OrthographicCamera.h"

namespace Immortal
{

void OrthographicCamera::ReCalculateViewMatrix()
{
    // transformation matrix = tranlate(identity matrix(uint matrix))
    Matrix4 transform = Vector::Translate(position) * Vector::Rotate(rotation, Vector3(0, 0, 1));

    view = Vector::Inverse(transform);
    viewProjection = projection * view;
}

}
