#include "factory/Factory.h"

#include <iostream>

using namespace std;

class Interface
{
public:
    virtual ~Interface() = default;
    virtual void hello() const = 0;
};

class A : public Interface,
        public ctm::FactoryMixin<A, Interface>
{
public:
    A() {
        cout << "A::A()" << endl;
    }
    ~A() override {
        cout << "A::~A()" << endl;
    }
    void hello() const override {
        cout << "A::hello()" << endl;
    }
};

class B : public Interface,
        public ctm::FactoryMixin<B, Interface>
{
public:
    B() {
        cout << "B::B()" << endl;
    }
    ~B() override {
        cout << "B::~B()" << endl;
    }
    void hello() const override {
        cout << "B::hello()" << endl;
    }
};

CTM_FACTORY_REGISTER_TYPE(A, "A");
CTM_FACTORY_REGISTER_TYPE(B, "B");


int main()
{
    try {
        auto x = ctm::Factory<Interface>::newInstance("A");
        x->hello();
        x = ctm::Factory<Interface>::newInstance("B");
        x->hello();
        return EXIT_SUCCESS;
    }
    catch(const exception& e) {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }
}
