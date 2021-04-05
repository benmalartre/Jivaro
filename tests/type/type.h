#include <pxr/pxr.h>
#include <pxr/base/tf/declarePtrs.h>
#include <pxr/base/tf/errorMark.h>
#include <pxr/base/tf/instantiateSingleton.h>
#include <pxr/base/tf/iterator.h>
#include <pxr/base/tf/notice.h>
#ifdef PXR_PYTHON_SUPPORT_ENABLED
#include <pxr/base/tf/pyObjWrapper.h>
#include <pxr/base/tf/pyUtils.h>
#endif // PXR_PYTHON_SUPPORT_ENABLED
#include <pxr/base/tf/refPtr.h>
#include <pxr/base/tf/regTest.h>
#include <pxr/base/tf/safeTypeCompare.h>
#include <pxr/base/tf/staticData.h>
#include <pxr/base/tf/type.h>
#include <pxr/base/tf/typeNotice.h>

#include <set>
#include <iostream>


using namespace std;
PXR_NAMESPACE_USING_DIRECTIVE

extern size_t COUNT;
enum _TestEnum {
    A, B, C
};

TF_DECLARE_WEAK_AND_REF_PTRS( CountedClass );

class CountedClass : public TfRefBase, public TfWeakBase {
public:
    static CountedClassRefPtr New() {
	return TfCreateRefPtr(new CountedClass());
    }

    static CountedClassRefPtr New(int initialVal) {
	return TfCreateRefPtr(new CountedClass(initialVal));
    }

    int GetNumber() const { return _number; }
    void SetNumber(int x) { _number = x; }

private:
    CountedClass() :
        _number(0)
    {
    }

    CountedClass(int initialVal) :
        _number(initialVal)
    {
    }

    CountedClass(const CountedClass &c) : 
        _number(c._number)
    {
    }

    int _number;
};

class CountedClassFactory : public TfType::FactoryBase {
public:
    static CountedClassRefPtr New() {
	return CountedClass::New();
    }

    static CountedClassRefPtr New(int initialVal) {
	return CountedClass::New(initialVal);
    }
};


TF_DECLARE_WEAK_PTRS(SingleClass);

class SingleClass : public TfWeakBase {
public:
    static SingleClass& GetInstance() {
	return TfSingleton<SingleClass>::GetInstance();
    }

    SingleClass() :
        _number(0)
    {
    }

    int GetNumber() const { return _number; }
    void SetNumber(int x) { _number = x; }

private:
    int _number;

    friend class TfSingleton<SingleClass>;
};

TF_INSTANTIATE_SINGLETON(SingleClass);


class ConcreteClass {
public:
    ConcreteClass() {number = 0;}
    ConcreteClass(int n) {number = n;}
    virtual ~ConcreteClass();
    virtual void ConcreteFunction() {}
    int number;
};

ConcreteClass::~ConcreteClass()
{
}

class IAbstractClass {
public:
    virtual ~IAbstractClass();
    virtual void AbstractFunction() = 0;
};

IAbstractClass::~IAbstractClass()
{
}

class ChildClass: public ConcreteClass, public IAbstractClass {
public:
    ChildClass() {number = 0;}
    ChildClass(int n) {number = n;}
    ChildClass(const ChildClass& c) {number = c.number * -1;}
    virtual void ConcreteFunction() {}
    virtual void AbstractFunction() {}
};

class GrandchildClass: public ChildClass {
public:
    virtual void ConcreteFunction() {}
    virtual void AbstractFunction() {}
};

// We'll never explicitly lookup a TfType for this class, but
// it should be initialized when we call GetDirectlyDerivedTypes() for Child.
class OtherGrandchildClass: public ChildClass {
public:
    virtual void ConcreteFunction() {}
    virtual void AbstractFunction() {}
};

class UnknownClass {
};

class SomeClassA : public ConcreteClass {
};

class SomeClassB : public IAbstractClass {
};

template <typename T>
T* _New() { return new T; }

class _NoticeListener : public TfWeakBase
{
public:
    _NoticeListener(std::set<TfType> *seenNotices) :
        _seenNotices(seenNotices)
    {
        TfNotice::Register( TfCreateWeakPtr(this),
                            & _NoticeListener::_HandleTypeDeclaredNotice );
    }

    void _HandleTypeDeclaredNotice( const TfTypeWasDeclaredNotice & n ) {
        TF_AXIOM( !n.GetType().IsUnknown() );
        _seenNotices->insert( n.GetType() );
    }

private:
    // Set of types we have seen notices for.
    std::set<TfType> *_seenNotices;
};

template <class T>
struct TfTest_PtrFactory : public TfType::FactoryBase {
    T* New() { return new T; }
};
template <class T>
struct TfTest_RefPtrFactory : public TfType::FactoryBase {
    TfRefPtr<T> New() { return T::New(); }
};
template <class T>
struct TfTest_SingletonFactory : public TfType::FactoryBase {
    T* New() { return &T::GetInstance(); }
};

__declspec(dllexport) bool Test_TfType();
