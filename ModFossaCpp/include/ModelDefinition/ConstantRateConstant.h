#ifndef CONSTANTRATECONSTANT_H_
#define CONSTANTRATECONSTANT_H_

#include <string>
#include <memory>

#include <ModelDefinition/RateConstantBase.h>

using std::string;

namespace ModelDefinition {

    class ConstantRateConstant : public RateConstantBase {
    public:
        ConstantRateConstant(string name, double k);
        virtual ~ConstantRateConstant();
        virtual double getRate(const shared_ptr<const StateOfTheWorld> state_of_the_world) const;
        string getName() const;
        double getK() const;

    private:
        const string name;
        const double k;
    };
}
#endif
