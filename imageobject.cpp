#include "imageobject.h"
#include <QImage>

ImageObject::ImageObject(QImage current_image, int current_size_factor, int current_rotation_angle)
{
    this->image = current_image;
    this->size_factor = current_size_factor;
    this->rotation_angle = current_rotation_angle;
}
