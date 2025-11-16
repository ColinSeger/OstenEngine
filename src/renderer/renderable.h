#include "../engine/transform.h"

struct Renderable
{
    Transform transform;
    const char* mesh_location;
    const char* texture_location;
};
