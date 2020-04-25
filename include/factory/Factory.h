#pragma once

/// \file
/// \brief ctm::Factory and ctm::FactoryMixin template classes, plus related classes and macros.

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <cassert>

#include "./PerTypeStorage.h"

namespace ctm {

/// \brief Base class for all factories.
///
/// Typically used internally to obtain the type used for type identifiers.
class FactoryBase
{
public:
    /// \brief Type used for type identifiers.
    typedef std::string TypeId;
};

/// \brief Factory template class.
///
/// \param Interface_ Type of interface associated with the factory.
/// Instances created by the factory must inherit and implement that interface.
///
/// The purpose of this class is to provide methods for dynamic creation of instances by
/// type identifiers, and to retrieve type identifiers of known interface implementations.
///
/// Suppose I is an interface, and X, Y are two implementations of I.
/// To use factory functionality, follow these steps.
/// - Make Factory<I> a base class of I.
/// - Make FactoryMixin<X, I> the base class of X.
/// - Make FactoryMixin<Y, I> the base class of Y.
/// - Register X and Y in the factory, e.g., by using global variables of types X::Registrator, Y::Registrator
///   and passing type identifiers in constructors.
/// - Now you can create instances by calling Factory<I>::newInstance() static method.
/// .
///
/// Example:
/// \code
/// struct I : Factory<I> {
/// public:
///     // Some pure virtual methods
/// };
///
/// class X :
///     public I,
///     public FactoryMixin<X, I>
/// {
/// ...
/// };
/// X::Registrator ImplementationRegistrator("X");
///
/// class Y :
///     public I,
///     public FactoryMixin<Y, I>
/// {
/// ...
/// };
/// Y::Registrator ImplementationRegistrator("Y");
///
/// void foo() {
///     auto x = Factory<I>::newInstance("X"); // x is shared_ptr<I> holding an instance of X
///     auto y = Factory<I>::newInstance("Y"); // y is shared_ptr<I> holding an instance of Y
/// }
/// \endcode
/// \sa FactoryMixin.
template< class Interface_ >
class Factory : public FactoryBase
{
public:
    /// \brief Type for interface associated with the factory.
    typedef Interface_ Interface;

    /// \brief Type for smart pointer to an instance created by the factory.
    typedef std::shared_ptr< Interface > InterfacePtr;

    /// \brief Type for function creating instances.
    typedef std::function<InterfacePtr()> Generator;

    /// \brief Virtual destructor.
    virtual ~Factory() {}

    /// \brief Registers generator for new type identifier
    /// \param typeId Type identifier to associate with \a generator.
    /// \param generator function that returns a new instance of interface implementation.
    /// \sa registeredTypes(), newInstance().
    static void registerType( const TypeId& typeId, Generator generator )
    {
        auto& r = registry();
        assert( r.count(typeId) == 0 );
        r[typeId] = generator;
    }

    /// \brief Creates and returns new instance of specified type.
    /// \param typeId Type identifier (must be previously registered with registerType()).
    /// \return New instance created by calling generator previously passed to registerType()
    /// along with \a typeId.
    /// \sa registerType().
    static InterfacePtr newInstance( const TypeId& typeId )
    {
        auto& r = registry();
        auto it = r.find( typeId );
        if( it == r.end() )
            throw std::runtime_error( std::string("Failed to find type '") + typeId + "' in registry" );
        return it->second();
    }

    /// \brief Returns all previously registered type identifiers.
    /// \sa registerType().
    static std::vector< TypeId > registeredTypes()
    {
        auto& r = registry();
        std::vector< TypeId > result( r.size() );
        std::transform( r.begin(), r.end(), result.begin(), []( const typename Registry::value_type& v ) { return v.first; } );
        return result;
    }

    /// \brief Returns true if the specified type identifier is registered, false if not.
    /// \param typeId Type identifier to check.
    static bool isTypeRegistered( const TypeId& typeId ) {
        return registry().count( typeId ) > 0;
    }

private:
    typedef std::map< TypeId, Generator > Registry;
    static Registry& registry() {
        return PerTypeStorage::value< Factory<Interface>, Registry >();
    }
};

/// \brief Interface for obtaining type identifier from an instance.
/// \param Interface Factory interface.
/// \sa Factory
template< class Interface >
class TypeIdGetter
{
public:
    /// \brief Type used for type identifiers.
    typedef typename Factory<Interface>::TypeId TypeId;

    /// \brief Virtual destructor.
    virtual ~TypeIdGetter() {}

    /// \brief Returns type identifier of this instance.
    virtual TypeId typeId() const = 0;

    /// \brief Returns type identifier of the specified instance.
    /// \param o Instance to get the type identifier from.
    /// \return If \a o supports this interface,
    /// the method invokes the virtual typeId() method on \a o.
    /// Otherwise, an empty typeId is returned (i.e., the empty string).
    template< class T >
    static TypeId typeId( const std::shared_ptr<T>& o ) {
        auto p = dynamic_cast< const TypeIdGetter<Interface>* >( o.get() );
        return p ?   p->typeId() :   TypeId();
    }
};

/// \brief Class to be inherited by implementations of an interface supporting factory.
/// \param Type The derived type.
/// \param Interface The interface associated with the factory, inherited by \a Type.
///
/// To use, just inherit this template class, as shown in the example for the Factory template class.
/// \sa Factory.
template< class Type, class Interface >
class FactoryMixin : public TypeIdGetter< Interface >
{
public:
    /// \brief Returns shared pointer to new instance of type \a Type.
    static std::shared_ptr<Type> newInstance() {
        return std::make_shared<Type>();
    }

