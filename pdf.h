#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "onb.h"

/**
 * @brief 概率密度函数
 */
class pdf
{
public:
    virtual ~pdf() {}

    virtual double sample(const vec3 &direction) const = 0;

    virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf
{
private:
    onb uvw;

public:
    cosine_pdf(const vec3 &w) { uvw.build_from_w(w); }

    virtual double sample(const vec3 &direction) const override
    {
        auto cosine = dot(unit_vector(direction), uvw.w());
        return (cosine <= 0) ? 0 : cosine / pi;
    }

    virtual vec3 generate() const override
    {
        return uvw.local(random_in_hemisphere());
    }
};

class hittable_pdf : public pdf
{
private:
    point3 o;
    shared_ptr<hittable> object;

public:
    hittable_pdf(shared_ptr<hittable> object, const point3 &origin)
        : object(object), o(origin)
    {
    }

    virtual double sample(const vec3 &direction) const override
    {
        return object->pdf_value(o, direction);
    }

    virtual vec3 generate() const override
    {
        return object->random(o);
    }
};

class mixture_pdf : public pdf
{
private:
    shared_ptr<pdf> p[2];

public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) : p{p0, p1}
    {
    }

    virtual double sample(const vec3 &direction) const override
    {
        return 0.5 * p[0]->sample(direction) + 0.5 * p[1]->sample(direction);
    }

    virtual vec3 generate() const override
    {
        if (random_double() < 0.5)
        {
            return p[0]->generate();
        }
        else
        {
            return p[1]->generate();
        }
    }
};

#endif