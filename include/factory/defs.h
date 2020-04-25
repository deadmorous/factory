#pragma once

#if defined(_WIN32) && defined(_MSC_VER)

#ifdef factory_EXPORTS
#define FACTORY_API __declspec(dllexport)
#else // factory_EXPORTS
#define FACTORY_API __declspec(dllimport)
#endif // factory_EXPORTS
#else // defined(_WIN32) && defined(_MSC_VER)
#define FACTORY_API
#endif // defined(_WIN32) && defined(_MSC_VER)
