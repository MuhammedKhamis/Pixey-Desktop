#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include <QImage>

class ImageObject
{
public:
    ImageObject(QImage current_image, double current_size_factor, int current_rotation_angle);
    ~ImageObject();

    QImage image;
    //to know percentage of zooming till now from the orignal image
    double size_factor;
    //to know angle of rotation till now from the orignal image
    int rotation_angle;
};

#endif // IMAGEOBJECT_H
