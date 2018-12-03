#pragma once

#include <G3Frame.h>
#include <G3Map.h>
#include <G3TimeStamp.h>

#include <stdint.h>

using namespace std;

// Template class for working with intervals -- pairs of objects of
// the same (well-ordered) type, with operations defined that support
// intersection, union, and subtraction.

template <typename T>
class Intervals : public G3FrameObject {
public:
    pair<T,T> domain;
    vector<pair<T,T>> segments;
    
    // Construction
    Intervals();
    Intervals(pair<T,T> domain) : domain{domain} {}
    Intervals(T start, T end) { Intervals(make_pair(start, end)); }

    // Basic ops
    Intervals<T>& merge(const Intervals<T> &src);
    Intervals<T>& intersect(const Intervals<T> &src);
    Intervals<T>& add_interval(const T start, const T end);
    Intervals<T> get_complement() const;

    void trim_to(T start, T end);
    void cleanup();

    // Operators.
    Intervals<T> operator-() const;
    void operator+=(const Intervals<T> &src);
    void operator-=(const Intervals<T> &src);
    Intervals<T> operator+(const Intervals<T> &src) const;
    Intervals<T> operator-(const Intervals<T> &src) const;
    Intervals<T> operator*(const Intervals<T> &src) const;

    // Required for G3FrameObjects.
    string Description() const;
    template <class A> void serialize(A &ar, unsigned v);
};


typedef Intervals<double> IntervalsFloat;
typedef Intervals<int64_t> IntervalsInt;
typedef Intervals<G3Time> IntervalsTime;

G3_SERIALIZABLE(IntervalsFloat, 0);
G3_SERIALIZABLE(IntervalsInt, 0);
G3_SERIALIZABLE(IntervalsTime, 0);

G3MAP_OF(std::string, IntervalsFloat, MapIntervalsFloat);
G3MAP_OF(std::string, IntervalsInt,   MapIntervalsInt);
G3MAP_OF(std::string, IntervalsTime,  MapIntervalsTime);