    /// \brief Class whose constructor registers a generator of type \a Type associated with certain type identifier.
    class Registrator {
    public:
        /// \brief Constructor.
        /// \param typeId Type identifier to be associated with \a Type.
        ///
        /// Calls Factory::registerType(), passing newInstance() as generator and \a typeId as the type identifier.
        explicit Registrator( typename Factory<Interface>::TypeId typeId ) {
            Factory<Interface>::registerType( typeId, &FactoryMixin<Type, Interface>::newInstance );
            PerTypeStorage::value< FactoryMixin<Type, Interface>, typename Factory<Interface>::TypeId >() = typeId;
        }
    };

    /// \brief Returns type identifier associated with generator and passed in the constructor.
    /// \note Only works after an instance of Registrator is created.
    static typename Factory<Interface>::TypeId staticTypeId() {
        return PerTypeStorage::value< FactoryMixin<Type, Interface>, typename Factory<Interface>::TypeId >();
    }

    /// \copydoc staticTypeId()
    typename Factory<Interface>::TypeId typeId() const {
        return staticTypeId();
    }
};

/// \brief Helper class that allows to statically associate a type identifier with an interface implementation.
/// \param Implementation A class derived from FactoryMixin.
/// \note To make use of this template class, one should provide its specializations.
/// To do so, one can use #CTM_DECL_IMPLEMENTATION_TRAITS
/// or #CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS macro.
/// \sa Factory.
template< class Implementation > struct ImplementationTypeTraits {
    /// \brief Returns type identifier statically associated with the \a Implementation type.
    static FactoryBase::TypeId typeId();
};

/// \brief Helper class for registering a type in a factory.
/// \param Implementation A class derived from FactoryMixin.
/// \note In contrast to FactoryMixin::Registrator, the constructor
/// of this class does not have any arguments. The type identifier of generator being registered
/// is obtained using ImplementationTypeTraits::typeId().
/// \sa Factory.
template< class Implementation >
class ImplementationRegistrator : public Implementation::Registrator {
public:
    /// \brief Constructor.
    ///
    /// Passes type identifier returned by ImplementationTypeTraits::typeId()
    /// to the FactoryMixin::Registrator constructor.
    ImplementationRegistrator() : Implementation::Registrator( ImplementationTypeTraits<Implementation>::typeId() ) {}
};

} // end namespace ctm

/// \brief Registers a type in a factory.
/// \param Class Implementation of factory interface to be registered.
/// Must inherit ctm::FactoryMixin.
/// \param typeId Type identifier to be associated with \a Class.
/// \note This macro declares a static variable of type ctm::FactoryMixin::Registrator, which
/// leads to the registration of \a Class in the appropriate factory.
/// \sa ctm::Factory.
#define CTM_FACTORY_REGISTER_TYPE( Class, typeId ) \
    static Class::Registrator Class##Registrator( typeId );

/// \brief Defines a specialization of ctm::ImplementationTypeTraits.
/// \param Implementation A class derived from FactoryMixin.
/// \param typeName Type identifier to be associated with \a Implementation.
/// \sa ctm::Factory, ctm::ImplementationTypeTraits.
#define CTM_DECL_IMPLEMENTATION_TRAITS( Implementation, typeName ) \
    template<> \
    struct ImplementationTypeTraits< Implementation > { \
    static FactoryBase::TypeId typeId() { return typeName; } \
    };

/// \def CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS( Implementation, typeName )
/// \brief Defines a specialization of ctm::ImplementationTypeTraits for a template implementation of a template interface.
/// \param Implementation A template class derived from ctm::FactoryMixin.
/// \param typeName Type identifier to be associated with \a Implementation.
/// \sa ctm::Factory, ctm::ImplementationTypeTraits, #CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS.
#if defined (__GNUC__) && (__GNUC__ < 8)
#define CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS( Implementation, typeName ) \
    template<> \
    template< class ... args > \
    struct ImplementationTypeTraits< Implementation<args...> > { \
    static FactoryBase::TypeId typeId() { return typeName; } \
    };
#else // defined (__GNUC__) && (__GNUC__ < 8)
#define CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS( Implementation, typeName ) \
    template< class ... args > \
    struct ImplementationTypeTraits< Implementation<args...> > { \
    static FactoryBase::TypeId typeId() { return typeName; } \
    };
#endif // defined (__GNUC__) && (__GNUC__ < 8)


/// \brief Declares a variable of type ctm::ImplementationRegistrator.
/// \param Implementation A class derived from ctm::FactoryMixin.
/// \note In particular, this macro can be used to declare fields of a class.
/// \note Make sure to provide the appropriate ctm::ImplementationTypeTraits specialization before
/// using this macro. To do so, use the #CTM_DECL_IMPLEMENTATION_TRAITS macro.
/// \sa ctm::Factory, ctm::ImplementationTypeTraits, #CTM_DECL_IMPLEMENTATION_TEMPLATE_REGISTRATOR.
#define CTM_DECL_IMPLEMENTATION_REGISTRATOR( Implementation ) \
    ImplementationRegistrator< Implementation > Implementation##Registrator;

/// \brief Declares a variable of type ImplementationRegistrator.
/// \param Implementation A template class derived from FactoryMixin.
/// \param ... Template parameters for \a Implementation.
/// \note In particular, this macro can be used to declare fields of a class.
/// \note Make sure to provide the appropriate ImplementationTypeTraits specialization before
/// using this macro. To do so, use the #CTM_DECL_IMPLEMENTATION_TEMPLATE_TRAITS macro.
/// \sa ctm::Factory, ctm::ImplementationTypeTraits, #CTM_DECL_IMPLEMENTATION_REGISTRATOR.
#define CTM_DECL_IMPLEMENTATION_TEMPLATE_REGISTRATOR( Implementation, ... ) \
    ImplementationRegistrator< Implementation<__VA_ARGS__> > Implementation##Registrator;
