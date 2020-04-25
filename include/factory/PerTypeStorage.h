#pragma once

/// \file
/// \brief Provides truly cross-platorm way to store per-type information across DLL/so boundaries.

#include <typeinfo>
#include <string>
#include <map>

#include "./defs.h"

namespace ctm {

/// \brief Class providing per-type storage.
///
/// Storage cells are identified by pairs (T1, T2), where T1 and T2 are types,
/// and T2 is the data type in the storage cell.
/// \note This class has a static field, \c m_data,
/// that has to be defined in a translation unit of one DLL/so module.
class PerTypeStorage
{
public:
    /// \brief Returns reference to the specified storage cell
    /// \param ClassType First type associated with the storage cell.
    /// \param ValueType Second type associated with the storage cell, also being the cell data type.
    /// \return Reference to storage cell data.
    /// \note When accessed first time, the memory for the storage cell
    /// is allocated and filled with the default-constructed value.
    template< class ClassType, class ValueType >
    static ValueType& value() {
        return value< ValueType >( std::string(typeid(ClassType).name()) + " / " + typeid(ValueType).name() );
    }
private:
    template< class ValueType >
    static ValueType& value( const std::string& key )
    {
        auto& d = data();
        auto it = d.find( key );
        if( it == d.end() ) {
            auto result = new ValueType;
            d[key] = result;
            return *result;
        }
        else
            return *reinterpret_cast<ValueType*>( it->second );
    }

    typedef std::map< std::string, void* > Data;

    static Data& data()
    {
        if( !m_data )
            m_data = new Data;
        return *m_data;
    }

    static FACTORY_API Data *m_data;
};

} // end namespace ctm
