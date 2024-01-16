#include <iostream>
#include <memory>

#include <Immortal.h>

class UnitTest
{
public:
    UnitTest() = default;

    UnitTest(const std::string &desc) :
        desc{ desc }
    {

    }

    virtual bool Conformance() const
    {
        return true;
    }

public:
    std::string desc;
};

class RefUnitTest : public UnitTest
{
public:
    virtual bool Conformance() const
    {
        using namespace Immortal;
        class A : public IObject
        {

        };

        /* Contructor */
        Ref<A> b = nullptr;

        /* Copy Assignment */
        {
            Ref<A> a = new A;
            b = a;
        }

        /* Move Constructor */
        {
            Ref<A> c = std::move(b);
        }

        Ref<A> c = new A;
        c = b;

        {
            URef<A> a = new A;
            a.Reset(new A);
        }

        return true;
    }
};

int main()
{
    RefUnitTest{}.Conformance();

    return 0;
}
