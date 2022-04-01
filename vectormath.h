#ifndef VECMATH_HEADER
#define VECMATH_HEADER

#include "iostream"

template<typename val> class vec4{
  public:
    val x = 0, y = 0, z = 0, w = 0;

    vec4(){}
    vec4(val n){
      x = n; y = n; z = n; w = n;
    }

    vec4(val x, val y, val z, val w){
      this->x = x; this->y = y; this->z = z; this->w = w;
    }


    vec4(const vec4<val>& v){
      x = v.x; y = v.y; z = v.z; w = v.w;
    }

    template<typename fromval> vec4(fromval *array, bool wUsed){
      x = (val)array[0]; y = (val)array[1]; z = (val)array[2]; w = wUsed? (val)array[3]: 0;
    }

    vec4<val> operator+(vec4<val> v){
      return vec4<val>(x+v.x, y+v.y, z+v.z, w+v.w);
    }

    vec4<val> operator-(vec4<val> v){
      return vec4<val>(x-v.x, y-v.y, z-v.z, w-v.w);
    }

    vec4<val> operator*(vec4<val> v){
      return vec4<val>(x*v.x, y*v.y, z*v.z, w*v.w);
    }

    template<typename numtype> vec4<val> operator*(numtype n){
      return vec4<val>(x*n, y*n, z*n, w*n);
    }

    template<typename toval> void CopyToMemory(toval *arr, bool wUsed){
      arr[0] = (toval)x; arr[1] = (toval)y; arr[2] = (toval)z; wUsed? arr[3] = (toval)w: 0;
    }

};

template<typename val> std::ostream& operator<<(std::ostream &os, vec4<val> v){
  os << v.x << " " << v.y << " " << v.z << " " << v.w;
  return os;
}

#endif