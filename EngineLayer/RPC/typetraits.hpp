#ifndef __PRIMITIVE_TRAITS_H
#define __PRIMITIVE_TRAITS_H

#include <string>

template<typename T>
struct PrimitiveTraits
{
   typedef enum {val = false};
};

/////////////////////////////////////////////////////////////////////////////////
//
//  P r i m i t i v e    P o i n t r e
//
//

template<>
struct PrimitiveTraits<int *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned int *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<long *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned long *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<short *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned short *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<float *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<double *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<std::string *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<char *>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<bool *>
{
   typedef enum {val = true};
};

/////////////////////////////////////////////////////////////////////////////////
//
//  P r i m i t i v e    R e f e r e n c e
//
//

template<>
struct PrimitiveTraits<int &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned int &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<long &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned long &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<short &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<unsigned short &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<float &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<double &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<std::string &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<char &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<bool &>
{
   typedef enum {val = true};
};

template<>
struct PrimitiveTraits<void>
{
   typedef enum {val = true};
};

template <typename T>
struct VoidTraits
{
   typedef enum {val = false};
};

template<>
struct VoidTraits <void>
{
   typedef enum {val = true};
};

#endif